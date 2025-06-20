
[![Powered by DartNode](https://dartnode.com/branding/DN-Open-Source-sm.png)](https://dartnode.com "Powered by DartNode - Free VPS for Open Source")

# 🟠 Stateless Bitcoin Wallet Generator  
### for ESP32-2432S028 / 2432S028R + SparkFun ATECC508A

> A fully airgapped, display-only Bitcoin wallet generator powered by **real hardware entropy** — not sketchy pseudorandom numbers.

---

## 👁️‍🗨️ Why This Exists

Cold wallets are sacred.  
The tools you use to generate them should be too.

This project gives you a **stateless**, **offline**, and **tamper-resistant** device to generate:
- A 24-word **BIP39 mnemonic seed**
- A **native SegWit (bech32)** Bitcoin address

It runs fully airgapped and leaves no trace.

- 🛑 No WiFi  
- 🛑 No SD card  
- 🛑 No persistent memory  
- ✅ No compromises

You get **one screen**, **one button**, and pure Bitcoin key material.

---

## 🧠 What It Does

- 🔐 Pulls **256 bits of entropy** from a [SparkFun ATECC508A](https://www.sparkfun.com/sparkfun-cryptographic-co-processor-breakout-atecc508a-qwiic.html)
- 🧠 Converts it into a **24-word BIP39 seed phrase**
- 🧭 Derives a native SegWit address at `m/84'/0'/0'/0/0`
- 📺 Displays the mnemonic + address on a 2.8" SPI TFT screen
- 🔁 Press **BOOT** to generate a new wallet
- 🧼 Old data is wiped from RAM — nothing is saved

> ✨ If you don’t write it down, it’s gone. That’s the idea.

---

## 🔐 Why We Don't Use `esp_random()`

ESP32’s built-in `esp_random()` is **not safe for cryptographic key generation**:

### ⚠️ Real Vulnerabilities:
 CVE‑2025‑27840 +
 
According to Espressif documentation:

The hardware RNG only produces true randomness when one of the following is active:
Wi‑Fi or Bluetooth is enabled
bootloader_random_enable() is called (prior to disabling for ADC or RF use)
During the second-stage bootloader 


If none of those are true, esp_random() falls back to pseudo-random output—predictable under certain conditions

### ❌ Technical Limitations:
- Software-seeded PRNG with unknown entropy state at boot
- Susceptible to low-entropy boot and reseeding edge cases
- Not suitable for irreversible cryptographic generation (like Bitcoin keys)

We use a real hardware random number generator instead.

---

## ✅ Why the ATECC508A?

The [SparkFun ATECC508A breakout](https://www.sparkfun.com/sparkfun-cryptographic-co-processor-breakout-atecc508a-qwiic.html) is a secure element with:

- ✅ True **hardware random number generator** (TRNG)
- ✅ Tamper resistance and side-channel protection
- ✅ NIST-certified entropy
- ✅ Proven use in HSMs, secure boot, hardware wallets, and identity tokens

It’s the ideal chip for generating **unguessable private keys** offline.

---

## 🔧 Hardware Compatibility

| Component                  | Description                                                                                      |
|----------------------------|--------------------------------------------------------------------------------------------------|
| **ESP32-2432S028**         | TFT + ESP32 combo board (SPI screen, no rotary)                                                  |
| **ESP32-2432S028R**        | Variant with rotary encoder + touchscreen — fully compatible ([Buy](https://www.amazon.com/AITRIP-Development-ESP32-2432S028R-Bluetooth-240X320/dp/B0CKYVPWX9/)) |
| **SparkFun ATECC508A**     | [Qwiic breakout](https://www.sparkfun.com/sparkfun-cryptographic-co-processor-breakout-atecc508a-qwiic.html) wired via I2C (GPIO27 / GPIO22) |
| **BOOT Button (GPIO0)**    | Press to regenerate wallet                                                                      |

> ✅ Works identically on both `S028` and `S028R` variants — rotary and touchscreen are unused.

Thanks to [Macsbug](https://macsbug.wordpress.com/2022/08/17/esp32-2432s028/) for excellent pinout docs.

---

## 💾 Software Stack

- [`uBitcoin`](https://docs.arduino.cc/libraries/ubitcoin/) — BIP39/32/84 HD wallet logic
- [`SparkFun ATECCX08A`](https://github.com/sparkfun/SparkFun_ATECCX08a_Arduino_Library) — secure entropy driver
- [`TFT_eSPI`](https://github.com/Bodmer/TFT_eSPI) — fast SPI TFT display library
- [`TP_Arduino_DigitalRain_Anim`](https://github.com/0015/TP_Arduino_DigitalRain_Anim) — Matrix animation

All installable via Arduino IDE Library Manager.

---

## 🚀 How It Works

1. Power on → shows logo + boot sequence  
2. Press **BOOT button**  
3. Fetches 256-bit entropy from ATECC508A  
4. Generates:
   - 24-word BIP39 mnemonic
   - bech32 Bitcoin address  
5. Displays result on-screen  
6. Press BOOT again to wipe and regenerate

---

## 🔌 Wiring Overview

| Device        | Pin      | Description         |
|---------------|----------|---------------------|
| ATECC508A SDA | GPIO27   | I2C data line       |
| ATECC508A SCL | GPIO22   | I2C clock line      |
| TFT Display   | SPI bus  | Prewired on board   |
| BOOT Button   | GPIO0    | Trigger input       |

---

donate BTC if you like it `bc1qq9rtrc7ly3e77s30f9es7k2x79qd4y0h2ev06x`
