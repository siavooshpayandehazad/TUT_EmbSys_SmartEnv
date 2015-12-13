/*
 * {trf7970BoosterPack.h}
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

#ifndef _Trf7970BOOSTERPACK_H_
#define _Trf7970BOOSTERPACK_H_

//================================================================

#include <msp430.h>                     // prozessor spezific header
#include "types.h"
#include "mcu.h"

//===============================================================

#define TRIGGER        0                        // if TRIGGER 1 trigger-point at LED 5

//=====MCU constants=============================================

//Trf7970 IO port.
//port4 is connected to the
#define TRF_WRITE           P2OUT
#define TRF_READ            P2IN
#define TRF_DIR_IN          P2DIR = 0x00
#define TRF_DIR_OUT         P2DIR = 0xFF
#define TRF_FUNC            P2SEL = 0x00


// EN pin on the Trf7970
// P2.2 is switched in output direction
#define ENABLE_PORT_SET          P2DIR |= BIT2
#define ENABLE_TRF          P2OUT |= BIT2
#define DISABLE_TRF         P2OUT &= ~BIT2

//---- PIN operations -------------------------------------------

#define MOD_SET             P1DIR |= BIT5;
#define MOD_ON              P1OUT |= BIT5
#define MOD_OFF             P1OUT &= ~BIT5;

// IRQ on P2.0
// rising edge interrupt
#define IRQ_PIN_SET         P2DIR &= ~IRQ_PIN;
#define IRQ_PIN             (BIT0 | BIT7)
#define IRQ_PORT            P2IN
#define IRQ_ON              P2IE |= IRQ_PIN
#define IRQ_OFF             P2IE &= ~IRQ_PIN
#define IRQ_EDGE_SET        P2IES &= ~IRQ_PIN
#define IRQ_CLR             P2IFG = 0x00
#define IRQ_REQ_REG         P2IFG

#define LED_PORT_SET        ;//P1DIR |= (BIT3+BIT4+BIT5);
#define LED_ALL_OFF         ;//P1OUT &= ~(BIT3+BIT4+BIT5);
#define LED_ALL_ON          ;//P1OUT |= (BIT3+BIT4+BIT5);
#define LED_POWER_ON        ;
#define LED_POWER_OFF       ;
#define LED_14443A_ON       ;//P1OUT |= BIT4;
#define LED_14443A_OFF      ;//P1OUT &= ~BIT4;
#define LED_14443B_ON       ;//P1OUT |= BIT3;
#define LED_14443B_OFF      ;//P1OUT &= ~BIT3;
#define LED_15693_ON        ;//P1OUT |= BIT5;
#define LED_15693_OFF       ;//P1OUT &= ~BIT5;
#define LED_OPEN1_ON        ;
#define LED_OPEN1_OFF       ;
#define LED_OPEN2_ON        ;
#define LED_OPEN2_OFF       ;

// Hard code to only SPI mode
#define SPIMODE             1

// CSn
#define SLAVE_SELECT_PORT_SET   P2DIR |= BIT1;
#define SLAVE_SELECT_HIGH       P2OUT |= BIT1;
#define SLAVE_SELECT_LOW        P2OUT &= ~BIT1;

#define OOK_DIR_IN          ;
//#define OOK_DIR_OUT         P2DIR |= BIT2
//#define OOK_OFF             P2OUT &= ~BIT2
//#define OOK_ON              P2OUT |= BIT2

// SPI Config
#define SPI_PORT_FUNC1      P1SEL
#define SPI_PORT_FUNC2      P1SEL2
#define SPI_DIR             P1DIR
#define SPI_OUT             P1OUT
#define SPI_CLK             BIT5
#define SPI_MISO            BIT6
#define SPI_MOSI            BIT7

//-----Counter-timer constants-----------------------------------
#define COUNT_VALUE         TACCR0                    //counter register
#define START_COUNTER       TACTL |=  MC1        //start counter in up mode
#define STOP_COUNTER        TACTL &= ~(MC0 + MC1)    //stops the counter
#define RESET_COUNTER       TACTL |= TACLR	    //Resets and stops counter.

//---------------------------------------------------------------
// Set timer capture register value based on clock source
// ACLK = VLO = 1.5kHz (for WDT)
#define COUNT_1ms           1.5
#define COUNT_60ms          90

//---------------------------------------------------------------

// 8MHz clock
#define DELAY_1ms           6780

//===============================================================

#endif
