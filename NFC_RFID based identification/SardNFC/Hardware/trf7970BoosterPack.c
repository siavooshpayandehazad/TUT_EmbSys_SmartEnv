/*
 * {trf7970BoosterPack.c}
 *
 * {MSP430G2553 specific functions}
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

#include "mcu.h"
#include "trf7970.h"

//===============================================================

extern u08_t i_reg;
extern u08_t irq_flag;

//===============================================================
// NAME: void McuDelayMillisecond (u08_t n_ms)
//
// BRIEF: Is used to create delays.
//
// INPUTS:
//    Parameters:
//        u08_t        n_ms        delay time in ms
//
// OUTPUTS:
//
// PROCESS:    [1] do loop of 1 ms duration as often as required
//
// CHANGE:
// DATE          WHO    DETAIL
// 23Nov2010    RP    Original Code
//===============================================================

void
McuDelayMillisecond(u32_t n_ms)
{
    while (n_ms--)
    {
        __delay_cycles(2*DELAY_1ms);        // clock speed in Hz divided by 1000
    }
}

//===============================================================
// NAME: void McuCounterSet (void)
//
// BRIEF: Is used to set the timerA.
//
// INPUTS:
//
// OUTPUTS:
//
// PROCESS:    [1] set timerA
//
// CHANGE:
// DATE          WHO    DETAIL
// 23Nov2010    RP    Original Code
//===============================================================

void
McuCounterSet(void)
{
    TACTL |= TASSEL_1 + TACLR;      // ACLK = 1.5 kHz, timer stopped

    TACCTL0 |= CCIE;                // compare interrupt enable
}

//===============================================================
// NAME: void McuOscSel (u08_t mode)
//
// BRIEF: Is used to select the oscilator.
//
// INPUTS:
//    Parameters:
//        u08_t        mode        crystal oscillator or DCO for
//                        main clock
//
// OUTPUTS:
//
// PROCESS:    [1] select the oscillator
//
// CHANGE:
// DATE          WHO    DETAIL
// 23Nov2010    RP    Original Code
//===============================================================

void
McuOscSel(u08_t mode)
{
    u08_t ii1 = 0;

    if (mode == 0x00)
    {
        // select crystal oscilator

        BCSCTL1 |= XTS;               // ACLK = LFXT1 HF XTAL
        BCSCTL3 |= LFXT1S1;                    // 3 – 16MHz crystal or resonator
        // turn external oscillator on
        do
        {
            IFG1 &= ~OFIFG;                           // Clear OSCFault flag
            for (ii1 = 0xFF; ii1 > 0; ii1--);         // Time delay for flag to set
        } while ((IFG1 & OFIFG) == OFIFG);            // OSCFault flag still set?

        BCSCTL2 |= SELM_2 + SELS;                       // MCLK = SMCLK = HF LFXT1 (safe)
    }
    else                                               //select DCO for main clock
    {
        // select DCO to 8MHz

        if (CALBC1_8MHZ != 0xFF)
        {
            // Follow recommended flow. First, clear all DCOx and MODx bits.
            // Then apply new RSELx values. Finally, apply new DCOx and MODx bit
            // values.
            DCOCTL = 0x00;
            BCSCTL1 = CALBC1_8MHZ;
            DCOCTL = CALDCO_8MHZ;
        }

        // Disable XT1 pins
        P2SEL &= ~(BIT6 + BIT7);

        // Disable XT1 high frequency mode ACLK = 12kHz/8 = 1.5kHz for WDT
        BCSCTL1 &= ~XTS;
        BCSCTL1 |= DIVA_3;

        // Set XT1 to VLO
        BCSCTL3 |= LFXT1S_2;
    }
}

//===============================================================
// NAME: void Msp430f23x0TimerAHandler (void)
//
// BRIEF: Is used to handle the interrupts generated by timerA.
//
// INPUTS:
//
// OUTPUTS:
//    Globals:
//        u08_t        i_reg        indicates TIMEOUTs
//
//
// PROCESS:    [1] handle the interrupts generated by timerA
//
// CHANGE:
// DATE          WHO    DETAIL
// 23Nov2010    RP    Original Code
//===============================================================

#pragma vector=TIMER0_A0_VECTOR
__interrupt void
timerHandler(void)
{
    u08_t irq_status[4];

    STOP_COUNTER;

    irq_flag = 0x02;

    Trf7970ReadIrqStatus(irq_status);

    *irq_status = *irq_status & 0xF7;                // set the parity flag to 0

    if(*irq_status == 0x00 || *irq_status == 0x80)
    {
        i_reg = 0x00;                                // timer interrupt
    }
    else
    {
        i_reg = 0x01;
    }

    //__bic_SR_register_on_exit(LPM0_bits);
}
