/* --COPYRIGHT--,BSD
 * Copyright (c) 2012, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * --/COPYRIGHT--*/
 
#include "msp430.h"
#include "uartCustom.h"

// Receive buffer for the UART.  Incoming bytes need a place to go immediately,
// otherwise there might be an overrun when the next comes in.  The USCI ISR
// puts them here.
uint8_t  UartRcvBuf[RXBUF_SIZE];

// The index within bcUartRcvBuf, where the next byte will be written.
uint16_t UartRcvBufIndex = 0;

// Boolean flag indicating whether bcUartRcvBufIndex has reached the
// threshold BC_RX_WAKE_THRESH.  0 = FALSE, 1 = TRUE
uint8_t  UartRxThreshReached = 0;

// Initializes the USCI_A0 module as a UART, using baudrate settings in
// bcUart.h.  The baudrate is dependent on SMCLK speed.

void UartInit(void)
{
    // Always use the step-by-step init procedure listed in the USCI chapter of
    // the F2xx Family User's Guide

	P1SEL  |= BIT1 + BIT2;					// Enable module function on pin
	P1SEL2 |= BIT1 + BIT2;					// Select second module; P1.1 = RXD, P1.2=TXD
    UCA0CTL1 |= UCSWRST + UCSSEL_2;         // **Put state machine in reset** and SMCLK
    UCA0BR0 = 104;                          // 1MHz 9600 (see User's Guide)
    UCA0BR1 = 0;                            // 1MHz 9600
    UCA0CTL1 &= ~UCSWRST;                   // **Initialize USCI state machine**
    IE2 |= UCA0RXIE;                        // Enable USCI_A0 RX interrupt
}


// Sends 'len' bytes, starting at 'buf'
void UartSendLen(uint8_t * buf, uint8_t len)
{
    uint8_t i = 0;

    // Write each byte in buf to USCI TX buffer, which sends it out
    while (i < len)
    {
        UCA0TXBUF = *(buf+(i++));

        // Wait until each bit has been clocked out...
        while(!(UCA0TXIFG==(UCA0TXIFG & IFG2))&&((UCA0STAT & UCBUSY)==UCBUSY));
    }
}

// Sends 'len' bytes, starting at 'buf'
void UartSend(uint8_t * buf)
{
    uint8_t i = 0;
    // uint8_t len = (sizeof(buf))/(sizeof(uint8_t)); //miks ta ei oska kokku lugeda?

    char len = 0;
    while (buf[len] != 0){
      len++;
    }

    // Write each byte in buf to USCI TX buffer, which sends it out
    while (i < len+1)
    {
        UCA0TXBUF = *(buf+(i++));

        // Wait until each bit has been clocked out...
        while(!(UCA0TXIFG==(UCA0TXIFG & IFG2))&&((UCA0STAT & UCBUSY)==UCBUSY));
    }
}

// Sends 'len' bytes, starting at 'buf'
void UartSendString(char* buf)
{
    uint8_t i = 0;

    char len = 0;
    while (buf[len] != 0){
      len++;
    }

    // Write each byte in buf to USCI TX buffer, which sends it out
    while (i < len+1)
    {
        UCA0TXBUF = *(buf+(i++));

        // Wait until each bit has been clocked out...
        while(!(UCA0TXIFG==(UCA0TXIFG & IFG2))&&((UCA0STAT & UCBUSY)==UCBUSY));
    }
}

void UartSendByte(uint8_t byte){
  while (!(IFG2 & UCA0TXIFG));
  UCA0TXBUF = byte;
}

// Copies into 'buf' whatever bytes have been received on the UART since the
// last fetch.  Returns the number of bytes copied.
uint16_t UartReceiveBytesInBuffer(uint8_t* buf)
{
    uint16_t i, count;
    while(UartRcvBufIndex != 12); //wait for all data to be received

    // Hold off ints for incoming data during the copy
    IE2 &= ~UCA0RXIE;

    //get the buffer stored duing interrupts
    for(i=0; i<UartRcvBufIndex; i++)
    {
        buf[i] = UartRcvBuf[i];
    }

    count = UartRcvBufIndex;

    //clear buffer
    for(i=0; i<UartRcvBufIndex; i++)
    	UartRcvBuf[i] = 0;
    UartRcvBufIndex = 0;     // Move index back to the beginning of the buffer
    UartRxThreshReached = 0;

    // Restore USCI interrupts, to resume receiving data.
    IE2 |= UCA0RXIE;

    return count;
}

// The USCI_A0 receive interrupt service routine (ISR).  Executes every time a
// byte is received on the back-channel UART.
#pragma vector = USCIAB0RX_VECTOR
__interrupt void UartISR(void)
{
	//synchronize data received --> it starts
	if(UCA0RXBUF == 0x55)
		UartRcvBufIndex = 0;
	else
		if(UCA0RXBUF == 0xAA) //to be sure to do not miss the start point
		{
			UartRcvBufIndex = 1;
			UartRcvBuf[0] = 0x55;
		}

	UartRcvBuf[UartRcvBufIndex++] = UCA0RXBUF;  // Fetch the byte, store
                                                    // it in the buffer

    // Wake main, to fetch data from the buffer.
    if(UartRcvBufIndex >= RX_WAKE_THRESH)
    {
        UartRxThreshReached = 1;
        __bic_SR_register_on_exit(LPM3_bits);       // Exit LPM0-3
    }
}
