/*
 * main_nwk.c
 *
 *  Created  on: 06.02.2015
 *      Author:  Tsotne Putkaradze, Nish Tahir
 *
 *  Version: 0.4                7.12.2015
 *
 *  communication:      https://docs.google.com/spreadsheets/d/1EX6megqmACzNTX3JTMHogJLxSqM1P2WMeiZZyxYM3Lw/edit#gid=0
 *  packet format:      http://ati.ttu.ee/embsys/images/thumb/a/ae/Variable_length_packet_format.png/600px-Variable_length_packet_format.png
 *  wireless chip:      http://prntscr.com/9d8lgj
 *
 *  whole project:      https://github.com/siavooshpayandehazad/TUT_EmbSys_SmartEnv
 *      this part:              https://github.com/tsotnep/MSP430G2553_MRF89XA_Receiver_Example_Project
 *      WIKI:                   http://ati.ttu.ee/embsys/index.php/Automated_indoor_lighting
 *
 *  Family Guide:       http://www.ti.com/lit/ug/slau144j/slau144j.pdf
 *  Datasheet:          http://www.ti.com/lit/ds/symlink/msp430g2553.pdf
 */
/***************************************************************************************************
 *              Include section                                                                                                                *
 ***************************************************************************************************/
 
#include "system.h"
#include "network.h"
#include "general.h"
 
// Drivers
#include "uart.h"
#include "spi.h"
#include "radio.h"
 
// Network
#include "nwk_security.h"
#include "nwk_radio.h"
 
/***************************************************************************************************
 *              Define section                                                                                                                 *
 ***************************************************************************************************/
//port 1
#define pwmLED1_P10_T00 BIT0    //1.0
#define pwmLED2_P11_T01 BIT1    //1.1
#define pwmLED3_P12_T02 BIT2    //1.2
 
//port 2
#define pwmLED4_P22_T10 BIT2    //2.2
#define pwmLED5_P23_T11 BIT3    //2.3
#define pwmLED6_P24_T12 BIT4    //2.4
 
 
#define timerZeroStartValue             0b1111111110011010 // -101 => period is 100 counts
#define timerOneStartValue              0b1111111110011010 // -101 => period is 100 counts
#define NumberOfLEDs                    6
 
/***************************************************************************************************
 *              Prototype section                                                                                                                  *
 ***************************************************************************************************/
 
void _init_timer0();
void _init_timer1();
void _init_CCRs();
void _update_CCRs();
void Print_Error(uint8 error_code);
/***************************************************************************************************
 *              Global Variable section                                                                                            *
 ***************************************************************************************************/
uint8 exit_code = 0;            // Exit code number that is set after function exits
uint8 payload_length;
uint8 temp_for_storing_received8 = 0;
uint16 temp_for_storing_received16 = 0;
uint16 LEDvalues[NumberOfLEDs] = { 0 };
 
/***************************************************************************************************
 *         Main section                                                                            *
 ***************************************************************************************************/
void main(void) {
        uint8 len, var;
        uint8 rssi_rx;
 
        // Initialize system
        Print_Error(System_Init());
 
        //Port 1 directions and initial values
        P1DIR |= (pwmLED1_P10_T00 | pwmLED2_P11_T01 | pwmLED3_P12_T02);
        P1OUT &= ~(pwmLED1_P10_T00 | pwmLED2_P11_T01 | pwmLED3_P12_T02);
 
        //Port 2 directions and initial values
        P2DIR |= (pwmLED4_P22_T10 | pwmLED5_P23_T11 | pwmLED6_P24_T12);
        P2OUT &= ~(pwmLED4_P22_T10 | pwmLED5_P23_T11 | pwmLED6_P24_T12);
 
        _init_timer0();
        _init_timer1();
        _init_CCRs();
 
        while (1) {
 
                // Clear received packet buffer
                for (var = RF_BUFFER_SIZE - 1; var > 0; var--)
                        RxPacket[var] = 0;
 
                // Set radio into RX mode
                Radio_Set_Mode(RADIO_RX);
 
                // Enter to LPM0 w/ interrupts enabled
                // Module stays in low power mode until interrupt is received from radio and wakes up
                __bis_SR_register(LPM0_bits + GIE);
 
                // Receive data, wait until 100ms until timeout and continue with other tasks
                if (exit_code = Radio_Receive_Data(RxPacket, &len, 100, &rssi_rx)) {
                        Print_Error(exit_code);
                } else {
                        for (var = 5; var < len; ++var) {
                                if (var > 5) { //TODO : read "L" ascii, and put rest of logic in IF(readAscii = 'L')
                                        temp_for_storing_received8 = RxPacket[var];
                                        temp_for_storing_received16 = temp_for_storing_received8 * 100;
                                        temp_for_storing_received16 /= 255;
                                        temp_for_storing_received8 = temp_for_storing_received16;
                                        LEDvalues[var - NumberOfLEDs] = 0b1111111100000000 | ~(temp_for_storing_received8);
                                        if (var == 5 + NumberOfLEDs)
                                                break;
                                }
                        }
                        _update_CCRs();
                }
        }
}
 
/********************* ROUTINE T I M E R 0 ************************/
#pragma vector=TIMER0_A0_VECTOR //ccr0 - interrupt routine
__interrupt void Timer00_A(void) {
        P1OUT |= (pwmLED1_P10_T00);                     // CCR0
}
 
#pragma vector=TIMER0_A1_VECTOR //ccr1, OverFlow - interrupt routine
__interrupt void Timer01_A(void) {
        switch (TA0IV) {
        case TA0IV_TACCR1:                      // CCR1
                P1OUT |= (pwmLED2_P11_T01);
                break;
        case TA0IV_TACCR2:                      // CCR2
                P1OUT |= (pwmLED3_P12_T02);
                break;
        case TA0IV_TAIFG:                       // OverFlow
                P1OUT &= ~(pwmLED1_P10_T00);
                P1OUT &= ~(pwmLED2_P11_T01);
                P1OUT &= ~(pwmLED3_P12_T02);
                TA0R = timerZeroStartValue;
                break;
        }
}
 
 
/********************* ROUTINE T I M E R 1 ************************/
#pragma vector=TIMER1_A0_VECTOR //ccr0 - interrupt routine
__interrupt void Timer10_A(void) {
        P2OUT |= (pwmLED4_P22_T10);                     // CCR0
}
 
#pragma vector=TIMER1_A1_VECTOR //ccr1, ccr2, OverFlow - interrupt routine
__interrupt void Timer11_A(void) {
        switch (TA1IV) {
        case TA1IV_TACCR1:                      // CCR1
                P2OUT |= (pwmLED5_P23_T11);
                break;
        case TA1IV_TACCR2:                      // CCR2
                P2OUT |= (pwmLED6_P24_T12);
                break;
        case TA1IV_TAIFG:                       // OverFlow
                P2OUT &= ~(pwmLED4_P22_T10);
                P2OUT &= ~(pwmLED5_P23_T11);
                P2OUT &= ~(pwmLED6_P24_T12);
                TA1R = timerOneStartValue;
                break;
        }
}
 
/******************* INITIALIZE T I M E R 0 ***********************/
void _init_timer0() {
        TA0R = timerZeroStartValue;             //0b1111111110011011
        TA0CCTL0 = CCIE;
        TA0CCTL1 = CCIE;
        TA0CCTL2 = CCIE;
        TA0CTL = TASSEL_2 | MC_2 | ID_3 | TAIE;
}
 
/******************* INITIALIZE T I M E R 1 ***********************/
void _init_timer1() {
        TA1R = timerOneStartValue;              //0b1111111110011011
        TA1CCTL0 = CCIE;
        TA1CCTL1 = CCIE;
        TA1CCTL2 = CCIE;
        TA1CTL = TASSEL_2 | MC_2 | ID_3 | TAIE;
}
 
/******************* INITIALIZE L E D S  **************************/
void _init_CCRs() {
 
        //Timer0  0b11111111
        TA0CCR0 = 0b1111111110011100;   //p1.0  //-100
        TA0CCR1 = 0b1111111110101011;   //p1.1  //-85
        TA0CCR2 = 0b1111111110111111;   //p1.2  //-65
 
        //Timer1  0b11111111
        TA1CCR0 = 0b1111111111010011;   //p2.2  //-45
        TA1CCR1 = 0b1111111111100111;   //p2.3  //-25
        TA1CCR2 = 0B1111111111111011;   //p2.4  //-5
}
 
/********************** UPDATE L E D S  **************************/
void _update_CCRs() {
 
        //Timer0
        TA0CCR0 = LEDvalues[0];         //p1.0
        TA0CCR1 = LEDvalues[1];         //p1.1
        TA0CCR2 = LEDvalues[2];         //p1.2
 
        //Timer1
        TA1CCR0 = LEDvalues[3];         //p2.2
        TA1CCR1 = LEDvalues[4];         //p2.3
        TA1CCR2 = LEDvalues[5];         //p2.4
}
 
/******************* PRINTING ERROR MESSAGES **********************/
void Print_Error(uint8 error_code) {
 
        // Print out error code only if it is not 0
        if (error_code) {
                UART_Send_Data("Error code: ");
                UART0_Send_ByteToChar(&error_code);
                UART_Send_Data("\r\n");
                error_code = 0;
        }
 
        // If packet size has wrong length then reset radio module
        if (error_code == ERR_RX_WRONG_LENGTH) {
                Radio_Init(RF_DATA_RATE, TX_POWER, RF_CHANNEL);
        }
 
        exit_code = 0;
}
