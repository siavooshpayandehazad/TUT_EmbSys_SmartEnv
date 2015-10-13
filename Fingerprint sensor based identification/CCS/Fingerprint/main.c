#include <msp430.h> 

/*
 * main.c
 */
int main(void) {
    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer
	printf("La vie est cool !");
	return 0;
	// Tqoginwsognoijn
}
