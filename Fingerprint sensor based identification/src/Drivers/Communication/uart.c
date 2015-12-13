/***************************************************************************************************
 *	        Include section					                       		   					       *
 ***************************************************************************************************/
#include "uart.h"
#include "radio.h"

/***************************************************************************************************
 *	        Define section					                       		   					       *
 ***************************************************************************************************/


/***************************************************************************************************
 *	        Prototype section					                       							   *
 ***************************************************************************************************/


/***************************************************************************************************
 *	        Global Variable section  				                            				   *
 ***************************************************************************************************/
// Receive buffer for the UART.  Incoming bytes need a place to go immediately,
// otherwise there might be an overrun when the next comes in.  The USCI ISR
// puts them here.
unsigned char  UartRcvBuf[RXBUF_SIZE];

// The index within bcUartRcvBuf, where the next byte will be written.
unsigned int UartRcvBufIndex = 0;

// Boolean flag indicating whether bcUartRcvBufIndex has reached the
// threshold BC_RX_WAKE_THRESH.  0 = FALSE, 1 = TRUE
unsigned char  UartRxThreshReached = 0;

// *************************************************************************************************
// @fn          Init_UART
// @brief       Initialize UART
//									Pinout: TXD = P1.1		RXD = P1.2
// @param       none
// @return      none
// *************************************************************************************************
void UART_Init(void) {
	// Define UART ports
	UART_PORT_SEL |= UART_RXD + UART_TXD;   	// P1.1 = RXD, P1.2=TXD

	// Configure UART
	UCA0CTL1 |= UCSSEL_2;                   	// SMCLK

	UCA0CTL1 |= UCSWRST + UCSSEL_2;         // **Put state machine in reset** and SMCLK
	UCA0BR0 = 104;                          // 1MHz 9600 (see User's Guide)
	UCA0BR1 = 0;                            // 1MHz 9600
	UCA0CTL1 &= ~UCSWRST;                   // **Initialize USCI state machine**

	UCA0IE |= UCRXIE;                        // Enable USCI_A0 RX interrupt
}



// *************************************************************************************************
// @fn          UART_Send_Byte
// @brief       Send char over UART
// @param       uint8 c				Char to send
// @return      none
// *************************************************************************************************
void UART_Send_Byte(unsigned char c) {
	while(!(UCA0IFG & UCTXIFG));
	UCA0TXBUF = c;
	while(UCA0STAT & UCBUSY);                 // Wait until the last byte is completely sent
	_NOP();
}


// *************************************************************************************************
// @fn          UART_Send_Data
// @brief       Send string over UART
// @param       const char str			String to send
// @return      none
// *************************************************************************************************
void UART_Send_Data(const char *str) {
	while (*str)
		UART_Send_Byte(*str++);
}

// Sends 'len' bytes, starting at 'buf'
void UartSendLen(unsigned char * buf, unsigned char len)
{
    unsigned char i = 0;

    // Write each byte in buf to USCI TX buffer, which sends it out
    while (i < len)
    {
        UCA0TXBUF = *(buf+(i++));

        // Wait until each bit has been clocked out...
        while(!(UCTXIFG==(UCTXIFG & UCA0IFG))&&((UCA0STAT & UCBUSY)==UCBUSY));
    }
}


// Copies into 'buf' whatever bytes have been received on the UART since the
// last fetch.  Returns the number of bytes copied.
unsigned int UartReceiveBytesInBuffer(unsigned char* buf)
{
    unsigned int i, count, Try = 0;
    while(UartRcvBufIndex != 12 && Try < 500)//wait for all data to be received
    {
    	__delay_cycles(5000);
    	Try++;
    }

    if(Try >= 500)
    	UartRcvBufIndex = 0;

    for(i = 0; i<12; i++)
    	buf[i] = 0;

    // Hold off ints for incoming data during the copy
    UCA0IE &= ~UCRXIE;

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
    UCA0IE |= UCRXIE;

    return count;
}

// The USCI_A0 receive interrupt service routine (ISR).  Executes every time a
// byte is received on the back-channel UART.
#pragma vector = USCI_A0_VECTOR
__interrupt void UartISR(void)
{
	P2IFG = 0;
	P2IE = 0;
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
    /*RF_IRQ0_IE();
    RF_IRQ1_IE();*/
}
