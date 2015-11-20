/***************************************************************************************************
 *	        Include section					                       		   					       *
 ***************************************************************************************************/
#include "system.h"
#include "network.h"
#include "uart.h"
#include "spi.h"
#include "radio.h"
#include "LCD_lib.h"

/***************************************************************************************************
 *	        Define section					                       		   					       *
 ***************************************************************************************************/

/***************************************************************************************************
 *	        Prototype section					                       							   *
 ***************************************************************************************************/

/***************************************************************************************************
 *	        Global Variable section  				                            				   *
 ***************************************************************************************************/

// *************************************************************************************************
// @fn          Init_System
// @brief       Initialize system
// @param       none
// @return      none
// *************************************************************************************************
uint8 System_Init(void) {
	uint8 exit_code = 0, var;

	// Ports
	FPS_OUT |= FPS_PIN;				// switch off FPS
	FPS_DIR |= FPS_PIN;				//Set FPS as VIN as output

	WDTCTL = WDTPW + WDTHOLD;	// Stop watchdog timer

	// Configure CPU clock
	System_Set_Speed();

	// Ports
	LED_PORT_DIR |= (LED1);				//Set LED1 pin as output
	LED_PORT_OUT &= ~(LED1);				// Set LED1 pin to LOW
	RF_RESET_OUT();							// Set RF module reset pin as output

	// Communication
	UART_Init();		//
	SPI_Init();			// RF module interface
	initLCD();

	// RF Module

	if (exit_code = Radio_Init(RF_DATA_RATE, TX_POWER, RF_CHANNEL))	// Initialize RF module with 2kbps speed
		return exit_code;

	if (exit_code = Radio_Set_Channel(RF_CHANNEL))
		return exit_code;

	// Clear received packet buffer
	for (var = RF_BUFFER_SIZE - 1; var > 0; var--)
		RxPacket[var] = 0;

	// Set radio into RX mode
	Radio_Set_Mode(RADIO_RX);

	//switch on FPS
	FPS_OUT &= ~(FPS_PIN);

	__enable_interrupt();

	return EXIT_NO_ERROR;
}

// *************************************************************************************************
// @fn          System_Set_Speed
// @brief       Initialize CPU with selected speed
// @param       uint8 mhz		MCU speed in MHz. Available options:
//									SYSTEM_SPEED_1MHZ
//									SYSTEM_SPEED_8MHZ
//									SYSTEM_SPEED_12MHZ
//									SYSTEM_SPEED_16MHZ
// @return      none
// *************************************************************************************************
void System_Set_Speed() {
	/* Configuration CPU Clock */

/*	UCSCTL3 |= SELREF_2;                      // Set DCO FLL reference = REFO
	UCSCTL4 |= SELA_2;                        // Set ACLK = REFO
	__bis_SR_register(SCG0);                  // Disable the FLL control loop
	UCSCTL0 = 0x0000;                         // Set lowest possible DCOx, MODx
	UCSCTL1 = DCORSEL_5;                     // Select DCO range 24MHz operation
	UCSCTL2 = FLLD_1 + 29;                // Set DCO Multiplier for 12MHz
										  // (N + 1) * FLLRef = Fdco
										  // (374 + 1) * 32768 = 12MHz
										  // Set FLL Div = fDCOCLK/2
	__bic_SR_register(SCG0);                  // Enable the FLL control loop

	__delay_cycles(375000);                  //
	// Loop until XT1,XT2 & DCO fault flag is cleared
	do {
		UCSCTL7 &= ~(XT2OFFG + XT1LFOFFG + DCOFFG);	// Clear XT2,XT1,DCO fault flags
		SFRIFG1 &= ~OFIFG;                      	// Clear fault flags
	} while (SFRIFG1 & OFIFG);                   // Test oscillator fault flag
	*/

}

// *************************************************************************************************
// @fn          cByteToHex
// @brief       Convert byte to char
// @param
// @return      uint8 result            Outputs char
// *************************************************************************************************
int cByteToHex(char input) {
	char result = 0;
	switch (input) {
	case 0x00:
		result = '0';
		break;
	case 0x01:
		result = '1';
		break;
	case 0x02:
		result = '2';
		break;
	case 0x03:
		result = '3';
		break;
	case 0x04:
		result = '4';
		break;
	case 0x05:
		result = '5';
		break;
	case 0x06:
		result = '6';
		break;
	case 0x07:
		result = '7';
		break;
	case 0x08:
		result = '8';
		break;
	case 0x09:
		result = '9';
		break;

	case 0x0A:
		result = 'A';
		break;
	case 0x0B:
		result = 'B';
		break;
	case 0x0C:
		result = 'C';
		break;
	case 0x0D:
		result = 'D';
		break;
	case 0x0E:
		result = 'E';
		break;
	case 0x0F:
		result = 'F';
		break;
	}
	return result;
}
