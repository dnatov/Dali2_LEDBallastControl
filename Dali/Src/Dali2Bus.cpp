//
// Created by danan on 2025-11-11.
//

#include "Dali-defs.h"
#include "Dali2Bus.h"
#include "ESUM-230S500BG-Defs.h"

namespace Carendes
{
    namespace Dali
    {
        void Dali2Bus::init()
        {
            // Default dali state flags
            _dali_state = NO_ACTION;

            // Default Timer Settings
            timer_Init();
        }

        void Dali2Bus::timer_Init()
        {
            _txClear();

            _isNotStartedTimer = true;

            _tick_count = 0;
            _bit_count  = 0;
        }

        unsigned char Dali2Bus::check_Special_Cmd(unsigned char addrByte)
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

        void Dali2Bus::receiving_Data()
        {
            unsigned char pulsePosition;
            //backward frame - 9 bits to receive - last 2 don't change phase
            //first bit is start bit (1), ignore, also last 2 bits are stop bits
            //FF - BF settlling time 7Te - 22Te (2Te = 8 interrupt intervals)
            //when change on pin is detected, tick_count is restarted.

            if(_tick_count == (_bit_count * 8 + 2))
            {
                _dali_array_receive_buffer[_bit_count] =
                _rxRead() ? 1 : 0;
            }

            //increment ticks
            _tick_count++;

            if((_tick_count + 1) % 8 == 0)
            {
                _bit_count++;
            }
            //transfer completed
            if(_bit_count > 8)
            {
                //set dali state
                _dali_state = BACKWARD_FRAME_RECEIVED;
            }
        }

        void Dali2Bus::sending_Data()
        {
            unsigned char pulsePosition;

            if(_tick_count < 8)
            {
                if(_tick_count < 4)
                    _txSet();
                else
                    _txClear();
            }
            else
                if(_bit_count < 17)
                {
                    if(_tick_count % 4 == 0)
                    {
                        pulsePosition = _tick_count / 4;
                        if(pulsePosition % 2 == 0)
                        {
                            if(_dali_array_cmd[_bit_count] == DALI_START_BIT_PULSE)
                                _txClear();
                            else
                                _txSet();
                        }
                        else
                        {
                            if(_dali_array_cmd[_bit_count] == DALI_START_BIT_PULSE)
                                _txSet();
                            else
                                _txClear();
                        }
                    }
                }
            _tick_count++;

            if(_tick_count % 8 == 0)
                _bit_count++;

            if(_bit_count > 16)
            {
                _dali_state = FORWARD_FRAME_SENT;
                _txClear();
            }
        }

        void Dali2Bus::prepareDataToSend(unsigned char *commandArray, volatile unsigned char *tx_array,
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

        void Dali2Bus::prepareAddressByte(unsigned char *commandArray, unsigned char addressType,
            unsigned char byteAddressPosition, bool setDirectArcCommand = false)
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
                    if(setDirectArcCommand)
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

        unsigned char Dali2Bus::FetchDaliData()
        {
            unsigned char i;
            unsigned char receivedData = 0x00;

            for (i = 0; i < 8; i++)
            {
                // Shift the bit to the correct position and OR it with result
                receivedData |= (_dali_array_receive_buffer[i+1] << (7-i));

            }
            // Reset state
            _dali_state = NO_ACTION;
            _expect_backchannel = FALSE;

            //return received byte
            return receivedData;
        }

        unsigned char Dali2Bus::DALI_Master_Status()
        {
            unsigned char i;

            if(_dali_state == NO_ACTION)
            {
                // Idle state - reset variables
                _tick_count = 0;
                _bit_count  = 0;
            }
            else if(_dali_state == FORWARD_FRAME_SENT)
            {
                _tick_count = 0;
                _bit_count  = 0;

                // Set TX pin to idle state
                _txClear();

                // Set settling time and prepare for backchannel if expected
                if(_expect_backchannel == TRUE)
                {
                    // A small delay to allow slave device to prepare
                    if (_delayMs) { _delayMs(1); }
                    mStartTimer();
                    _dali_state = WAIT_FOR_BACKCHANNEL_TO_RECEIVE;
                }
                else
                {
                    // No backchannel expected, go to settling state
                    _dali_state = SETTLING_FF_TO_FF;
                    if (_delayMs) { _delayMs(10); }
                    // Could add a timer to transition to NO_ACTION after settling time
                    // For now, just transition directly
                    _dali_state = NO_ACTION;
                }
            }
            else if(_dali_state == BACKWARD_FRAME_RECEIVED)
            {
                // Process the received data if needed

                // Then transition to idle state
                _expect_backchannel = FALSE;
            }
            else if(_dali_state == ERR)
            {
                // Error handling - reset pins
                _txClear();
                _dali_state = NO_ACTION;
            }

            return _dali_state;
        }

        void Dali2Bus::mDoWorkInMainLoop()
        {
            if (!_isInitialized)
            {
                _isInitialized = true;
                // HAL_Delay(110);
                Send_Cmd(ENABLE_LED_MODULES_H, 0x06, ENABLE_LED_MODULES_H, true); // Enable LED Modules DT6
                Send_Cmd(BROADCAST_CMD, 0xEE, BROADCAST_CMD, true); // Enable all modules
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
            _dali_state = DALI_Master_Status();
        }

        void Dali2Bus::mDoWorkTimerInterrupt()
        {
            unsigned char i;

            // If dali state is not idle
            if(_dali_state != NO_ACTION)
            {
                // Check if data is sending
                if(_dali_state == SENDING_DATA)
                {
                    // Send data to slave device
                    sending_Data();
                }
                // Receiving state - receiving backward frame
                else if(_dali_state == RECEIVING_DATA)
                {
                    // Check and receive data from RX line
                    receiving_Data();
                }
                else if(_dali_state == WAIT_FOR_BACKCHANNEL_TO_RECEIVE)
                {
                    if(_tick_count > 200) // Timeout -> max wait time ~9.2ms
                    {
                        // No response -> answer 'NO'
                        _dali_state = ERR;
                        // There wasn't response from slave device... answer is 'NO'
                        for(i = 0; i < 8; i++)
                            _dali_array_receive_buffer[i] = 0;
                    }
                    else
                    {
                        // Check if there is a difference in RX line
                        if(_rxRead())
                        {
                            // Edge detected - set state to RECEIVING
                            _dali_state = RECEIVING_DATA;
                            _tick_count = 0;
                            _bit_count  = 0; // Reset for start bit
                        }
                    }

                    _tick_count++;
                }
            }
        }

        void Dali2Bus::mStartTimer()
        {
            // if (_isNotStartedTimer)
            // {
            //     HAL_TIM_Base_Start_IT(&htim2);
            // }
            //
            //reset tick and bit counters
            _tick_count = 0;
            _bit_count = 0;
            _isNotStartedTimer = false;
        }

        void Dali2Bus::mStopTimer()
        {
            // if (!_isNotStartedTimer)
            // {
            //     HAL_TIM_Base_Stop_IT(&htim2);
            // }

            //reset tick and bit counters
            _tick_count = 0;
            _bit_count = 0;
            _isNotStartedTimer = true;
        }

        unsigned char Dali2Bus::Send_Cmd(unsigned char ballastAddr, unsigned char cmd,
                                                unsigned char typeOfCmd, bool setDirectArcCommand = false)
        {
            unsigned char data_array[2];
            unsigned char i;

            //set output pin to 0
            _txClear();

            _tick_count = 0;
            _bit_count  = 0;

            //set DALI state to send data
            _dali_state = SENDING_DATA;

            //fetch ballast address and command
            data_array[0] = (char)ballastAddr;
            data_array[1] = (char)cmd;

            //reset dali_array_cmd values
            for (i = 0; i < 17; i++)         //16
                _dali_array_cmd[i] = 0;

            //prepare address byte to be sent
            prepareAddressByte(data_array, typeOfCmd, 0, setDirectArcCommand);

            //encode data - Manchester encoding
            prepareDataToSend(data_array, _dali_array_cmd, 2);

            //check type of command
            //set backchannel
            if((cmd >= 0x00) && (cmd <= 0x1F)) //Indirect arc power control commands
            {
                _expect_backchannel   = FALSE;
            }
            if((cmd >= 20) && (cmd <= 0x80)) //Configurations commands
            {
                _expect_backchannel = FALSE;
            }
            if((cmd >= 0x90))                //Query commands
            {
                _expect_backchannel = TRUE;     //set status to expect Backchannel. Posible answer:
                //1111 1111                             - YES
                //no response; no ba1ckchannel received  - NO
                //8bit info                             - 8 bit
            }
            if (data_array[0] == DTR || data_array[0] == REG_DTR1)
            {
                _expect_backchannel = FALSE;
            }
            //check for special command
            if(check_Special_Cmd(data_array[0]))
            {
                _expect_backchannel = TRUE;
                //
                if(data_array[0] == TERMINATE_H_BITS || data_array[0] == DTR || data_array[0] == REG_DTR1)
                    _expect_backchannel = FALSE;
                else if(data_array[0] == VERIFY_SHORT_ADDRESS || data_array[0] == QUERY_SHORT_ADDRESS_H)
                    _expect_backchannel = TRUE;
                else
                    _expect_backchannel = FALSE;


            }

            //start timer
            mStartTimer();

            return TRUE;
        }
    }
}
