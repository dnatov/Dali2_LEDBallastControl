//
// Created by danan on 2025-11-11.
//
#pragma once

#include <cstdint>
#include "Dali2Device.h"
#include "ESUM-230S500BG-defs.h"

namespace Carendes::Dali
{
    class ESUM230S500BGBallast : public Dali2Device
    {
    private:
        enum class DiagnosticBank : uint8_t
        {
            Bank1 = REG_DIAG_BANK1,
            Bank4 = REG_DIAG_BANK4
        };

        enum class Measurement : uint8_t
        {
            None,
            AcVoltage,
            AcPower
        };

        enum class ReadStep : uint8_t
        {
            SelectBank,
            SelectOffset,
            ReadByte
        };

        Measurement _activeMeasurement = Measurement::None;
        ReadStep _readStep = ReadStep::SelectBank;
        uint8_t _bytesToRead = 0;
        uint8_t _bytesRead = 0;
        uint32_t _rawValue = 0;

        unsigned char SelectDiagnosticBank(DiagnosticBank bank);
        unsigned char SelectDiagnosticOffset(uint8_t offset);
        unsigned char StartDiagnosticMemoryQuery();
        bool ReadMeasurement(Measurement measurement, uint32_t& rawValue);
        void ResetMeasurementRead();

    public:
        using Dali2Device::Dali2Device;

        bool TryGetAcVoltage(float& voltage);
        bool TryGetAcPower(float& power);
    };
}
