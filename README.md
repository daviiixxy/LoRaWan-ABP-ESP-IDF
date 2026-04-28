# LoRaWan-ABP

**Biblioteca autoral de conectividade LoRaWAN via ABP para ESP32 com ESP-IDF.**

Desenvolvida como componente nativo do ESP-IDF em C puro, encapsulando o stack LMIC (MCCI LoRaWAN LMIC library) com uma API simplificada e focada exclusivamente no modo de ativação **ABP (Activation By Personalization)**.

> Criada pelo time HUB I4.0 — Universidade do Estado do Amazonas (UEA) como parte do Projeto Aurora.

---

## ✨ Características

- **ABP nativo** — sem handshake OTAA, sessão iniciada imediatamente no boot
- **Persistência de Frame Counters** via NVS (flash) e RTC (deep sleep)
- **API simples e desacoplada** — o aplicativo não precisa conhecer o LMIC
- **Frequência configurável** via `menuconfig` (AU915, US915, EU868 e mais)
- **Thread-safe** — HAL com seções críticas para uso do rádio em FreeRTOS
- **C puro** — sem dependências de C++ ou Arduino

---

## 📁 Estrutura

```
lib/LoRaWan-ABP/
├── include/
│   └── lorawan_abp.h                       # API pública da biblioteca
├── src/
│   ├── lorawan_abp.c                       # Implementação principal
│   ├── lorawan_abp_nvs.c/h                 # Persistência em Flash (NVS)
│   ├── lorawan_abp_rtc.c/h                 # Persistência em RTC memory (deep sleep)
│   ├── lorawan_abp_logging.c/h             # Log de eventos LMIC
│   ├── lorawan_abp_provisioning.c/h        # Gerenciamento de chaves ABP
│   ├── esp_idf_lmic_config.h               # Mapeamento Kconfig → LMIC defines
│   ├── hal/
│   │   ├── hal_esp32.c/h                   # HAL do ESP32 para o LMIC
│   ├── lmic/                               # Stack LMIC (MCCI) — não editar
│   └── aes/                                # Implementação AES via mbedTLS
├── Kconfig                                 # Opções do menuconfig
└── CMakeLists.txt
```

---

## 🚀 Como usar

### 1. Configurar credenciais ABP

No arquivo `Config/Defaults.h` do seu projeto:

```c
#define LORAWAN_ABP_NETID   0x000013       // NetID da sua rede (ex: ChirpStack)
#define LORAWAN_ABP_DEVADDR 0x26XXXXXX     // Device Address (hex)
#define LORAWAN_ABP_NWKSKEY { 0x2B, 0x7E, ... } // Network Session Key (16 bytes)
#define LORAWAN_ABP_APPSKEY { 0x3C, 0xF1, ... } // Application Session Key (16 bytes)
```

### 2. Inicializar no código

```c
#include "lorawan_abp.h"

void app_main(void) {
    // Inicializa a seção crítica interna
    lorawan_abp_init();

    // Configura os pinos do rádio SX1276 (T-Beam)
    lorawan_abp_configure_pins(SPI2_HOST, 18, 0xFF, 23, 26, 33);

    // Registra callback de downlink (opcional)
    lorawan_abp_on_message(my_downlink_handler);

    // Inicializa sessão ABP com as chaves
    uint8_t nwk_skey[16] = LORAWAN_ABP_NWKSKEY;
    uint8_t app_skey[16] = LORAWAN_ABP_APPSKEY;
    lorawan_abp_setup(LORAWAN_ABP_NETID, LORAWAN_ABP_DEVADDR, nwk_skey, app_skey);

    // Envia um payload (porta 1, sem confirmação)
    uint8_t payload[] = { 0x01, 0x02, 0x03 };
    lorawan_abp_transmit_message(payload, sizeof(payload), 1, false);
}
```

### 3. Configurar frequência via menuconfig

```bash
idf.py menuconfig
# Component config → LoRaWan-ABP → LoRaWAN frequency → AU915
```

---

## 🔋 Deep Sleep e Power Off

A biblioteca preserva o estado da sessão (Frame Counters) para evitar Replay Attacks após reboots.

```c
// Antes de entrar em deep sleep (salva em RTC memory)
lorawan_abp_prepare_for_deep_sleep();
esp_deep_sleep_start();

// No boot seguinte, restaura a sessão da RTC memory
lorawan_abp_resume_after_deep_sleep();

// Antes de desligar completamente (salva em NVS/Flash)
lorawan_abp_prepare_for_power_off();

// No boot seguinte, restaura do NVS com compensação de tempo
lorawan_abp_resume_after_power_off(off_duration_seconds);
```

---

## ⚙️ Configuração do ChirpStack (ABP)

| Campo                | Valor                        |
|----------------------|------------------------------|
| Device Profile       | OTAA: **desabilitado**       |
| MAC version          | LoRaWAN 1.0.2                |
| RX1 Delay            | `1`                          |
| RX1 Data-rate offset | `0`                          |
| RX2 Data-rate        | `8` (AU915 padrão)           |
| RX2 Frequency        | `923300000` Hz               |
| Uplink Frame-counter | `0` (reset junto com a placa)|

Na aba **Activation** do Device, preencha:
- **Device Address** — mesmo valor de `LORAWAN_ABP_DEVADDR`
- **NwkSKey** / **AppSKey** — mesmos arrays de `Defaults.h`

---

## 📡 Sub-bandas AU915

O Brasil usa majoritariamente a **Sub-banda 1** (canais 915.2–916.6 MHz).  
Ajuste em `lorawan_abp.c` → `lorawan_abp_setup()`:

```c
LMIC_selectSubBand(0);  // Sub-banda 1 → 915.2 ~ 916.6 MHz
// LMIC_selectSubBand(1); // Sub-banda 2 → 916.8 ~ 918.2 MHz (TTN Brasil)
```

---

## 🛠️ Dependências

- **ESP-IDF** v4.4+
- **FreeRTOS** (incluso no ESP-IDF)
- **nvs_flash** (incluso no ESP-IDF)
- **mbedTLS** (incluso no ESP-IDF)
- **MCCI LMIC** (incluso em `src/lmic/` — não é submodule externo)

---

## 📄 Licença

O código autoral desta biblioteca está sob licença **MIT**.  
O stack LMIC em `src/lmic/` é copyright (C) MCCI Corporation, licenciado sob MIT.  
A HAL para ESP32 em `src/hal/` é derivada originalmente do projeto `ttn-esp32` de Manuel Bleichenbacher, adaptada e renomeada.
