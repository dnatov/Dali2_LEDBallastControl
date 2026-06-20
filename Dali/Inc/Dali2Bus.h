//
// Created by danan on 2025-11-11.
//
#pragma once

#include <cstdint>
#include <functional>
#include "Dali-defs.h"
#include "LLE_IOPIN.h"

namespace Carendes::Dali
{
    /**
     * @brief Interface representing a DALI Click module.
     *
     * This defines the core methods for managing and interacting with a DALI Click device.
     * It assumes the existence of two IOPIN objects, one for transmitting (pDaliTxPin) and one for
     * receiving (pDaliRxPin), which need to be implemented by any derived class.
     *
     * Derived classes are expected to implement specific functionality for active work
     * handling and timer management.
     */
    class Dali2Bus
    {
    private:
        using DelayMsFn = void(*)(uint32_t);
        bool _isInitialized = false;
        volatile bool _isNotStartedTimer = true;
        volatile unsigned int _tick_count = 0;
        volatile unsigned char _bit_count = 0;

        volatile unsigned char _dali_state = NO_ACTION;

        volatile unsigned char _dali_array_cmd[17] = {};
        volatile unsigned char _dali_array_receive_buffer[9] = {};

        volatile unsigned char _expect_backchannel = false;
        volatile unsigned char _expected_response = false;
        LowLevelEmbedded::IOPIN* _txPin;
        LowLevelEmbedded::IOPIN* _rxPin;
        uint8_t finishTransfer();
        void init();
        void timer_Init();
        unsigned char check_Special_Cmd(unsigned char addrByte);
        void receiving_Data();
        void sending_Data();
        void prepareDataToSend(unsigned char *commandArray, volatile unsigned char *tx_array,
                              unsigned char bytesInCmd);
        void prepareAddressByte(unsigned char *commandArray, unsigned char addressType,
                               unsigned char byteAddressPosition, bool setDirectArcCommand);
        unsigned char DALI_Master_Status();
    protected:
    public:
        Dali2Bus(LowLevelEmbedded::IOPIN* txPin,
                 LowLevelEmbedded::IOPIN* rxPin)
        : _txPin(txPin),
          _rxPin(rxPin)
        {
            init();
        }

        unsigned char FetchDaliData();
        unsigned char Status() const;
        bool IsIdle() const;
        bool HasReceivedData() const;

        // Stick this in your main loop to process the Dali State Machine
        void mDoWorkInMainLoop();

        // Stick this timer function into the global Tim 2 interrupt
        // Timer is set up to trigger every ~104us periodic.
        void mDoWorkTimerInterrupt();

        // Start STM32 Timer
        virtual void mStartTimer() = 0;

        // Stop STM32 Timer
        virtual void mStopTimer() = 0;

        unsigned char Send_Cmd(unsigned char ballastAddr, unsigned char cmd,
                                    unsigned char typeOfCmd, bool setDirectArcCommand);
    };
}
