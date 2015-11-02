#include <msp430.h> 

#define LED1 BIT0
#define LED2 BIT6

#define RX		BIT1
#define TX		BIT2

int pos = 0;   // Index to PWM's duty cycle table (= brightness)

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

	// Initial CCR1 (= brightness)
	TACCR1 = 0;

	__enable_interrupt();
	__bis_SR_register(LPM0 | GIE);

	return 0;
}

#pragma vector=TIMER0_A1_VECTOR
__interrupt void Timer_A1(void) {
	TACCTL1 &= ~CCIFG;
	TACCR1 = pos;
}

#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void) {
	pos = UCA0RXBUF;
}