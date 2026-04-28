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
