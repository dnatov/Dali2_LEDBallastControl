# Introduction

This repository contains code for operating an **Inventronics LED Ballast (ESUM-230S500BG)** on a custom **DALI-2** bus implementation.

---

## 🧰 Hardware Used

- **Inventronics CPR30+** – for NFC programming  
- **Inventronics PRG-MUL2** – for basic DALI commands  
- **STM32F401 NUCLEO** (or most STM32 MCUs)  
- **Saleae Logic Pro 8** – for debugging and signal analysis  
- [**DALI-2 Click**](https://www.mikroe.com/dali-2-click?srsltid=AfmBOoq4ZUR7_ctMxmy2nqPthuts0VC9PJ3BMXPmYJjt4CPfqWMUKpMS)

---

## 🔍 Debugging the DALI Bus

Debugging a DALI-2 bus is best done using a **logic analyzer** that supports **Manchester decoding** and can tolerate approximately **12 V** on its input pins.  
The **Saleae Logic Pro 8** works perfectly for this.

### Hardware Connection

1. Connect **two 1.5 kΩ resistors** in series between the two DALI bus lines (at your programmer or board).  
2. Connect:
   - One DALI input to **GND** on your logic analyzer  
   - The midpoint between the resistors to your **logic input**

### Analyzer Settings

| Parameter | Setting |
|------------|----------|
| Bit rate | 1200 bit/s |
| Encoding | Manchester |
| Edge polarity | Negative edge = 0 |
| Bits per frame | 16 (responses are 8 bits) |
| Bit order | MSB first |
| Start bit | First bit |
| Optional filters | 20 µs glitch filter and/or 25 % period tolerance |

These optional filters may improve results if your analyzer struggles to interpret the waveform.

---

## 💡 DALI Control Basics

DALI commands are 16-bit operations in the form:

```
(Address) - (Command)
```

- **Address range:** `0x00` – `0x3F` (64 individual devices)  
- **Broadcast address:** `0xFF` (all ballasts)  
- **Group addressing:** allows collective control of multiple ballasts.

Because each DALI-2 transaction is limited to 16 bits, many commands use a **DTR (Data Transfer Register)** to temporarily store data used by subsequent commands.

---

## ⚙️ Special Commands

Special commands extend functionality by using additional DTR registers. These commands are typically specific to the ballast model.

**Format:**

To read an offset value from a diagnostic bank you need to use this format. It uses both DTR registers and the Diagnostic Memory Location (See Registers Below).

1. 0xC3 0x(*Diagnostic Bank #*)
2. 0xA3 0x(*Offset Value*)
3. (*Short/Broadcast Address*) 0xC5
4. Byte 1
5. *Repeat last command for more bytes...*

**Example:**

```
0xC3 (DTR1) → 0xCE (Diagnostic Bank 1)
0xA3 (DTR)  → 0x14 (Offset: Real-Time Current in mA)
0xFF (Broadcast) → 0xC5 (Read Diagnostic Memory)
Byte 1...
```

For multi-byte reads, repeatedly read the diagnostic bank to retrieve less significant bytes.

---

## 🗃️ Diagnostic Registers

| Register | Description |
|----------|--------------|
| `0xCA`   | Diagnostic Bank 1 |
| `0xCB`   | Diagnostic Bank 2 |
| `0xCC`   | Diagnostic Bank 3 |
| `0xCD`   | Diagnostic Bank 4 |
| `0xCE`   | Diagnostic Bank 5 |
| `0xA3`   | DTR |
| `0xC3`   | DTR1 |
| `0xC5`   | Diagnostic Memory Location |

---

### 🧾 Diagnostic Bank 1 (`0xCA`)

| Parameter | Offset | Size | Units | Notes |
|------------|---------|------|--------|--------|
| Output Power (Active) | `0x0C` | 4 bytes | 0.1 W | May Require Doing This Before -> `0xC3CA → 0xA30B → 0x01C3` -> which returns 1 byte |

---

### 🧾 Diagnostic Bank 2 (`0xCB`)

| Parameter | Offset | Size | Units | Notes                                                                               |
|------------|---------|------|--------|-------------------------------------------------------------------------------------|
| Output Power (Reactive) | `0x0C` | 4 bytes | 0.1 VA | May Require Doing This Before -> `0xC3CB → 0xA30B → 0x01C5` -> which returns 1 byte |

---

### 🧾 Diagnostic Bank 3 (`0xCC`)

| Parameter | Offset | Size | Units | Notes |
|------------|---------|------|--------|--------|
| Output Power | `0x0C` | 4 bytes | 0.1 W | May Require Doing This Before -> `0xC3CC → 0xA30B → 0x01C5` -> which returns 1 byte |

---

### 🧾 Diagnostic Bank 4 (`0xCD`)

| Parameter | Offset | Size | Units | Notes |
|------------|---------|------|--------|--------|
| Internal Temperature | `0x1B` | 1 byte | °C | Output shifted +60 °C |
| Total Runtime | `0x04` | 4 bytes | seconds | — |
| Power Factor | `0x0E` | 1 byte | ≤ 100 | — |
| AC Input Voltage | `0x0B` | 2 bytes | 0.1 V | — |
| AC Frequency | `0x0D` | 1 byte | Hz | — |

---

### 🧾 Diagnostic Bank 5 (`0xCE`)

| Parameter | Offset | Size | Units | Notes |
|------------|---------|------|--------|--------|
| External Temperature | `0x20` | 1 byte | °C | Output shifted +60 °C |
| Output Current | `0x14` | 2 bytes | mA | — |
| Output Voltage | `0x12` | 2 bytes | 0.1 V | — |
| Cumulative Lighting Time | `0x0E` | 4 bytes | seconds | — |
| Resettable Lighting Time | `0x0A` | 4 bytes | seconds | Can be reset via command |
