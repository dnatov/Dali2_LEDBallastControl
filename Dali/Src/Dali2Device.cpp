//
// Created by danan on 2025-11-11.
//

#include "Dali2Device.h"

namespace Carendes::Dali
{
    Dali2Device::Dali2Device(Dali2Bus& bus,
                             uint8_t address,
                             DaliAddressMode addressMode)
        : _bus(bus),
          _address(address),
          _addressMode(addressMode)
    {
    }

    Dali2Bus& Dali2Device::Bus()
    {
        return _bus;
    }

    const Dali2Bus& Dali2Device::Bus() const
    {
        return _bus;
    }

    uint8_t Dali2Device::Address() const
    {
        return _address;
    }

    DaliAddressMode Dali2Device::AddressMode() const
    {
        return _addressMode;
    }

    uint8_t Dali2Device::commandAddress() const
    {
        return _addressMode == DaliAddressMode::Broadcast ? BROADCAST_CMD : _address;
    }

    uint8_t Dali2Device::directArcAddress() const
    {
        return _addressMode == DaliAddressMode::Broadcast ? BROADCAST_DIRECT : _address;
    }

    uint8_t Dali2Device::commandAddressType() const
    {
        if (_addressMode == DaliAddressMode::Broadcast)
            return BROADCAST_CMD;

        if (_addressMode == DaliAddressMode::Group)
            return GROUP_ADDRESS;

        return SHORT_ADDRESS;
    }

    uint8_t Dali2Device::directArcAddressType() const
    {
        if (_addressMode == DaliAddressMode::Broadcast)
            return BROADCAST_DIRECT;

        if (_addressMode == DaliAddressMode::Group)
            return GROUP_ADDRESS;

        return SHORT_ADDRESS;
    }

    unsigned char Dali2Device::SendCommand(uint8_t command)
    {
        if (!_bus.IsIdle())
            return FALSE;

        return _bus.Send_Cmd(commandAddress(), command, commandAddressType(), true);
    }

    unsigned char Dali2Device::SendSpecialCommand(uint8_t command, uint8_t data)
    {
        if (!_bus.IsIdle())
            return FALSE;

        return _bus.Send_Cmd(command, data, DALI_COMMAND, false);
    }

    unsigned char Dali2Device::StartQuery(uint8_t queryCommand)
    {
        return SendCommand(queryCommand);
    }

    unsigned char Dali2Device::SetDataTransferRegister(uint8_t value)
    {
        return SendSpecialCommand(DTR, value);
    }

    unsigned char Dali2Device::SetDataTransferRegister1(uint8_t value)
    {
        return SendSpecialCommand(REG_DTR1, value);
    }

    unsigned char Dali2Device::SetArcPowerLevel(uint8_t level)
    {
        if (!_bus.IsIdle())
            return FALSE;

        return _bus.Send_Cmd(directArcAddress(), level, directArcAddressType(), false);
    }

    unsigned char Dali2Device::Off()
    {
        return SendCommand(OFF);
    }

    unsigned char Dali2Device::Up()
    {
        return SendCommand(UP);
    }

    unsigned char Dali2Device::Down()
    {
        return SendCommand(DOWN);
    }

    unsigned char Dali2Device::StepUp()
    {
        return SendCommand(STEP_UP);
    }

    unsigned char Dali2Device::StepDown()
    {
        return SendCommand(STEP_DOWN);
    }

    unsigned char Dali2Device::RecallMaxLevel()
    {
        return SendCommand(RECALL_MAX_LEVEL);
    }

    unsigned char Dali2Device::RecallMinLevel()
    {
        return SendCommand(RECALL_MIN_LEVEL);
    }

    unsigned char Dali2Device::Reset()
    {
        return SendCommand(RESET);
    }

    unsigned char Dali2Device::StoreActualLevelInDtr()
    {
        return SendCommand(STORE_ACTUAL_LEVEL_IN_THE_DTR);
    }

    unsigned char Dali2Device::StoreDtrAsMaxLevel()
    {
        return SendCommand(STORE_THE_DTR_AS_MAX_LEVEL);
    }

    unsigned char Dali2Device::StoreDtrAsMinLevel()
    {
        return SendCommand(STORE_THE_DTR_AS_MIN_LEVEL);
    }

    unsigned char Dali2Device::StoreDtrAsPowerOnLevel()
    {
        return SendCommand(STORE_THE_DTR_AS_POWER_ON_LEVEL);
    }

    unsigned char Dali2Device::StoreDtrAsSystemFailureLevel()
    {
        return SendCommand(STORE_THE_DTR_AS_SYSTEM_FAILURE_LEVEL);
    }

    unsigned char Dali2Device::StoreDtrAsFadeTime()
    {
        return SendCommand(STORE_THE_DTR_AS_FADE_TIME);
    }

    unsigned char Dali2Device::StoreDtrAsFadeRate()
    {
        return SendCommand(STORE_THE_DTR_AS_FADE_RATE);
    }

    unsigned char Dali2Device::StartQueryStatus()
    {
        return StartQuery(QUERY_STATUS);
    }

    unsigned char Dali2Device::StartQueryBallast()
    {
        return StartQuery(QUERY_BALLAST);
    }

    unsigned char Dali2Device::StartQueryLampFailure()
    {
        return StartQuery(QUERY_LAMP_FAILURE);
    }

    unsigned char Dali2Device::StartQueryLampPowerOn()
    {
        return StartQuery(QUERY_LAMP_POWER_ON);
    }

    unsigned char Dali2Device::StartQueryLimitError()
    {
        return StartQuery(QUERY_LIMIT_ERROR);
    }

    unsigned char Dali2Device::StartQueryResetState()
    {
        return StartQuery(QUERY_RESET_STATE);
    }

    unsigned char Dali2Device::StartQueryMissingShortAddress()
    {
        return StartQuery(QUERY_MISSING_SHORT_ADDRESS);
    }

    unsigned char Dali2Device::StartQueryVersionNumber()
    {
        return StartQuery(QUERY_VERSION_NUMBER);
    }

    unsigned char Dali2Device::StartQueryContentDtr()
    {
        return StartQuery(QUERY_CONTENT_DTR);
    }

    unsigned char Dali2Device::StartQueryDeviceType()
    {
        return StartQuery(QUERY_DEVICE_TYPE);
    }

    unsigned char Dali2Device::StartQueryPhysicalMinimumLevel()
    {
        return StartQuery(QUERY_PHYSICAL_MINIMUM_LEVEL);
    }

    unsigned char Dali2Device::StartQueryActualLevel()
    {
        return StartQuery(QUERY_ACTUAL_LEVEL);
    }

    unsigned char Dali2Device::StartQueryMaxLevel()
    {
        return StartQuery(QUERY_MAX_LEVEL);
    }

    unsigned char Dali2Device::StartQueryMinLevel()
    {
        return StartQuery(QUERY_MIN_LEVEL);
    }

    unsigned char Dali2Device::StartQueryPowerOnLevel()
    {
        return StartQuery(QUERY_POWER_ON_LEVEL);
    }

    unsigned char Dali2Device::StartQuerySystemFailureLevel()
    {
        return StartQuery(QUERY_SYSTEM_FAILURE_LEVEL);
    }

    unsigned char Dali2Device::StartQueryFadeTimeFadeRate()
    {
        return StartQuery(QUERY_FADE_TIME_FADE_RATE);
    }

    unsigned char Dali2Device::FetchResponse()
    {
        return _bus.FetchDaliData();
    }
}
