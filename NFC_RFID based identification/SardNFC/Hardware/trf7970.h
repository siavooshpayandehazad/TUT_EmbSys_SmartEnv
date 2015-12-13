/*
 * {trf7970.h}
 *
 * {Header File}
 *
 * Copyright (C) 2013 Texas Instruments Incorporated - http://www.ti.com/
 *
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *    Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 *    Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the
 *    distribution.
 *
 *    Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
*/
#ifndef _Trf7970_H_
#define _Trf7970_H_

//===============================================================

#include <msp430.h>                         // prozessor spezific header
#include <stdlib.h>                         // general purpose standard library
#include "iso14443a.h"
#include "mcu.h"
#include "spi.h"
#include "types.h"
#include "uart.h"

//===============================================================

#define DBG    0                        // if DBG 1 interrupts are display

//==== Trf7970 definitions ======================================

//---- Direct commands ------------------------------------------

#define IDLE                        0x00
#define SOFT_INIT                   0x03
#define INITIAL_RF_COLLISION        0x04
#define RESPONSE_RF_COLLISION_N     0x05
#define RESPONSE_RF_COLLISION_0     0x06
#define RESET                       0x0F
#define TRANSMIT_NO_CRC             0x10
#define TRANSMIT_CRC                0x11
#define DELAY_TRANSMIT_NO_CRC       0x12
#define DELAY_TRANSMIT_CRC          0x13
#define TRANSMIT_NEXT_SLOT          0x14
#define CLOSE_SLOT_SEQUENCE         0x15
#define STOP_DECODERS               0x16
#define RUN_DECODERS                0x17
#define CHECK_INTERNAL_RF           0x18
#define CHECK_EXTERNAL_RF           0x19
#define ADJUST_GAIN                 0x1A

//---- Reader register ------------------------------------------

#define CHIP_STATE_CONTROL              0x00
#define ISO_CONTROL                     0x01
#define ISO_14443B_OPTIONS              0x02
#define ISO_14443A_OPTIONS              0x03
#define TX_TIMER_EPC_HIGH               0x04
#define TX_TIMER_EPC_LOW                0x05
#define TX_PULSE_LENGTH_CONTROL         0x06
#define RX_NO_RESPONSE_WAIT_TIME        0x07
#define RX_WAIT_TIME                    0x08
#define MODULATOR_CONTROL               0x09
#define RX_SPECIAL_SETTINGS             0x0A
#define REGULATOR_CONTROL               0x0B
#define IRQ_STATUS                      0x0C    // IRQ Status Register
#define IRQ_MASK                        0x0D    // Collision Position and Interrupt Mask Register
#define COLLISION_POSITION              0x0E
#define RSSI_LEVELS                     0x0F
#define SPECIAL_FUNCTION                0x10
#define RAM_START_ADDRESS               0x11    //RAM is 6 bytes long (0x11 - 0x16)
#define NFCID                           0x17
#define NFC_TArGET_LEVEL                0x18
#define NFC_TARGET_PROTOCOL             0x19
#define TEST_SETTINGS_1                 0x1A
#define TEST_SETTINGS_2                 0x1B
#define FIFO_STATUS                     0x1C
#define TX_LENGTH_BYTE_1                0x1D
#define TX_LENGTH_BYTE_2                0x1E
#define FIFO                            0x1F

//===============================================================

void Trf7970CommunicationSetup(void);
void Trf7970DirectCommand(u08_t *pbuf);
void Trf7970DirectMode(void);
void Trf7970DisableSlotCounter(void);
void Trf7970EnableSlotCounter(void);
void Trf7970InitialSettings(void);
void Trf7970RawWrite(u08_t *pbuf, u08_t length);
void Trf7970ReConfig(void);
void Trf7970ReadCont(u08_t *pbuf, u08_t length);
void Trf7970ReadIrqStatus(u08_t *pbuf);
void Trf7970ReadSingle(u08_t *pbuf, u08_t length);
void Trf7970Reset(void);
void Trf7970ResetIrqStatus(void);
void Trf7970RunDecoders(void);
void Trf7970StopDecoders(void);
void Trf7970TransmitNextSlot(void);
void Trf7970TurnRfOff(void);
void Trf7970TurnRfOn(void);
void Trf7970WriteCont(u08_t *pbuf, u08_t length);
void Trf7970WriteIsoControl(u08_t iso_control);
void Trf7970WriteSingle(u08_t *pbuf, u08_t length);

//===============================================================

#endif
