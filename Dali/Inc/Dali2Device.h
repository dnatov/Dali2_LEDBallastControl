//
// Created by danan on 2025-11-11.
//
#pragma once

#include <cstdint>
#include "Dali-defs.h"
#include "Dali2Bus.h"

namespace Carendes::Dali
{
    enum class DaliAddressMode : uint8_t
    {
        Short,
        Group,
        Broadcast
    };

    /**
     * @brief Base class for a DALI control gear/device on a DALI master bus.
     *
     * This class contains common IEC 62386 control gear commands and keeps
     * address formatting out of the transport/state-machine class.
     */
    class Dali2Device
    {
    private:
        Dali2Bus& _bus;
        uint8_t _address;
        DaliAddressMode _addressMode;

        enum class ShortAddressChangeStep : uint8_t
        {
            None,
            SetDtr,
            StoreShortAddress,
            WaitUntilStored
        };

        ShortAddressChangeStep _shortAddressChangeStep = ShortAddressChangeStep::None;
        uint8_t _pendingShortAddress = 0;

        [[nodiscard]] uint8_t commandAddress() const;
        [[nodiscard]] uint8_t directArcAddress() const;
        [[nodiscard]] uint8_t commandAddressType() const;
        [[nodiscard]] uint8_t directArcAddressType() const;

    protected:
        unsigned char SendCommand(uint8_t command);
        unsigned char SendSpecialCommand(uint8_t command, uint8_t data);
        unsigned char StartQuery(uint8_t queryCommand);

    public:
        Dali2Device(Dali2Bus& bus,
                    uint8_t address,
                    DaliAddressMode addressMode = DaliAddressMode::Short);

        Dali2Bus& Bus();
        [[nodiscard]] const Dali2Bus& Bus() const;
        [[nodiscard]] uint8_t Address() const;
        [[nodiscard]] DaliAddressMode AddressMode() const;

        unsigned char SetArcPowerLevel(uint8_t level);
        unsigned char Off();
        unsigned char Up();
        unsigned char Down();
        unsigned char StepUp();
        unsigned char StepDown();
        unsigned char RecallMaxLevel();
        unsigned char RecallMinLevel();
        unsigned char Reset();
        unsigned char SetDataTransferRegister(uint8_t value);
        unsigned char SetDataTransferRegister1(uint8_t value);
        unsigned char StoreActualLevelInDtr();
        bool TryChangeShortAddress(uint8_t newShortAddress);
        unsigned char StoreDtrAsMaxLevel();
        unsigned char StoreDtrAsMinLevel();
        unsigned char StoreDtrAsPowerOnLevel();
        unsigned char StoreDtrAsSystemFailureLevel();
        unsigned char StoreDtrAsFadeTime();
        unsigned char StoreDtrAsFadeRate();

        unsigned char StartQueryStatus();
        unsigned char StartQueryBallast();
        unsigned char StartQueryLampFailure();
        unsigned char StartQueryLampPowerOn();
        unsigned char StartQueryLimitError();
        unsigned char StartQueryResetState();
        unsigned char StartQueryMissingShortAddress();
        unsigned char StartQueryVersionNumber();
        unsigned char StartQueryContentDtr();
        unsigned char StartQueryDeviceType();
        unsigned char StartQueryPhysicalMinimumLevel();
        unsigned char StartQueryActualLevel();
        unsigned char StartQueryMaxLevel();
        unsigned char StartQueryMinLevel();
        unsigned char StartQueryPowerOnLevel();
        unsigned char StartQuerySystemFailureLevel();
        unsigned char StartQueryFadeTimeFadeRate();

        unsigned char FetchResponse();
    };
}
