#include <msp430.h> 

/*
 * main.c
 */
int main(void) {
	int test = 0;
    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer
	printf("La vie est cool !");
	while(test)
		printf("test again");
	return 0;
}
