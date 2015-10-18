#include <msp430.h>

#define LED_R BIT0
#define LED_G BIT6

#define RXD BIT1
#define TXD BIT2

#define BTN BIT3

/*
 * This is sample code for testing uart connection between MSP and Raspberry
 * TODO: transmit sample data packets instead of single bytes.
*/

unsigned char transmitChar = 'a';

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
  transmitChar = UCA0RXBUF;

  // Say that data received
  while (!(IFG2 & UCA0TXIFG));
  UCA0TXBUF = 0x06;// ACK char

  P1OUT ^= LED_G;
}

#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void) {
  P1OUT |= LED_R;
  P1IFG &= ~BTN;

  while (!(IFG2 & UCA0TXIFG));
  UCA0TXBUF = transmitChar;

  P1OUT ^= LED_R;
}
