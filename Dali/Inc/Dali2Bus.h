//
// Created by danan on 2025-11-11.
//

#pragma once

#include "ESUM-230S500BG-Defs.h"
#include "LLE_IOPIN.h"

#pragma once

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
        bool _isInitialized = false;
        volatile bool _isNotStartedTimer = true;
        volatile unsigned int tick_count = 0;
        volatile unsigned char bit_count = 0;

        volatile unsigned char dali_state = NO_ACTION;

        volatile unsigned char dali_array_cmd[17] = {};
        volatile unsigned char dali_array_receive_buffer[9] = {};

        volatile unsigned char expect_backchannel = false;
        volatile unsigned char expected_response = false;
        LowLevelEmbedded::IOPIN* pDaliTxPin;
        LowLevelEmbedded::IOPIN* pDaliRxPin;
        uint8_t finishTransfer();
        void DALI_Init();
        void Timer_DALI_Init();
        unsigned char DALI_Send_Cmd(unsigned char ballastAddr, unsigned char cmd,
                                    unsigned char typeOfCmd, unsigned char followingType);
        unsigned char DALI_Check_Special_Cmd(unsigned char addrByte);
        void DALI_Receiving_Data();
        void DALI_Sending_Data();
        void PrepareDataToSend(unsigned char *commandArray, volatile unsigned char *tx_array,
                              unsigned char bytesInCmd);
        void PrepareAddressByte(unsigned char *commandArray, unsigned char addressType,
                               unsigned char byteAddressPosition, unsigned char followingType);
        unsigned char DALI_Get_Ballast_Answer();
        unsigned char DALI_Master_Status();
    protected:
    public:
        Dali2Bus(LowLevelEmbedded::IOPIN* txPin, LowLevelEmbedded::IOPIN* rxPin)
        {
            pDaliTxPin = txPin;
            pDaliRxPin = rxPin;

            DALI_Init();
        }

        void StartDataQuery(uint8_t address, uint8_t requestedData);
        void StartDiagnosticDataQuery(uint8_t address, uint8_t requestedData);

        uint32_t FetchDaliData32(uint8_t data);
        uint16_t FetchDaliData16(uint8_t data);
        uint8_t FetchDaliData8(uint8_t data);

        // Stick this in your main loop to process the Dali State Machine
        void mDoWorkInMainLoop();

        // Stick this timer function into the global Tim 2 interrupt
        // Timer is set up to trigger every ~104us periodic.
        void mDoWorkTimerInterrupt();

        // Start STM32 Timer
        virtual void mStartTimer() = 0;

        // Stop STM32 Timer
        virtual void mStopTimer() = 0;
    };
}
