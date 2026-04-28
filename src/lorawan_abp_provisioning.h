/******************************************************************************
 * Copyright © 2026 HUB I4.0 - Universidade do Estado do Amazonas
 * All rights reserved
 *****************************************************************************
 * Functions for provisioning LoRaWAN ABP communication.
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    void lorawan_abp_provisioning_init(void);

    bool lorawan_abp_provisioning_have_keys(void);
    bool lorawan_abp_provisioning_decode_keys(const char *devaddr_str, const char *nwkskey_str,
                                              const char *appskey_str);
    bool lorawan_abp_provisioning_save_keys(void);
    bool lorawan_abp_provisioning_restore_keys(bool silent);

    uint32_t lorawan_abp_get_devaddr(void);
    const uint8_t *lorawan_abp_get_nwkskey(void);
    const uint8_t *lorawan_abp_get_appskey(void);

#ifdef __cplusplus
}
#endif
