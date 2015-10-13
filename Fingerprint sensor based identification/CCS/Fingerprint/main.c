#include <msp430.h> 

/*
 * main.c
 */
int main(void) {
	int test = 0;
    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer
	printf("Bonjour !");
	while(test)
		printf("test again");
	return 0;
}
