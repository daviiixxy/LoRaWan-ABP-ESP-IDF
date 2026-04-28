/******************************************************************************
 * Copyright © 2026 HUB I4.0 - Universidade do Estado do Amazonas
 * All rights reserved
 *****************************************************************************
 * LoRaWan-ABP - Custom ABP LoRaWAN library for ESP-IDF / SX127x
 */

#include "lorawan_abp.h"

#include "esp_event.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "hal/hal_esp32.h"
#include "lmic/lmic.h"
#include "lorawan_abp_logging.h"
#include "lorawan_abp_nvs.h"
#include "lorawan_abp_provisioning.h"
#include "lorawan_abp_rtc.h"

#define TAG "lorawan_abp"
#define DEFAULT_MAX_TX_POWER -1000

typedef enum
{
    LORA_WAITING_NONE,
    LORA_WAITING_FOR_TRANSMISSION
} lora_waiting_reason_t;

typedef enum
{
    LORA_EVENT_NONE,
    LORA_EVENT_MESSAGE_RECEIVED,
    LORA_EVENT_TRANSMISSION_COMPLETED,
    LORA_EVENT_TRANSMISSION_FAILED
} lora_event_t;

typedef struct
{
    lora_event_t event;
    uint8_t port;
    const uint8_t *message;
    size_t message_size;
} lora_lmic_event_t;

static bool is_started;
static QueueHandle_t lmic_event_queue;
static lora_message_cb message_callback;
static lora_waiting_reason_t waiting_reason;
static int subband = 2;
static lora_data_rate_t data_rate = LORA_DR_JOIN_DEFAULT;
static int max_tx_power = DEFAULT_MAX_TX_POWER;

static void config_rf_params(void);
static void event_callback(void *user_data, ev_t event);
static void message_received_callback(void *user_data, uint8_t port, const uint8_t *message, size_t message_size);
static void message_transmitted_callback(void *user_data, int success);

void lorawan_abp_init(void)
{
    message_callback = NULL;
    hal_esp32_init_critical_section();
}

void lorawan_abp_configure_pins(spi_host_device_t spi_host, uint8_t nss, uint8_t rxtx, uint8_t rst, uint8_t dio0,
                                uint8_t dio1)
{
    hal_esp32_configure_pins(spi_host, nss, rxtx, rst, dio0, dio1);
}

void lora_set_subband(int band)
{
    subband = band;
}

void lora_set_data_rate(lora_data_rate_t dr)
{
    data_rate = dr;

    if (is_started)
    {
        hal_esp32_enter_critical_section();
        LMIC_setDrTxpow((dr_t)dr, LMIC.adrTxPow);
        hal_esp32_leave_critical_section();
    }
}

void lora_set_max_tx_power(int tx_pow)
{
    max_tx_power = tx_pow;

    if (is_started)
    {
        hal_esp32_enter_critical_section();
        LMIC_setDrTxpow(LMIC.datarate, tx_pow);
        hal_esp32_leave_critical_section();
    }
}

bool lora_adr_enabled(void)
{
    return LMIC.adrEnabled != 0;
}

void lora_set_adr_enabled(bool enabled)
{
    hal_esp32_enter_critical_section();
    LMIC_setAdrMode(enabled);
    hal_esp32_leave_critical_section();
}

void config_rf_params(void)
{
#if defined(CFG_us915) || defined(CFG_au915)
    if (subband != 0)
        LMIC_selectSubBand(subband - 1);
#endif

    if (data_rate != LORA_DR_JOIN_DEFAULT || max_tx_power != DEFAULT_MAX_TX_POWER)
    {
        dr_t dr = data_rate == LORA_DR_JOIN_DEFAULT ? LMIC.datarate : (dr_t)data_rate;
        s1_t txpow = max_tx_power == DEFAULT_MAX_TX_POWER ? LMIC.adrTxPow : max_tx_power;
        LMIC_setDrTxpow(dr, txpow);
    }
}

bool lorawan_abp_setup(uint32_t netid, uint32_t devaddr, const uint8_t *nwk_skey, const uint8_t *app_skey)
{
    if (is_started)
        return false;

    LMIC_registerEventCb(event_callback, NULL);
    LMIC_registerRxMessageCb(message_received_callback, NULL);

    os_init_ex(NULL);
    hal_esp32_enter_critical_section();

    LMIC_reset();
    LMIC_setClockError(MAX_CLOCK_ERROR * 10 / 100);

    // ABP Config
    LMIC_setSession(netid, devaddr, (xref2u1_t)nwk_skey, (xref2u1_t)app_skey);

    // Disable LinkCheckMode for ABP
    LMIC_setLinkCheckMode(0);

    // Apply user-configured RF params (subband, DR, TX power)
    config_rf_params();

    waiting_reason = LORA_WAITING_NONE;

    hal_esp32_leave_critical_section();

    lmic_event_queue = xQueueCreate(4, sizeof(lora_lmic_event_t));
    hal_esp32_start_lmic_task();

    is_started = true;
    ESP_LOGI(TAG, "ABP Configured and Background Task Started");
    return true;
}

lora_response_code_t lorawan_abp_transmit_message(const uint8_t *payload, size_t length, lora_port_t port, bool confirm)
{
    hal_esp32_enter_critical_section();
    if (waiting_reason != LORA_WAITING_NONE || (LMIC.opmode & OP_TXRXPEND) != 0)
    {
        hal_esp32_leave_critical_section();
        return LORA_ERROR_TRANSMISSION_FAILED;
    }

    waiting_reason = LORA_WAITING_FOR_TRANSMISSION;
    LMIC.client.txMessageCb = message_transmitted_callback;
    LMIC.client.txMessageUserData = NULL;


    LMIC_setTxData2(port, (xref2u1_t)payload, length, confirm);
    hal_esp32_wake_up();
    hal_esp32_leave_critical_section();

    while (true)
    {
        lora_lmic_event_t result;
        xQueueReceive(lmic_event_queue, &result, portMAX_DELAY);

        switch (result.event)
        {
        case LORA_EVENT_MESSAGE_RECEIVED:
            if (message_callback != NULL)
                message_callback(result.message, result.message_size, result.port);
            break;

        case LORA_EVENT_TRANSMISSION_COMPLETED:
            return LORA_SUCCESSFUL_TRANSMISSION;

        case LORA_EVENT_TRANSMISSION_FAILED:
            return LORA_ERROR_TRANSMISSION_FAILED;

        default:
            return LORA_ERROR_UNEXPECTED;
        }
    }
}

void lorawan_abp_on_message(lora_message_cb callback)
{
    message_callback = callback;
}

// --- State Save & Restore ---

static void stop(void)
{
    if (!is_started)
        return;
    hal_esp32_enter_critical_section();
    LMIC_shutdown();
    hal_esp32_stop_lmic_task();
    waiting_reason = LORA_WAITING_NONE;
    hal_esp32_leave_critical_section();
    is_started = false;
}

void lorawan_abp_prepare_for_deep_sleep(void)
{
    lorawan_abp_rtc_save();
    stop();
}

void lorawan_abp_prepare_for_power_off(void)
{
    lorawan_abp_nvs_save();
    stop();
}

bool lorawan_abp_resume_after_deep_sleep(void)
{
    if (!lorawan_abp_provisioning_have_keys())
    {
        lorawan_abp_provisioning_restore_keys(true);
    }
    if (!lorawan_abp_provisioning_have_keys())
        return false;

    lorawan_abp_setup(0x000000, lorawan_abp_get_devaddr(), lorawan_abp_get_nwkskey(), lorawan_abp_get_appskey());
    if (!lorawan_abp_rtc_restore())
        return false;

    return true;
}

bool lorawan_abp_resume_after_power_off(int off_duration)
{
    if (!lorawan_abp_provisioning_have_keys())
    {
        lorawan_abp_provisioning_restore_keys(true);
    }
    if (!lorawan_abp_provisioning_have_keys())
        return false;

    lorawan_abp_setup(0x000000, lorawan_abp_get_devaddr(), lorawan_abp_get_nwkskey(), lorawan_abp_get_appskey());
    if (!lorawan_abp_nvs_restore(off_duration))
        return false;

    return true;
}

// --- Callbacks ---

#if CONFIG_LOG_DEFAULT_LEVEL >= 3 || LMIC_ENABLE_event_logging
static const char *event_names[] = {LMIC_EVENT_NAME_TABLE__INIT};
#endif

void event_callback(void *user_data, ev_t event)
{
#if LMIC_ENABLE_event_logging
    lorawan_abp_log_event(event, event_names[event], 0);
#elif CONFIG_LOG_DEFAULT_LEVEL >= 3
    ESP_LOGI(TAG, "event %s", event_names[event]);
#endif

    if (event == EV_TXCOMPLETE)
    {
        // Handled natively by LMIC internally for successful TX/RX queue passing
    }
}

void message_received_callback(void *user_data, uint8_t port, const uint8_t *message, size_t message_size)
{
    lora_lmic_event_t result = {
        .event = LORA_EVENT_MESSAGE_RECEIVED, .port = port, .message = message, .message_size = message_size};
    xQueueSend(lmic_event_queue, &result, pdMS_TO_TICKS(100));
}

void message_transmitted_callback(void *user_data, int success)
{
    waiting_reason = LORA_WAITING_NONE;
    lora_lmic_event_t result = {.event = success ? LORA_EVENT_TRANSMISSION_COMPLETED : LORA_EVENT_TRANSMISSION_FAILED};
    xQueueSend(lmic_event_queue, &result, pdMS_TO_TICKS(100));
}
