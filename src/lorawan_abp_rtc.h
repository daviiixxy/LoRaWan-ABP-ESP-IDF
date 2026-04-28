/******************************************************************************
 * Copyright © 2026 HUB I4.0 - Universidade do Estado do Amazonas
 * All rights reserved
 *****************************************************************************
 * Functions for storing and retrieving LoRaWAN ABP communication state from RTC memory.
 */

#pragma once

#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif

    void lorawan_abp_rtc_save();
    bool lorawan_abp_rtc_restore();

#ifdef __cplusplus
}
#endif
