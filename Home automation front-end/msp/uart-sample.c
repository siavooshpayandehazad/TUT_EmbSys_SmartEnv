#include <msp430.h>

#define LED_R BIT0
#define LED_G BIT6

#define RXD BIT1
#define TXD BIT2

#define BTN BIT3

/*
 * This is sample code for testing uart connection between MSP and Raspberry
*/

// Packet structure: {length, from, to, data, ...}
// Max length 19. Extra byte for delimiter
unsigned char packet[20] = {8, 2, 4, 'C', 'a', 'b', 'c', 'd', 0};
// Byte that indicates the end of packet.
const unsigned char packetDelimiter = 0;

// While receiving data to replace current packet variable,
// this is index where received byte should be written in array.
unsigned char receivePosition = 0;

/**
 * Send multiple bytes over uart.
 */
void uartSend(unsigned char* data) {
  while (*data != packetDelimiter) {
    while (!(IFG2 & UCA0TXIFG));
    UCA0TXBUF = *data;
    ++data;
  }

  // Wait for last byte to be sent
  while (!(IFG2 & UCA0TXIFG));
}

int main(void) {
  /* DO NOT MODIFY */
  // Stop watchdog timer
  WDTCTL = WDTPW | WDTHOLD;

  // Configure CPU clock
  if (CALBC1_1MHZ == 0xFF) {// If calibration constant erased
    while (1);// do not load, trap CPU
  }

  DCOCTL = 0;// Select lowest DCOx and MODx settings
  BCSCTL1 = CALBC1_1MHZ;// Set DCO to 1MHz
  DCOCTL = CALDCO_1MHZ;
  /* END OF DO NOT MODIFY */

  // Enable peripheral module function on UART pins
  P1SEL |= RXD + TXD;//Enable module function on pin
  P1SEL2 |= RXD + TXD;//Select second module

  //Initialize UART module with the speed of 9600 bps
  UCA0CTL1 |= UCSSEL_2;// Set clock source SMCLK
  UCA0BR0 = 104;// 1MHz 9600 (N=BRCLK/baudrate)
  UCA0BR1 = 0;// 1MHz 9600
  UCA0MCTL = UCBRS0;// Modulation UCBRSx = 1
  UCA0CTL1 &= ~UCSWRST;// **Initialize USCI state machine**

  // Enable UART RX interrupt
  IE2 |= UCA0RXIE;

  P1DIR |= LED_R + LED_G;
  P1OUT &= ~(LED_R + LED_G);

  // Configure push button
  P1DIR &= ~BTN;
  P1IE |= BTN;
  P1IES |= BTN;
  P1REN |= BTN;
  P1OUT |= BTN;

  P1IFG &= ~BTN;

  __enable_interrupt();
  __bis_SR_register(LPM0_bits + GIE);

	return 0;
}

#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void) {
  P1OUT |= LED_G;

  packet[receivePosition] = UCA0RXBUF;

  if (packet[receivePosition] == packetDelimiter) {
    receivePosition = 0;
    // TODO: Should acknowledge that packet is received?
    // Some special char should be used that connot be 1st byte of any real packet.
    // For example, 0x01, because no real packet has lenght of 1 (first byte is length)
  } else {
    ++receivePosition;
  }

  P1OUT ^= LED_G;
}

#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void) {
  P1OUT |= LED_R;
  P1IFG &= ~BTN;

  uartSend(&packet);

  P1OUT ^= LED_R;
}
