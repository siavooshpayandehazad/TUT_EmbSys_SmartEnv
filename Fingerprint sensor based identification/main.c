/***************************************************************************************************
 *	main.c  - Template for the MSP430 Launchpad project   								   		   *
 ***************************************************************************************************/

/***************************************************************************************************
 *	        Include section					                       		   					       *
 ***************************************************************************************************/
#include <msp430.h> 

/***************************************************************************************************
 *	        Define section					                       		   					       *
 ***************************************************************************************************/
#define TXLED BIT0
#define RXLED BIT6

#define TXD BIT2
#define RXD BIT1

typedef struct
{
	char cmdStart1;
	char cmdStart2;
	char deviceID[2];
	char parameter[4];
	char command[2];
	char checksum[2];
}Commande;

Commande commande;

/***************************************************************************************************
 *	        Prototypes 				                            				   *
 ***************************************************************************************************/
void delay_ms();

/***************************************************************************************************
 *         Main section                                                                            *
 ***************************************************************************************************/
void send_command()
{
	while(!(IFG2 & UCA0TXIFG));
		UCA0TXBUF = commande.cmdStart1;
	while(!(IFG2 & UCA0TXIFG));
		UCA0TXBUF = commande.cmdStart2;
	while(!(IFG2 & UCA0TXIFG));
		UCA0TXBUF = commande.deviceID[0];
	while(!(IFG2 & UCA0TXIFG));
		UCA0TXBUF = commande.deviceID[1];
	while(!(IFG2 & UCA0TXIFG));
		UCA0TXBUF = commande.parameter[0];
	while(!(IFG2 & UCA0TXIFG));
		UCA0TXBUF = commande.parameter[1];
	while(!(IFG2 & UCA0TXIFG));
		UCA0TXBUF = commande.parameter[2];
	while(!(IFG2 & UCA0TXIFG));
		UCA0TXBUF = commande.parameter[3];
	while(!(IFG2 & UCA0TXIFG));
		UCA0TXBUF = commande.command[0];
	while(!(IFG2 & UCA0TXIFG));
		UCA0TXBUF = commande.command[1];
	while(!(IFG2 & UCA0TXIFG));
		UCA0TXBUF = commande.checksum[0];
	while(!(IFG2 & UCA0TXIFG));
		UCA0TXBUF = commande.checksum[1];
}

void init_command()
{
	commande.cmdStart1 = 0x55;
	commande.cmdStart2 = 0xAA;
	commande.deviceID[0] = 0x01;
	commande.deviceID[1] = 0x00;
	commande.parameter[0] = 0x00;
	commande.parameter[1] = 0x00;
	commande.parameter[2] = 0x00;
	commande.parameter[3] = 0x00;
	commande.command[0] = 0x01;
	commande.command[1] = 0x00;
	commande.checksum[0] = 0x01;
	commande.checksum[1] = 0x01;
}

void LED_command()
{
	commande.cmdStart1 = 0x55;
	commande.cmdStart2 = 0xAA;
	commande.deviceID[0] = 0x01;
	commande.deviceID[1] = 0x00;
	commande.parameter[0] = 0x01;
	commande.parameter[1] = 0x00;
	commande.parameter[2] = 0x00;
	commande.parameter[3] = 0x00;
	commande.command[0] = 0x12;
	commande.command[1] = 0x00;
	commande.checksum[0] = 0x01;
	commande.checksum[1] = 0x13;
}

void main(void)
{
	/* Configuration Watchdog, I/O */

    WDTCTL = WDTPW | WDTHOLD;		// Stop watchdog timer

    /* Configuration CPU Clock */

    if (CALBC1_1MHZ==0xFF)		    // If calibration constant erased
    {
    	while(1);                   // do not load, trap CPU
    }
    DCOCTL = 0;
    BCSCTL1 = CALBC1_1MHZ; // Set DCO = 1MHz
    DCOCTL = CALDCO_1MHZ;

    /* Configuration UART */

    P2OUT &= 0x00; 					// All P2.x reset
    P2DIR = 0xFF; 					// All P2.x outputs<
	P1SEL |= RXD + TXD ; 			// P1.1 = RXD, P1.2=TXD Enable module function on pin
	P1SEL2 |= RXD + TXD ; 			// Select second module; P1.1 = RXD, P1.2=TXD
	P1DIR |= RXLED + TXLED;
	P1OUT &= 0x00;

	UCA0CTL0 = 0b00000000;			//initialize UART
    UCA0CTL1 |= UCSSEL_2;			// Set clock source SMCLK
    UCA0BR0 = 104;                 	// 1MHz 9600 (N=BRCLK/baudrate)
    UCA0BR1 = 0;					// 1MHz 9600
    UCA0MCTL = UCBRS0;				// Modulation UCBRSx = 1
    UCA0CTL1 &= ~UCSWRST;			// **Initialize USCI state machine** --> software reset enable
    UC0IE |= UCA0RXIE; 				// Enable USCI_A0 RX interrupt
    UC0IE |= UCA0TXIE; 				// Enable USCI_A0 TX interrupt

    __enable_interrupt();

    init_command();
    send_command();

    LED_command();
    send_command();

    while(1);
}

#pragma vector=USCIAB0TX_VECTOR
__interrupt void USCI0TX_ISR(void)
{
	int i;
	P1OUT |= TXLED;
	for(i = 0; i<50000; i++);
	P1OUT &= ~TXLED;
}

#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void)
{
	int i;
	P1OUT |= RXLED;
	for(i = 0; i<50000; i++);
	P1OUT &= ~RXLED;
}

