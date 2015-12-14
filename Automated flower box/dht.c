/***************************************************************************************************
 *	        Include section					                       		   					       *
 ***************************************************************************************************/
#include <stdlib.h>
#include "system.h"
#include "dht.h"

/***************************************************************************************************
 *	        Define section					                       		   					       *
 ***************************************************************************************************/


/***************************************************************************************************
 *	        Prototype section					                       							   *
 ***************************************************************************************************/


/***************************************************************************************************
 *	        Global Variable section  				                            				   *
 ***************************************************************************************************/
volatile unsigned txdata = 0;
uint8 quot1, rem1;


void DHT_Init(void) {
	TACCTL0 = OUT;                                    // Setup serial tx I/O
	TA0CTL = TASSEL_2 | MC_2;                        // TimerA SMCLK, continuous
}

#pragma vector = TIMER0_A0_VECTOR
__interrupt void Timer0_A0(void) {
    if(txdata) {
        (txdata & 1) ? (TA0CCTL0 &= ~OUTMOD2) : (TA0CCTL0 |=  OUTMOD2);
        txdata >>= 1;
        TA0CCR0 += 104;
    } else {
        TA0CCTL0 &= ~CCIE;
    }
}

void putc(const char c) {
    while(CCTL0 & CCIE);
    txdata = 0x0200 | (c << 1);
    TA0CCR0 = TAR + 16;
    TA0CCTL0 = OUTMOD0 | CCIE;
}

void print(const char *s) {
    while(*s) putc(*s++);
}


void print_hex_digit(unsigned n) {
    putc("0123456789ABCDEF"[n & 0x0F]);
}

void print_hex_byte(unsigned n) {
    print_hex_digit(n >> 4);
    print_hex_digit(n);
}


void print_dec2(int n) {

    // Print 2 decimal digits  0 to 99
    const div_t d = div(n, 10);
    if(d.quot) {
    	quot1 = '0' + d.quot;
    	putc('0' + d.quot);
    }
    rem1 = '0' + d.rem;
    putc('0' + d.rem);
}


int8 read_dht(unsigned char *p) {
                                                                			// Note: TimerA must be continuous mode (MC_2) at 1 MHz
    const unsigned char *end = p + 6;                           			// End of data buffer
    register unsigned char m = 1;                               			// First byte will have only start bit
    register unsigned st, et;                                   			// Start and end times

    p[0] = p[1] = p[2] = p[3] = p[4] = p[5] = 0;                			// Clear data buffer

    DHT_LO();		                                            			// Pull low
    DHT_OUT();             			                            			// Output
    DHT_PORT_REN &= ~DHT_PIN;                                         		// Drive low
    st = TAR; while((TAR - st) < 18000);                        			// Wait 18 ms
    DHT_PORT_REN |= DHT_PIN;                                          		// Pull low
    DHT_IN();                                          						// Pull high
    DHT_IN();                                         						// Input

    st = TAR;                                                   			// Get start time for timeout
    while(DHT_PORT_IN & DHT_PIN) if((TAR - st) > 100) return -1;      		// Wait while high, return if no response
    et = TAR;                                                   			// Get start time for timeout
    do {
        st = et;                                                			// Start time of this bit is end time of previous bit
        while(!(DHT_PORT_IN & DHT_PIN)) if((TAR - st) > 100) return -2;     // Wait while low, return if stuck low
        while(DHT_PORT_IN & DHT_PIN) if((TAR - st) > 200) return -3;        // Wait while high, return if stuck high
        et = TAR;                                               			// Get end time
        if((et - st) > 110) *p |= m;                            			// If time > 110 us, then it is a one bit
        if(!(m >>= 1)) m = 0x80, ++p;                           			// Shift mask, move to next byte when mask is zero
    } while(p < end);                                           			// Do until array is full

    p -= 6;                                                     			// Point to start of buffer
    if(p[0] != 1) return -4;                                    			// No start bit
    if(((p[1] + p[2] + p[3] + p[4]) & 0xFF) != p[5]) return -5; 			// Bad checksum

    return 0;                                                   			// Good read
}


