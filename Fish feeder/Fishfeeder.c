/******************************************************************************************************************************
 *                                                                                                                            *
 *                                         COPYRIGHT (C) 2015    Tallinn University of Technology                             *
 *                                                                                                                            *
 ******************************************************************************************************************************
 *                                                                                                                            *
 *                                                                                                                            *
 *                                              CCCCC   FFFFFF    AAA                                                         *
 *                                             C        F        A   A                                                        *
 *                                             C        FFFF    A     A                                                       *
 *                                             C        F       AAAAAAA                                                       *
 *                                             C        F       A     A                                                       *
 *                                              CCCCC   F       A     A                                                       *
 *                                                                                                                            *
 *                                           Common firmware application SW                                                   *
 *                                                                                                                            *
 *                                                                                                                            *
 $                 $Workfile::   main.c                                                                                       $
 $                 $Revision::   1.0                                                                                          $
 $                     $Date::   13.10.2015                                                                                   $
 *                                                        								      								  *
 *                    Manager:   Yuanxu Xu                                                                                    *
 *                 Developers:   Martin Grosberg									      *
 *                  		 Aivar Koodi										      *
 *				 Marek Aare										      *
 *		                 Sander Sinijärv									      *
 *		                 Kätlin Lahtvee                                                                               *
 *                     Target:   Fishfeeder Project 2015                                                                      *
 *                                                                                                                            *
 *****************************************************************************************************************************/

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

/***************************************************************************************************
 *	        Prototype section					                       							   *
 ***************************************************************************************************/


/***************************************************************************************************
 *	        Global Variable section  				                            				   *
 ***************************************************************************************************/


/***************************************************************************************************
 *         Main section                                                                            *
 ***************************************************************************************************/
int main(void) {
    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer
	
	return 0;
}

void init_pins (void)
{
	P2SEL &=(~BIT0); //Set SEL P2.0 as GPIO
	P2DIR &=(~BIT0); //input direction
	P2IES |=(BIT0);
	P2IFG  &=  (~BIT0);    //  Clear   interrupt   flag    for P2.0
	P2IE   |=  (BIT0); //  Enable  interrupt   for P2.0
}


void timerA(){

}

void timerB(){

}

void temperature(){

}

int lighting(int LightLevel){
	
	/*Init ADC*/
	ADC10CTL0 = ADC10IE + ADC10SHT_3 + ADC10ON;
	ADC10CTL1 = INCH_0 + ADC10DIV_3;

	__delay_cycles(5000);       	// settle time 30ms

	/*ADC conversion start*/
	__delay_cycles(1000);           // Wait for reference to settle
	ADC10CTL0 |= ENC + ADC10SC; 	// Sampling and conversion start
	while(ADC10CTL1 & BUSY);    	// Wait..i am converting...
	light = ADC10MEM;                    // Read ADC memory
	light = light / 3;

	while (!(IFG2&UCA0TXIFG));      // Wait until TX buffer is ready
	UCA0TXBUF = light;
	
	if(light < 130){
		return 1;
		}
	else{
		return 0;
		}
	}
int waterlevel()
{
	if (P2IN == 1){
		return 1;
	}
	else
	{
		return 0;	
	}
}

void feedlevel(){
	
}

void motor(){
	
}

void UART(){
	/*setting up UART*/
	P1SEL  |= BIT1 + BIT2;  //Enable module function on pin
	P1SEL2 |= BIT1 + BIT2;  //Select second module; P1.1 = RXD, P1.2=TXD

	UCA0CTL1 |= UCSSEL_2;    // Set clock source SMCLK
	UCA0BR0 = 104;           // 1MHz 9600 (N=BRCLK/baudrate)
	UCA0BR1 = 0;             // 1MHz 9600
	UCA0MCTL = UCBRS0;       // Modulation UCBRSx = 1
	UCA0CTL1 &= ~UCSWRST;    // **Initialize USCI state machine**
}
void clockSetup1MHZ(void){
	// Configure CPU clock
	if (CALBC1_1MHZ==0xFF)		    // If calibration constant erased
	{
		while(1);                   // do not load, trap CPU
	}
	DCOCTL = 0;                     // Select lowest DCOx and MODx settings
	BCSCTL1 = CALBC1_1MHZ;          // Set DCO to 1MHz
	DCOCTL = CALDCO_1MHZ;
	/* END OF DO NOT MODIFY */
}
/***************************************************************************************************
 *         History of changes                                                                       *
 ***************************************************************************************************/

//[Week 8] Added empty function slots for future code.
//[week 11] [10.11.2015 00:24] Added function init_pins whisch is inizilizing the water level sensor pin as input
// Added waterllevel function which is returning eather 1 or 0
// Added Uart inizilation function which will start communication bridge between uart
// Added Calibration function for clock -- main function is still empty because we have no idea how to arrange yet everything.
