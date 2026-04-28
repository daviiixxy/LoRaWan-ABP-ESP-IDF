/******************************************************************************
 * Copyright © 2026 HUB I4.0 - Universidade do Estado do Amazonas
 * All rights reserved
 *****************************************************************************
 * Functions for provisioning LoRaWAN ABP communication.
 */

#include "lorawan_abp_provisioning.h"

#include "driver/uart.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "hal/hal_esp32.h"
#include "lmic/lmic.h"
#include "nvs_flash.h"
#include <string.h>

#define TAG "lorawan_prov"
#define NVS_FLASH_PARTITION "lorawan_abp"
#define NVS_FLASH_KEY_DEVADDR "devAddr"
#define NVS_FLASH_KEY_NWKSKEY "nwkSKey"
#define NVS_FLASH_KEY_APPSKEY "appSKey"

static bool read_nvs_value(nvs_handle handle, const char *key, uint8_t *data, size_t expected_length, bool silent);
static bool write_nvs_value(nvs_handle handle, const char *key, const uint8_t *data, size_t len);
static bool hex_str_to_bin(const char *hex, uint8_t *buf, int len);
static int hex_tuple_to_byte(const char *hex);
static int hex_digit_to_val(char ch);

static uint32_t global_devaddr = 0;
static uint8_t global_nwkskey[16] = {0};
static uint8_t global_appskey[16] = {0};

static bool have_keys = false;

void lorawan_abp_provisioning_init(void)
{
}

bool lorawan_abp_provisioning_have_keys(void)
{
    return have_keys;
}

uint32_t lorawan_abp_get_devaddr(void)
{
    return global_devaddr;
}
const uint8_t *lorawan_abp_get_nwkskey(void)
{
    return global_nwkskey;
}
const uint8_t *lorawan_abp_get_appskey(void)
{
    return global_appskey;
}

bool lorawan_abp_provisioning_decode_keys(const char *devaddr_str, const char *nwkskey_str, const char *appskey_str)
{
    uint8_t buf_devaddr[4];
    uint8_t buf_nwkskey[16];
    uint8_t buf_appskey[16];

    if (strlen(devaddr_str) != 8 || !hex_str_to_bin(devaddr_str, buf_devaddr, 4))
    {
        ESP_LOGW(TAG, "Invalid DevAddr: %s", devaddr_str);
        return false;
    }
    if (strlen(nwkskey_str) != 32 || !hex_str_to_bin(nwkskey_str, buf_nwkskey, 16))
    {
        ESP_LOGW(TAG, "Invalid NwkSKey: %s", nwkskey_str);
        return false;
    }
    if (strlen(appskey_str) != 32 || !hex_str_to_bin(appskey_str, buf_appskey, 16))
    {
        ESP_LOGW(TAG, "Invalid AppSKey: %s", appskey_str);
        return false;
    }

    global_devaddr = (buf_devaddr[0] << 24) | (buf_devaddr[1] << 16) | (buf_devaddr[2] << 8) | buf_devaddr[3];
    memcpy(global_nwkskey, buf_nwkskey, 16);
    memcpy(global_appskey, buf_appskey, 16);

    have_keys = (global_devaddr != 0);
    return true;
}

bool lorawan_abp_provisioning_save_keys()
{
    nvs_handle handle = 0;
    esp_err_t res = nvs_open(NVS_FLASH_PARTITION, NVS_READWRITE, &handle);
    if (res != ESP_OK)
        return false;

    uint8_t devaddr_buf[4] = {(global_devaddr >> 24) & 0xFF, (global_devaddr >> 16) & 0xFF,
                              (global_devaddr >> 8) & 0xFF, global_devaddr & 0xFF};

    if (!write_nvs_value(handle, NVS_FLASH_KEY_DEVADDR, devaddr_buf, 4))
        goto done;
    if (!write_nvs_value(handle, NVS_FLASH_KEY_NWKSKEY, global_nwkskey, 16))
        goto done;
    if (!write_nvs_value(handle, NVS_FLASH_KEY_APPSKEY, global_appskey, 16))
        goto done;

    nvs_commit(handle);
    ESP_LOGI(TAG, "ABP Keys saved in NVS storage");

done:
    nvs_close(handle);
    return true;
}

bool lorawan_abp_provisioning_restore_keys(bool silent)
{
    uint8_t buf_devaddr[4];
    uint8_t buf_nwkskey[16];
    uint8_t buf_appskey[16];

    nvs_handle handle = 0;
    esp_err_t res = nvs_open(NVS_FLASH_PARTITION, NVS_READONLY, &handle);
    if (res != ESP_OK)
        return false;

    if (!read_nvs_value(handle, NVS_FLASH_KEY_DEVADDR, buf_devaddr, 4, silent))
        goto done;
    if (!read_nvs_value(handle, NVS_FLASH_KEY_NWKSKEY, buf_nwkskey, 16, silent))
        goto done;
    if (!read_nvs_value(handle, NVS_FLASH_KEY_APPSKEY, buf_appskey, 16, silent))
        goto done;

    global_devaddr = (buf_devaddr[0] << 24) | (buf_devaddr[1] << 16) | (buf_devaddr[2] << 8) | buf_devaddr[3];
    memcpy(global_nwkskey, buf_nwkskey, 16);
    memcpy(global_appskey, buf_appskey, 16);

    have_keys = (global_devaddr != 0);

    if (have_keys && !silent)
    {
        ESP_LOGI(TAG, "ABP Keys restored from NVS storage");
    }

done:
    nvs_close(handle);
    return true;
}

bool read_nvs_value(nvs_handle handle, const char *key, uint8_t *data, size_t expected_length, bool silent)
{
    size_t size = expected_length;
    esp_err_t res = nvs_get_blob(handle, key, data, &size);
    if (res == ESP_OK && size == expected_length)
        return true;
    return false;
}

bool write_nvs_value(nvs_handle handle, const char *key, const uint8_t *data, size_t len)
{
    esp_err_t res = nvs_set_blob(handle, key, data, len);
    return res == ESP_OK;
}

bool hex_str_to_bin(const char *hex, uint8_t *buf, int len)
{
    const char *ptr = hex;
    for (int i = 0; i < len; i++)
    {
        int val = hex_tuple_to_byte(ptr);
        if (val < 0)
            return false;
        buf[i] = val;
        ptr += 2;
    }
    return true;
}

int hex_tuple_to_byte(const char *hex)
{
    int n1 = hex_digit_to_val(hex[0]);
    int n2 = hex_digit_to_val(hex[1]);
    if (n1 < 0 || n2 < 0)
        return -1;
    return (n1 << 4) | n2;
}

int hex_digit_to_val(char ch)
{
    if (ch >= '0' && ch <= '9')
        return ch - '0';
    if (ch >= 'A' && ch <= 'F')
        return ch + 10 - 'A';
    if (ch >= 'a' && ch <= 'f')
        return ch + 10 - 'a';
    return -1;
}

// --- LMIC core linker callbacks (Required by LMIC even in ABP mode) ---

void os_getArtEui(u1_t *buf)
{
    memset(buf, 0, 8);
}

void os_getDevEui(u1_t *buf)
{
    memset(buf, 0, 8);
}

void os_getDevKey(u1_t *buf)
{
    memset(buf, 0, 16);
}
