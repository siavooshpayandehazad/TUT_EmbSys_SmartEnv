/***************************************************************************************************
 *	main.c  - Template for the MSP430 Launchpad project   								   		   *
 ***************************************************************************************************/

/***************************************************************************************************
 *	        Include section					                       		   					       *
 ***************************************************************************************************/
#include <msp430.h>
#include <stdlib.h>
#include "i2c_1602_lib.h"
#include "MSP_FPS_GT511C3.hpp"
#include "uartCustom.h"




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

int main(void)
{
  volatile unsigned int i;

  WDTCTL = WDTPW+WDTHOLD;                   // Stop WDT
  P1DIR |= BIT0;                            // P1.0 set as output                     // For debugger

  /* Configuration CPU Clock */

  if (CALBC1_1MHZ==0xFF)		    // If calibration constant erased
  {
	while(1);                   // do not load, trap CPU
  }
  DCOCTL = 0;
  BCSCTL1 = CALBC1_1MHZ; 			// Set DCO = 1MHz
  DCOCTL = CALDCO_1MHZ;

  /* Configuration I2C */

  UCB0CTL1 |= UCSWRST; 					// Enable SW reset
  UCB0CTL0 = UCMST + UCMODE_3 + UCSYNC; 	// I2C Master, synchronous mode
  UCB0CTL1 = UCSSEL_2 + UCSWRST; 			// Use SMCLK, keep SW reset
  UCB0BR0 = 10; 							// fSCL = 1Mhz/10 = ~100kHz
  UCB0BR1 = 0;
  UCB0I2CSA = 0x20; 						// I2C Adress for PCF8574T
  UCB0CTL1 &= ~UCSWRST; 					// **Initialize USCI state machine**


  UartInit(); // Initialize Debug Serial on pins P4.4, P4.5

  __bis_SR_register(GIE);       // Enter LPM0, interrupts enabled
  // __no_operation();

/*
  initLCD();
  lcdcursor(0,0);
  lcdword("Hello user !");
  lcdcursor(0,1);
  lcdword("Waiting...");
*/

  FPS_GT511C3 fps(1, 2);
  fps.UseSerialDebug = false;
  fps.Open();

  while(1)                                  // continuous loop
  {
	P1OUT |= BIT0;
    fps.SetLED(true);
    for(i=50000;i>0;i--);                   // Delay
    P1OUT &= ~BIT0;
    fps.SetLED(false);
    for(i=50000;i>0;i--);					// Delay
  }
}


// *************************************************************************************************
// @fn          ExampleFunction
// @brief       Give an answer depeding on number of ticks
// @param       ticks
// @return      x
// *************************************************************************************************
