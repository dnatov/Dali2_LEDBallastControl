//
// Created by danan on 2025-11-11.
//

#include "Dali-defs.h"
#include "Dali2Bus.h"
#include "Delay.h"

namespace Carendes
{
    namespace Dali
    {
        void Dali2Bus::DALI_Init()
        {
            // Default dali state flags
            dali_state = NO_ACTION;

            // Default Timer Settings
            Timer_DALI_Init();
        }

        void Dali2Bus::Timer_DALI_Init()
        {
            //pDaliRxPin->Set();
            pDaliTxPin->Clear();

            _isNotStartedTimer = true;

            tick_count = 0;
            bit_count  = 0;
        }

        unsigned char Dali2Bus::DALI_Send_Cmd(unsigned char ballastAddr, unsigned char cmd,
                                                unsigned char typeOfCmd, unsigned char followingType)
        {
            unsigned char data_array[2];
            unsigned char i;

            //set output pin to 0
            pDaliTxPin->Clear();

            tick_count = 0;
            bit_count  = 0;

            //set DALI state to send data
            dali_state = SENDING_DATA;

            //fetch ballast address and command
            data_array[0] = (char)ballastAddr;
            data_array[1] = (char)cmd;

            //reset dali_array_cmd values
            for (i = 0; i < 17; i++)         //16
                dali_array_cmd[i] = 0;

            //prepare address byte to be sent
            PrepareAddressByte(data_array, typeOfCmd, 0, followingType);

            //encode data - Manchester encoding
            PrepareDataToSend(data_array, dali_array_cmd, 2);

            //check type of command
            //set backchannel
            if((cmd >= 0x00) && (cmd <= 0x1F)) //Indirect arc power control commands
            {
                expect_backchannel   = FALSE;
            }
            if((cmd >= 20) && (cmd <= 0x80)) //Configurations commands
            {
                expect_backchannel = FALSE;
            }
            if((cmd >= 0x90))                //Query commands
            {
                expect_backchannel = TRUE;     //set status to expect Backchannel. Posible answer:
                //1111 1111                             - YES
                //no response; no ba1ckchannel received  - NO
                //8bit info                             - 8 bit
            }
            if (data_array[0] == DTR || data_array[0] == REG_DTR1)
            {
                expect_backchannel = FALSE;
            }
            //check for special command
            if(DALI_Check_Special_Cmd(data_array[0]))
            {
                expect_backchannel = TRUE;
                //
                if(data_array[0] == TERMINATE_H_BITS || data_array[0] == DTR || data_array[0] == REG_DTR1)
                    expect_backchannel = FALSE;
                else if(data_array[0] == VERIFY_SHORT_ADDRESS || data_array[0] == QUERY_SHORT_ADDRESS_H)
                    expect_backchannel = TRUE;
                else
                    expect_backchannel = FALSE;


            }

            //start timer
            mStartTimer();

            return TRUE;
        }

        unsigned char Dali2Bus::DALI_Check_Special_Cmd(unsigned char addrByte)
        {
            volatile unsigned char addrToCheck;

            addrToCheck = addrByte;    //get address byte
            if ((addrToCheck == 0x90) || (addrToCheck == 0xA0)) //check for 1010 or 1011
            {
                if(addrToCheck & 0x01) //LSB must be 1
                    return TRUE;
                else
                    return FALSE;
            }
            else
            {
                return FALSE;
            }
        }

        void Dali2Bus::DALI_Receiving_Data()
        {
            unsigned char pulsePosition;
            //backward frame - 9 bits to receive - last 2 don't change phase
            //first bit is start bit (1), ignore, also last 2 bits are stop bits
            //FF - BF settlling time 7Te - 22Te (2Te = 8 interrupt intervals)
            //when change on pin is detected, tick_count is restarted.

            if(tick_count == (bit_count * 8 + 2))
            {
                if(pDaliRxPin->GetValue() == 0)
                    dali_array_receive_buffer[bit_count] = 0;
                else
                    dali_array_receive_buffer[bit_count] = 1;
            }

            //increment ticks
            tick_count++;

            if((tick_count + 1) % 8 == 0)
            {
                bit_count++;
            }
            //transfer completed
            if(bit_count > 8)
            {
                //set dali state
                dali_state = BACKWARD_FRAME_RECEIVED;
            }
        }

        void Dali2Bus::DALI_Sending_Data()
        {
            unsigned char pulsePosition;

            if(tick_count < 8)
            {
                if(tick_count < 4)
                    pDaliTxPin->Set();
                else
                    pDaliTxPin->Clear();
            }
            else
                if(bit_count < 17)
                {
                    if(tick_count % 4 == 0)
                    {
                        pulsePosition = tick_count / 4;
                        if(pulsePosition % 2 == 0)
                        {
                            if(dali_array_cmd[bit_count] == DALI_START_BIT_PULSE)
                                pDaliTxPin->Clear();
                            else
                                pDaliTxPin->Set();
                        }
                        else
                        {
                            if(dali_array_cmd[bit_count] == DALI_START_BIT_PULSE)
                                pDaliTxPin->Set();
                            else
                                pDaliTxPin->Clear();
                        }
                    }
                }
            tick_count++;

            if(tick_count % 8 == 0)
                bit_count++;

            if(bit_count > 16)
            {
                dali_state = FORWARD_FRAME_SENT;
                pDaliTxPin->Clear();
            }
        }

        void Dali2Bus::PrepareDataToSend(unsigned char *commandArray, volatile unsigned char *tx_array,
            unsigned char bytesInCmd)
        {
            //set default value for the mask
            unsigned char mask = 0x80;
            //variable which hold one byte value - one element from commandArray
            unsigned char dummy;
            //number of bytes in command
            unsigned char bytes_counter;
            unsigned char i;
            //number of active bit
            unsigned char bitCounter;
            //set default value
            bitCounter = 0;

            for (i = 0; i < 9; i++)
            {
                tx_array[0] = 0;
            }

            //loop through all bytes in commandArray
            for(bytes_counter = 0; bytes_counter < bytesInCmd; bytes_counter++)
            {
                //assign byte for use
                dummy = commandArray[bytes_counter];
                //set mask to default value
                mask = 0x80;
                //increment number of active bit
                bitCounter++;

                //check if active bit is the first one
                if(bitCounter == 1)
                {
                    //start bit is always 1 - in manchester that is END_BIT_PULSE
                    tx_array[0] = DALI_END_BIT_PULSE;
                }
                //2 byte command
                //go through all bytes and use Manchester
                for(i = 1; i < 9; i++) //1 & 9
                {
                    //check if bit is one
                    if(dummy & mask)
                    {
                        //assign pulse value - manchester
                        tx_array[i + (8 * bytes_counter)] = DALI_END_BIT_PULSE;
                    }
                    else
                    {
                        //assign pulse value - manchester
                        tx_array[i + (8 * bytes_counter)] = DALI_START_BIT_PULSE;
                    }
                    //check mask value
                    if(mask == 0x01)
                        mask <<= 7;     //shift mask bit to MSB
                    else
                        mask >>= 1;     //shift mask bit to 1 right
                }
            }
            //tx_array[17] = DALI_END_BIT_PULSE;

            //add 2 stop bits at the end
            /*for (i = 1; i < 3; i++)
              {
                //assign pulse value - manchester
                tx_array[16 + i] = DALI_END_BIT_PULSE;
              }*/
        }

        void Dali2Bus::PrepareAddressByte(unsigned char *commandArray, unsigned char addressType,
            unsigned char byteAddressPosition, unsigned char followingType)
        {
            unsigned char addr_tmp;
            //broadcast command to all ballasts
            if(addressType == BROADCAST_CMD)
            {
                //set address byte to Broadcast command - value 0xFF
                commandArray[byteAddressPosition] = BROADCAST_CMD;
            }
            else
            {
                //fetch address value from array to operate
                addr_tmp = commandArray[byteAddressPosition];

                if (addressType == BROADCAST_DIRECT)
                    //broadcast direct arc level to all ballasts - value 0xFE
                        commandArray[byteAddressPosition] = BROADCAST_DIRECT;
                else if (addr_tmp > ADDRESS64)
                {
                    // DTR, Special Commands and Others don't need address shifting or command setting
                }
                else
                {
                    //shift address value for 1 to left
                    addr_tmp <<= 1;

                    //check if the command byte is following address byte
                    if(followingType == FOLLOWING_COMMAND)
                        //set LSB
                            addr_tmp |= 0x01;
                    //if it is a group address
                    if (addressType == GROUP_ADDRESS)
                        //add group value to address byte
                            addr_tmp |= GROUP_ADDRESS;
                    //assign return value
                    commandArray[byteAddressPosition] = addr_tmp;
                }
            }
        }

        unsigned char Dali2Bus::DALI_Get_Ballast_Answer()
        {
            unsigned char i;
            unsigned char receivedData = 0x00;

            for (i = 0; i < 8; i++)
            {
                // Shift the bit to the correct position and OR it with result
                receivedData |= (dali_array_receive_buffer[i+1] << (7-i));

            }
            //return received byte
            return receivedData;
        }

        unsigned char Dali2Bus::DALI_Master_Status()
        {
            unsigned char i;

            if(dali_state == NO_ACTION)
            {
                // Idle state - reset variables
                tick_count = 0;
                bit_count  = 0;
            }
            else if(dali_state == FORWARD_FRAME_SENT)
            {
                tick_count = 0;
                bit_count  = 0;

                // Set TX pin to idle state
                pDaliTxPin->Clear();

                // Set settling time and prepare for backchannel if expected
                if(expect_backchannel == TRUE)
                {
                    // A small delay to allow slave device to prepare
                    LowLevelEmbedded::Utility::Delay_ms(1);
                    mStartTimer();
                    dali_state = WAIT_FOR_BACKCHANNEL_TO_RECEIVE;
                }
                else
                {
                    // No backchannel expected, go to settling state
                    dali_state = SETTLING_FF_TO_FF;
                    LowLevelEmbedded::Utility::Delay_ms(10);
                    // Could add a timer to transition to NO_ACTION after settling time
                    // For now, just transition directly
                    dali_state = NO_ACTION;
                }
            }
            else if(dali_state == BACKWARD_FRAME_RECEIVED)
            {
                // Process the received data if needed

                // Then transition to idle state
                expect_backchannel = FALSE;
            }
            else if(dali_state == ERR)
            {
                // Error handling - reset pins
                pDaliTxPin->Clear();
                dali_state = NO_ACTION;
            }

            return dali_state;
        }

        void Dali2Bus::mDoWorkInMainLoop()
        {
            if (!_isInitialized)
            {
                _isInitialized = true;
                // HAL_Delay(110);
                DALI_Send_Cmd(ENABLE_LED_MODULES_H, 0x06, ENABLE_LED_MODULES_H, FOLLOWING_COMMAND); // Enable LED Modules DT6
                DALI_Send_Cmd(BROADCAST_CMD, 0xEE, BROADCAST_CMD, FOLLOWING_COMMAND); // Enable all modules
                // //     // Send broadcast command to all ballasts
                // //     //DALI_Send_Cmd(ADDRESS01, OFF, BROADCAST_CMD, FOLLOWING_COMMAND);
                // //     //HAL_Delay(1500);
                // //     mDoWorkInMainLoop();
                // DALI_Send_Cmd(DTR, 0x00, DALI_COMMAND, FOLLOWING_DIRECT_ARC_POWER_LVL); // Store roughly 20% power into DTR
                // HAL_Delay(45);
                // DALI_Send_Cmd(BROADCAST_CMD, STORE_THE_DTR_AS_POWER_ON_LEVEL, BROADCAST_CMD, FOLLOWING_COMMAND); // Choose DTR Memory Location
                // HAL_Delay(45);
                // DALI_Send_Cmd(BROADCAST_CMD, STORE_THE_DTR_AS_POWER_ON_LEVEL, BROADCAST_CMD, FOLLOWING_COMMAND); // Choose DTR Memory Location
                // HAL_Delay(45);
                // DALI_Send_Cmd(ADDRESS01, STORE_ACTUAL_LEVEL_IN_THE_DTR, BROADCAST_CMD, FOLLOWING_COMMAND); // Stores DTR Level to Memory
                // HAL_Delay(50);
                // DALI_Send_Cmd(BROADCAST_CMD, QUERY_POWER_ON_LEVEL, BROADCAST_CMD, FOLLOWING_COMMAND); // Query DTR
                //DALI_Send_Cmd(BROADCAST_CMD, QUERY_BALLAST, BROADCAST_CMD, FOLLOWING_COMMAND);
                // DALI_Send_Cmd(BROADCAST_CMD, QUERY_BALLAST, BROADCAST_CMD, FOLLOWING_COMMAND);
            }



            //get DALI status - master
            dali_state = DALI_Master_Status();
        }

        void Dali2Bus::mDoWorkTimerInterrupt()
        {
            unsigned char i;

            // If dali state is not idle
            if(dali_state != NO_ACTION)
            {
                // Check if data is sending
                if(dali_state == SENDING_DATA)
                {
                    // Send data to slave device
                    DALI_Sending_Data();
                }
                // Receiving state - receiving backward frame
                else if(dali_state == RECEIVING_DATA)
                {
                    // Check and receive data from RX line
                    DALI_Receiving_Data();
                }
                else if(dali_state == WAIT_FOR_BACKCHANNEL_TO_RECEIVE)
                {
                    if(tick_count > 200) // Timeout -> max wait time ~9.2ms
                    {
                        // No response -> answer 'NO'
                        dali_state = ERR;
                        // There wasn't response from slave device... answer is 'NO'
                        for(i = 0; i < 8; i++)
                            dali_array_receive_buffer[i] = 0;
                    }
                    else
                    {
                        // Check if there is a difference in RX line
                        if(pDaliRxPin->GetValue())
                        {
                            // Edge detected - set state to RECEIVING
                            dali_state = RECEIVING_DATA;
                            tick_count = 0;
                            bit_count  = 0; // Reset for start bit
                        }
                    }

                    tick_count++;
                }
            }
        }

        // void Dali2Bus::mStartTimer()
        // {
        //     if (_isNotStartedTimer)
        //     {
        //         HAL_TIM_Base_Start_IT(&htim2);
        //     }
        //
        //     //reset tick and bit counters
        //     tick_count = 0;
        //     bit_count = 0;
        //     _isNotStartedTimer = false;
        // }
        //
        // void Dali2Bus::mStopTimer()
        // {
        //     if (!_isNotStartedTimer)
        //     {
        //         HAL_TIM_Base_Stop_IT(&htim2);
        //     }
        //
        //     //reset tick and bit counters
        //     tick_count = 0;
        //     bit_count = 0;
        //     _isNotStartedTimer = true;
        // }

        // Finalizes the end of the return data and resets the State Machine
        uint8_t Dali2Bus::finishTransfer()
        {
            // Get the received temperature value
            unsigned char temperature = DALI_Get_Ballast_Answer();

            // Reset state
            dali_state = NO_ACTION;
            expect_backchannel = FALSE;
            return temperature;
        }
    }
}
