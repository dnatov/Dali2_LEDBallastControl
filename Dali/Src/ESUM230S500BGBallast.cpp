//
// Created by danan on 2025-11-11.
//

#include "ESUM230S500BGBallast.h"

namespace Carendes::Dali
{
    namespace
    {
        constexpr uint8_t AcVoltageBytes = 2;
        constexpr uint8_t AcPowerBytes = 4;

        float DecodeAcVoltage(uint16_t rawDeciVolts)
        {
            return static_cast<float>(rawDeciVolts) / 10.0f;
        }

        float DecodeAcPower(uint32_t rawDeciWatts)
        {
            return static_cast<float>(rawDeciWatts) / 10.0f;
        }
    }

    unsigned char ESUM230S500BGBallast::SelectDiagnosticBank(DiagnosticBank bank)
    {
        return SetDataTransferRegister1(static_cast<uint8_t>(bank));
    }

    unsigned char ESUM230S500BGBallast::SelectDiagnosticOffset(uint8_t offset)
    {
        return SetDataTransferRegister(offset);
    }

    unsigned char ESUM230S500BGBallast::StartDiagnosticMemoryQuery()
    {
        return SendCommand(REG_DIAG_MEM_LOC);
    }

    bool ESUM230S500BGBallast::ReadMeasurement(Measurement measurement, uint32_t& rawValue)
    {
        if (_activeMeasurement != Measurement::None && _activeMeasurement != measurement)
            return false;

        if (_activeMeasurement == Measurement::None)
        {
            _activeMeasurement = measurement;
            _readStep = ReadStep::SelectBank;
            _bytesToRead = measurement == Measurement::AcVoltage ? AcVoltageBytes : AcPowerBytes;
            _bytesRead = 0;
            _rawValue = 0;
        }

        if (_readStep == ReadStep::ReadByte && Bus().HasReceivedData())
        {
            _rawValue = (_rawValue << 8) | FetchResponse();
            _bytesRead++;

            if (_bytesRead >= _bytesToRead)
            {
                rawValue = _rawValue;
                ResetMeasurementRead();
                return true;
            }
        }

        if (!Bus().IsIdle())
            return false;

        if (_readStep == ReadStep::SelectBank)
        {
            const auto bank = measurement == Measurement::AcVoltage
                                  ? DiagnosticBank::Bank4
                                  : DiagnosticBank::Bank1;

            if (!SelectDiagnosticBank(bank))
                return false;

            _readStep = ReadStep::SelectOffset;
            return false;
        }

        if (_readStep == ReadStep::SelectOffset)
        {
            const auto offset = measurement == Measurement::AcVoltage
                                    ? OFFS_B4_AC_INPUT_VOLTAGE
                                    : OFFS_B1_OUTPUT_POWER_ACTIVE;

            if (!SelectDiagnosticOffset(offset))
                return false;

            _readStep = ReadStep::ReadByte;
            return false;
        }

        StartDiagnosticMemoryQuery();
        return false;
    }

    void ESUM230S500BGBallast::ResetMeasurementRead()
    {
        _activeMeasurement = Measurement::None;
        _readStep = ReadStep::SelectBank;
        _bytesToRead = 0;
        _bytesRead = 0;
        _rawValue = 0;
    }

    bool ESUM230S500BGBallast::TryGetAcVoltage(float& voltage)
    {
        uint32_t rawValue = 0;

        if (!ReadMeasurement(Measurement::AcVoltage, rawValue))
            return false;

        voltage = DecodeAcVoltage(static_cast<uint16_t>(rawValue));
        return true;
    }

    bool ESUM230S500BGBallast::TryGetAcPower(float& power)
    {
        uint32_t rawValue = 0;

        if (!ReadMeasurement(Measurement::AcPower, rawValue))
            return false;

        power = DecodeAcPower(rawValue);
        return true;
    }
}
