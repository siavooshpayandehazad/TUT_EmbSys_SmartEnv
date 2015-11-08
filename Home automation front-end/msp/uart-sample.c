#include <msp430.h>

#define LED_R BIT0
#define LED_G BIT6

#define RXD BIT1
#define TXD BIT2

#define BTN BIT3

/*
 * This is sample code for testing uart connection between MSP and Raspberry
*/

// Packet structure: {data length, from, to, encryption, conf, data, ...}
// Total length 21
unsigned char packet[21] = {16, 4, 2, 0, 0, 'C', 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 0, 0, 0, 0, 0};
unsigned char headerSize = 5;
unsigned char totalSize = 21;

unsigned char receivedPacket[21] = {16, 4, 2, 0, 0, 'C', 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 0, 0, 0, 0, 0};

// Function declarations
void uartSend(unsigned char* data);
void onPacketReceved(unsigned char* packet);

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

// While receiving data to replace current packet variable,
// this is index where received byte should be written in array.
unsigned char receivePosition = 0;
// Length of current incoming packet.
unsigned char receiveLength;

// TODO: timeout between consecutively received bytes
#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void) {
  P1OUT |= LED_R;

  receivedPacket[receivePosition] = UCA0RXBUF;

  if (receivePosition == 0) {
    receiveLength = receivedPacket[receivePosition] + headerSize;
  }

  ++receivePosition;

  if (receivePosition == receiveLength) {

    while (receivePosition < totalSize) {
      receivedPacket[receivePosition++] = 0;
    }

    receivePosition = 0;

    onPacketReceved(receivedPacket);

    // TODO: Should acknowledge that packet is received?
    // Some special char should be used that connot be 1st byte of any real packet.
    // For example, 0x01, because no real packet has lenght of 1 (first byte is length)
  }

  P1OUT ^= LED_R;
}

#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void) {
  P1OUT |= LED_R;
  P1IFG &= ~BTN;

  uartSend(packet);

  P1OUT ^= LED_R;
}

/**
 * Send multiple bytes over uart.
 */
void uartSend(unsigned char* data) {
  unsigned char dataLength = data[0] + headerSize;
  unsigned char i = 0;
  for (; i < dataLength; ++i) {
    while (!(IFG2 & UCA0TXIFG));
    UCA0TXBUF = data[i];
  }

  while (!(IFG2 & UCA0TXIFG));
}

void onPacketReceved(unsigned char* packet) {
  if (packet[5] != 'L') {
    return;
  }

  if (packet[6] != 0) {
    P1OUT |= LED_G;
  } else {
    P1OUT &= ~LED_G;
  }
}
