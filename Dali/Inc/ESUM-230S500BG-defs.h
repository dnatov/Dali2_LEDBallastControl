//
// Created by danan on 2025-11-11.
//

#pragma once

/* =========================
 * Base / Initial Registers
 * ========================= */
#define REG_DIAG_BANK1          0xCA  /* Diagnostic Bank 1 */
#define REG_DIAG_BANK2          0xCB  /* Diagnostic Bank 2 */
#define REG_DIAG_BANK3          0xCC  /* Diagnostic Bank 3 */
#define REG_DIAG_BANK4          0xCD  /* Diagnostic Bank 4 */
#define REG_DIAG_BANK5          0xCE  /* Diagnostic Bank 5 */

#define REG_DTR                 0xA3  /* Data Transfer Register */
#define REG_DTR1                0xC3  /* Data Transfer Register 1 */
#define REG_DIAG_MEM_LOC        0xC5  /* Diagnostic Memory Location */

/* =========================
 * Diagnostic Bank 1 (0xCA)
 * ========================= */
#define OFFS_B1_OUTPUT_POWER_ACTIVE     0x0C  /* 4 bytes, 0.1 W (may return 1 byte) */

/* =========================
 * Diagnostic Bank 2 (0xCB)
 * ========================= */
#define OFFS_B2_OUTPUT_POWER_REACTIVE   0x0C  /* 4 bytes, 0.1 VA (may return 1 byte) */

/* =========================
 * Diagnostic Bank 3 (0xCC)
 * ========================= */
#define OFFS_B3_OUTPUT_POWER            0x0C  /* 4 bytes, 0.1 W (may return 1 byte) */

/* =========================
 * Diagnostic Bank 4 (0xCD)
 * ========================= */
#define OFFS_B4_TOTAL_RUNTIME           0x04  /* 4 bytes, seconds */
#define OFFS_B4_AC_INPUT_VOLTAGE        0x0B  /* 2 bytes, 0.1 V */
#define OFFS_B4_AC_FREQUENCY            0x0D  /* 1 byte, Hz */
#define OFFS_B4_POWER_FACTOR            0x0E  /* 1 byte, ≤ 100 */
#define OFFS_B4_INTERNAL_TEMP           0x1B  /* 1 byte, °C (shifted +60 °C) */

/* =========================
 * Diagnostic Bank 5 (0xCE)
 * ========================= */
#define OFFS_B5_RESETTABLE_LIGHT_TIME   0x0A  /* 4 bytes, seconds (resettable) */
#define OFFS_B5_CUMULATIVE_LIGHT_TIME   0x0E  /* 4 bytes, seconds */
#define OFFS_B5_OUTPUT_VOLTAGE          0x12  /* 2 bytes, 0.1 V */
#define OFFS_B5_OUTPUT_CURRENT          0x14  /* 2 bytes, mA */
#define OFFS_B5_EXTERNAL_TEMP           0x20  /* 1 byte, °C (shifted +60 °C) */

enum Dali_Data_Diag_Query
{
 ActivePower = OFFS_B1_OUTPUT_POWER_ACTIVE,
 ReactivePower = OFFS_B2_OUTPUT_POWER_REACTIVE,
 RealPower = OFFS_B3_OUTPUT_POWER,
 TotalRunTime = OFFS_B4_TOTAL_RUNTIME,
 AcInputVoltage = OFFS_B4_AC_INPUT_VOLTAGE,
 AcFrequency = OFFS_B4_AC_FREQUENCY,
 PowerFactor = OFFS_B4_POWER_FACTOR,
 InternalTemperature = OFFS_B4_INTERNAL_TEMP,
 ResettableLightTimeOn = OFFS_B5_RESETTABLE_LIGHT_TIME,
 CumulativeLightTimeOn = OFFS_B5_CUMULATIVE_LIGHT_TIME,
 OutputVoltage = OFFS_B5_OUTPUT_VOLTAGE,
 OutputCurrent = OFFS_B5_OUTPUT_CURRENT,
 ExternalTemperature = OFFS_B5_EXTERNAL_TEMP
};