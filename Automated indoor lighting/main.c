#include <msp430.h> 

#define LED1 BIT0
#define LED2 BIT6

#define RX		BIT1
#define TX		BIT2
#define NumberOfLEDs 2 //if you want to change number of LEDs edit this line only.

//read UART
#define readUart	UART_data_I = UCA0RXBUF;
#define writeUartInArray ledValue_I[ledInpCount_I] = UART_data_I;
#define checkArrayIndexer ledInpCount_I =((ledInpCount_I == NumberOfLEDs - 1) ? 0 : (ledInpCount_I + 1));

//LED values writing
int ledValue_I[NumberOfLEDs] = { 0 };
int ledInpCount_I = 0;
int UART_data_I = 0;

/*
 * DO NOT MODIFY
 */
void _init() {
	WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer

	//Configure CPU clock
	if (CALBC1_1MHZ == 0xFF) {
		while (1)
			;
	}

	DCOCTL = 0;
	BCSCTL1 = CALBC1_1MHZ;
	DCOCTL = CALDCO_1MHZ;
}

void _init_UART(){

	P1SEL |= RX + TX;
	P1SEL2 |= RX + TX;

	UCA0CTL1 |= UCSSEL_2;

	UCA0BR0 = 104;
	UCA0BR1 = 0;
	UCA0MCTL = UCBRS0;
	UCA0CTL1 &= ~UCSWRST;

    //Enable RX interupt
    IE2 |= UCA0RXIE;
}

int main(void) {
	_init();
    _init_UART()

	P1DIR |= LED1 + LED2;
	P1OUT &= ~(LED1 + LED2);
	P1SEL |= LED1 + LED2;

	// PWM period = 125 KHz / 625 = 200 Hz
	TACCR0 = 625;

	// Source Timer A from SMCLK (TASSEL_2), up mode (MC_1).
	// Up mode counts up to TACCR0.
	TACTL = TASSEL_2 + MC_1 + ID_3;

	// OUTMOD_7 = Reset/set output when the timer counts to TACCR1/TACCR0
	// CCIE = Interrupt when timer counts to TACCR1
	TACCTL1 = OUTMOD_7 | CCIE;
	// TACCTL1 = CCIE;
	// TACCTL2 = CCIE;

	// Initial CCR1 (= brightness)
	TACCR0 = 625;
	TACCR1 = 200;
	TACCR2 = 100;
	ledValue_I[0]= 200;
	ledValue_I[1]= 100;

	__enable_interrupt();
	__bis_SR_register(LPM0 | GIE);

	return 0;
}

#pragma vector=TIMER0_A1_VECTOR
__interrupt void Timer_A1(void) {
	TACCTL1 &= ~CCIFG;
	TACCR1 = ledValue_I[0];
	/*
	switch ( TAIV ) {
	case 2:
		TACCR1 = ledValue_I[0];
		break;                          // CCR1 not used
	case 4:
		TACCR2 = ledValue_I[1];
		break;                          // CCR2 not used
	}

	switch ( TAIV ) {
	case 2:
		TACCTL1 &= ~CCIFG;
		break;                          // CCR1 not used
	case 4:
		TACCTL2 &= ~CCIFG;
		break;                          // CCR2 not used
	}
	*/
}

//UART interrupt
#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void) {
//to make it work, send 8 bits of LED data one by one - LEDNUMBER times, From UART

	//data from UART is written into a variable UART_data_I
	readUart
	;
	//UART_data_I is written into a appropriate array element
	writeUartInArray
	;
	//if array element counter(that points to) is more than 7, it is resetted to 0
	checkArrayIndexer
	;
}