/******************************************************************************
 * Copyright © 2026 HUB I4.0 - Universidade do Estado do Amazonas
 * All rights reserved
 *****************************************************************************
 * LoRaWan-ABP - Custom ABP LoRaWAN library for ESP-IDF / SX127x
 *
 * High-level C API for APB Class A.
 *******************************************************************************/

#pragma once

#include "driver/spi_master.h"
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define LORA_NOT_CONNECTED 0xff

    /**
     * @brief Integer data type for specifiying the port of an uplink or downlink message.
     */
    typedef uint8_t lora_port_t;

    /**
     * @brief Response codes
     */
    typedef enum
    {
        LORA_ERROR_TRANSMISSION_FAILED = -1,
        LORA_ERROR_UNEXPECTED = -10,
        LORA_SUCCESSFUL_TRANSMISSION = 1,
        LORA_SUCCESSFUL_RECEIVE = 2
    } lora_response_code_t;

    /**
     * @brief RX/TX windows
     */
    typedef enum
    {
        /**
         * @brief Outside of any window
         */
        LORA_WINDOW_IDLE = 0,
        /**
         * @brief TX window (up to RX1 window)
         */
        LORA_WINDOW_TX = 1,
        /**
         * @brief RX1 window (up to RX2 window)
         */
        LORA_WINDOW_RX1 = 2,
        /**
         * @brief RX2 window 2
         */
        LORA_WINDOW_RX2 = 3
    } lora_rx_tx_window_t;

    
    /**
     * @brief Spreading factors
     */
    typedef enum
    {
        /**
         * @brief Unused / undefined spreading factor
         */
        LORA_SF_NONE = 0,
        /**
         * @brief Frequency Shift Keying (FSK)
         */
        LORA_FSK = 1,
        /**
         * @brief Spreading Factor 7 (SF7)
         */
        LORA_SF7 = 2,
        /**
         * @brief Spreading Factor 8 (SF8)
         */
        LORA_SF8 = 3,
        /**
         * @brief Spreading Factor 9 (SF9)
         */
        LORA_SF9 = 4,
        /**
         * @brief Spreading Factor 10 (SF10)
         */
        LORA_SF10 = 5,
        /**
         * @brief Spreading Factor 11 (SF11)
         */
        LORA_SF11 = 6,
        /**
         * @brief Spreading Factor 12 (SF12)
         */
        LORA_SF12 = 7
    } lora_spreading_factor_t;
    
/**
     * @brief Bandwidth
     */
    typedef enum
    {
        /**
         * @brief Undefined/unused bandwidth
         */
        LORA_BW_NONE = 0,
        /**
         * @brief Bandwidth of 125 kHz
         */
        LORA_BW_125 = 1,
        /**
         * @brief Bandwidth of 250 kHz
         */
        LORA_BW_250 = 2,
        /**
         * @brief Bandwidth of 500 kHz
         */
        LORA_BW_500 = 3
    } lora_bandwidth_t;

    /**
     * @brief Data Rate
     *
     * Note that the spreading factor, bandwidth, bit rate and maximum message
     * size associated with each data rate depends on the region.
     */
    typedef enum
    {
        /**
         * @brief Data rate for region AS923 using SF12 and 125 kHz bandwidth.
         */
        LORA_DR_AS923_SF12 = 0,
        /**
         * @brief Data rate for region AS923 using SF11 and 125 kHz bandwidth.
         */
        LORA_DR_AS923_SF11 = 1,
        /**
         * @brief Data rate for region AS923 using SF10 and 125 kHz bandwidth.
         */
        LORA_DR_AS923_SF10 = 2,
        /**
         * @brief Data rate for region AS923 using SF9 and 125 kHz bandwidth.
         */
        LORA_DR_AS923_SF9 = 3,
        /**
         * @brief Data rate for region AS923 using SF8 and 125 kHz bandwidth.
         */
        LORA_DR_AS923_SF8 = 4,
        /**
         * @brief Data rate for region AS923 using SF7 and 125 kHz bandwidth.
         */
        LORA_DR_AS923_SF7_BW125 = 5,
        /**
         * @brief Data rate for region AS923 using SF7 and 250 kHz bandwidth.
         */
        LORA_DR_AS923_SF7_BW250 = 6,
        /**
         * @brief Data rate for region AS923 using FSK and 50 kpbs.
         */
        LORA_DR_AS923_FSK = 7,

        /**
         * @brief Data rate for region AU915 using SF12 and 125 kHz bandwidth.
         */
        LORA_DR_AU915_SF12 = 0,
        /**
         * @brief Data rate for region AU915 using SF11 and 125 kHz bandwidth.
         */
        LORA_DR_AU915_SF11 = 1,
        /**
         * @brief Data rate for region AU915 using SF10 and 125 kHz bandwidth.
         */
        LORA_DR_AU915_SF10 = 2,
        /**
         * @brief Data rate for region AU915 using SF9 and 125 kHz bandwidth.
         */
        LORA_DR_AU915_SF9 = 3,
        /**
         * @brief Data rate for region AU915 using SF8 and 125 kHz bandwidth.
         */
        LORA_DR_AU915_SF8 = 4,
        /**
         * @brief Data rate for region AU915 using SF7 and 125 kHz bandwidth.
         */
        LORA_DR_AU915_SF7 = 5,
        /**
         * @brief Data rate for region AU915 using SF8 and 500 kHz bandwidth.
         */
        LORA_DR_AU915_SF8_BW500 = 6,
        /**
         * @brief Data rate for region AU915 using SF12 and 500 kHz bandwidth.
         *
         * Reserved for future applications.
         */
        LORA_DR_AU915_SF12_BW500 = 8,
        /**
         * @brief Data rate for region AU915 using SF11 and 500 kHz bandwidth.
         *
         * Reserved for future applications.
         */
        LORA_DR_AU915_SF11_BW500 = 9,
        /**
         * @brief Data rate for region AU915 using SF10 and 500 kHz bandwidth.
         *
         * Reserved for future applications.
         */
        LORA_DR_AU915_SF10_BW500 = 10,
        /**
         * @brief Data rate for region AU915 using SF9 and 500 kHz bandwidth.
         *
         * Reserved for future applications.
         */
        LORA_DR_AU915_SF9_BW500 = 11,
        /**
         * @brief Data rate for region AU915 using SF8 and 500 kHz bandwidth.
         *
         * Reserved for future applications.
         */
        LORA_DR_AU915_SF8_BW500_DR12 = 12,
        /**
         * @brief Data rate for region AU915 using SF7 and 500 kHz bandwidth.
         *
         * Reserved for future applications.
         */
        LORA_DR_AU915_SF7_BW500 = 13,

        /**
         * @brief Data rate for region EU868 using SF12 and 125 kHz bandwidth.
         */
        LORA_DR_EU868_SF12 = 0,
        /**
         * @brief Data rate for region EU868 using SF11 and 125 kHz bandwidth.
         */
        LORA_DR_EU868_SF11 = 1,
        /**
         * @brief Data rate for region EU868 using SF10 and 125 kHz bandwidth.
         */
        LORA_DR_EU868_SF10 = 2,
        /**
         * @brief Data rate for region EU868 using SF9 and 125 kHz bandwidth.
         */
        LORA_DR_EU868_SF9 = 3,
        /**
         * @brief Data rate for region EU868 using SF8 and 125 kHz bandwidth.
         */
        LORA_DR_EU868_SF8 = 4,
        /**
         * @brief Data rate for region EU868 using SF7 and 125 kHz bandwidth.
         */
        LORA_DR_EU868_SF7_BW125 = 5,
        /**
         * @brief Data rate for region EU868 using SF7 and 250 kHz bandwidth.
         */
        LORA_DR_EU868_SF7_BW250 = 6,
        /**
         * @brief Data rate for region EU868 using FSK and 50 kpbs.
         */
        LORA_DR_EU868_FSK = 7,

        /**
         * @brief Data rate for region IN866 using SF12 and 125 kHz bandwidth.
         */
        LORA_DR_IN866_SF12 = 0,
        /**
         * @brief Data rate for region IN866 using SF11 and 125 kHz bandwidth.
         */
        LORA_DR_IN866_SF11 = 1,
        /**
         * @brief Data rate for region IN866 using SF10 and 125 kHz bandwidth.
         */
        LORA_DR_IN866_SF10 = 2,
        /**
         * @brief Data rate for region IN866 using SF9 and 125 kHz bandwidth.
         */
        LORA_DR_IN866_SF9 = 3,
        /**
         * @brief Data rate for region IN866 using SF8 and 125 kHz bandwidth.
         */
        LORA_DR_IN866_SF8 = 4,
        /**
         * @brief Data rate for region IN866 using SF7 and 125 kHz bandwidth.
         */
        LORA_DR_IN866_SF7 = 5,
        /**
         * @brief Data rate for region IN866 using FSK and 50 kpbs.
         */
        LORA_DR_IN866_FSK = 7,

        /**
         * @brief Data rate for region KR920 using SF12 and 125 kHz bandwidth.
         */
        LORA_DR_KR920_SF12 = 0,
        /**
         * @brief Data rate for region KR920 using SF11 and 125 kHz bandwidth.
         */
        LORA_DR_KR920_SF11 = 1,
        /**
         * @brief Data rate for region KR920 using SF10 and 125 kHz bandwidth.
         */
        LORA_DR_KR920_SF10 = 2,
        /**
         * @brief Data rate for region KR920 using SF9 and 125 kHz bandwidth.
         */
        LORA_DR_KR920_SF9 = 3,
        /**
         * @brief Data rate for region KR920 using SF8 and 125 kHz bandwidth.
         */
        LORA_DR_KR920_SF8 = 4,
        /**
         * @brief Data rate for region KR920 using SF7 and 125 kHz bandwidth.
         */
        LORA_DR_KR920_SF7 = 5,

        /**
         * @brief Data rate for region US915 using SF10 and 125 kHz bandwidth.
         */
        LORA_DR_US915_SF10 = 0,
        /**
         * @brief Data rate for region US915 using SF9 and 125 kHz bandwidth.
         */
        LORA_DR_US915_SF9 = 1,
        /**
         * @brief Data rate for region US915 using SF8 and 125 kHz bandwidth.
         */
        LORA_DR_US915_SF8 = 2,
        /**
         * @brief Data rate for region US915 using SF7 and 125 kHz bandwidth.
         */
        LORA_DR_US915_SF7 = 3,
        /**
         * @brief Data rate for region US915 using SF8 and 500 kHz bandwidth.
         */
        LORA_DR_US915_SF8_BW500 = 4,
        /**
         * @brief Data rate for region US915 using SF12 and 500 kHz bandwidth.
         *
         * Reserved for future applications.
         */
        LORA_DR_US915_SF12_BW500 = 8,
        /**
         * @brief Data rate for region US915 using SF11 and 500 kHz bandwidth.
         *
         * Reserved for future applications.
         */
        LORA_DR_US915_SF11_BW500 = 9,
        /**
         * @brief Data rate for region US915 using SF10 and 500 kHz bandwidth.
         *
         * Reserved for future applications.
         */
        LORA_DR_US915_SF10_BW500 = 10,
        /**
         * @brief Data rate for region US915 using SF9 and 500 kHz bandwidth.
         *
         * Reserved for future applications.
         */
        LORA_DR_US915_SF9_BW500 = 11,
        /**
         * @brief Data rate for region US915 using SF8 and 500 kHz bandwidth.
         *
         * Reserved for future applications.
         */
        LORA_DR_US915_SF8_BW500_DR12 = 12,
        /**
         * @brief Data rate for region US915 using SF7 and 500 kHz bandwidth.
         *
         * Reserved for future applications.
         */
        LORA_DR_US915_SF7_BW500 = 13,

        /**
         * @brief Default data rate for joining.
         */
        LORA_DR_JOIN_DEFAULT = 255
    } lora_data_rate_t;

    /**
     * @brief Callback for recieved messages
     *
     * @param payload  pointer to the received bytes
     * @param length   number of received bytes
     * @param port     port the message was received on
     */
    typedef void (*lora_message_cb)(const uint8_t *payload, size_t length, lora_port_t port);

    /**
     * @brief Initializes the LoRa device instance.
     *
     * Call this function once at the start of the program.
     */
    void lorawan_abp_init(void);

    /**
     * @brief Configures the pins used to communicate with the LoRaWAN radio chip.
     */
    void lorawan_abp_configure_pins(spi_host_device_t spi_host, uint8_t nss, uint8_t rxtx, uint8_t rst, uint8_t dio0,
                                    uint8_t dio1);

    /**
     * @brief Sets the frequency sub-band to be used.
     *
     * For regions with sub-bands (AU915, US915), sets the sub-band
     * for uplink communication. For other regions, this function has no effect.
     *
     * Must be called before lorawan_abp_setup().
     * If not set, defaults to sub-band 2.
     *
     * @param band band (0 for all bands, or value between 1 and 8)
     */
    void lora_set_subband(int band);

    /**
     * @brief Sets the data rate to be used for transmission.
     *
     * Can be called before or after lorawan_abp_setup().
     * If called after, takes effect immediately.
     *
     * @param dr data rate (use region-specific LORA_DR_* constants)
     */
    void lora_set_data_rate(lora_data_rate_t dr);

    /**
     * @brief Sets the maximum TX power.
     *
     * Can be called before or after lorawan_abp_setup().
     * If called after, takes effect immediately.
     *
     * @param tx_pow maximum TX power in dBm (e.g. 14)
     */
    void lora_set_max_tx_power(int tx_pow);

    /**
     * @brief Returns whether Adaptive Data Rate (ADR) is currently enabled.
     * @return true if ADR is enabled
     */
    bool lora_adr_enabled(void);

    /**
     * @brief Enables or disables Adaptive Data Rate (ADR).
     *
     * When enabled, the network server can adjust SF and TX power.
     *
     * @param enabled true to enable, false to disable
     */
    void lora_set_adr_enabled(bool enabled);

    /**
     * @brief Starts the session via ABP (Activation by Personalization).
     *
     * @param netid    Network ID
     * @param devaddr  Device Address
     * @param app_skey Application Session Key (16 bytes)
     * @param nwk_skey Network Session Key (16 bytes)
     * @return true if successfully initiated.
     */
    bool lorawan_abp_setup(uint32_t netid, uint32_t devaddr, const uint8_t *nwk_skey, const uint8_t *app_skey);

    /**
     * @brief Transmits a message using Class A.
     *
     * @param payload  bytes to be transmitted
     * @param length   number of bytes to be transmitted
     * @param port     port (use 1 as default)
     * @param confirm  flag indicating if a confirmation should be requested (use `false` as default)
     * @return LORA_SUCCESSFUL_TRANSMISSION for successful transmission
     */
    lora_response_code_t lorawan_abp_transmit_message(const uint8_t *payload, size_t length, lora_port_t port,
                                                      bool confirm);

    /**
     * @brief Sets the function to be called when a message is received (Class A downlinks).
     *
     * @param callback  the callback function
     */
    void lorawan_abp_on_message(lora_message_cb callback);

    /**
     * @brief Prepares the system for deep sleep (saves LMIC state to RTC mem)
     */
    void lorawan_abp_prepare_for_deep_sleep(void);

    /**
     * @brief Prepares the system for power off (saves LMIC state to NVS)
     */
    void lorawan_abp_prepare_for_power_off(void);

    /**
     * @brief Resumes the session after a deep sleep (loads LMIC state from RTC mem)
     * @return true if successfully loaded
     */
    bool lorawan_abp_resume_after_deep_sleep(void);

    /**
     * @brief Resumes the session after a power off (loads LMIC state from NVS)
     * @param off_duration expected off duration in seconds
     * @return true if successfully loaded
     */
    bool lorawan_abp_resume_after_power_off(int off_duration);

#ifdef __cplusplus
}
#endif
