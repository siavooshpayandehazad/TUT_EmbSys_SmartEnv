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
 $                 $Date::   13.10.2015                                                                                       $
 *                                                                                    	 				      *
 *                 Manager:       Yuanxu Xu                                                                                   *
 *                 Developers:    Martin Grosberg  		                                                              *
 *                  		  Aivar Koodi			                                                              *
 *				  Marek Aare			                                                              *
 *		                  Sander Sinijärv	 	                                                              *
 *		                  Kätlin Lahtvee                                                                              *
 *                 Target:        Fishfeeder Project 2015 for communication MSP430                                            *
 *                                                                                                                            *
 *****************************************************************************************************************************/

/***************************************************************************************************
 *	main.c  - Template for the MSP430 Launchpad project   								   		   *
 ***************************************************************************************************/

/***************************************************************************************************
 *	        Include section					                       		   					       *
 ***************************************************************************************************/
#include <stdint.h>
#include "system.h"
#include "network.h"
#include "general.h"

// Drivers
#include "uart.h"
#include "spi.h"
#include "radio.h"

// Network
#include "nwk_security.h"
#include "nwk_radio.h"

/***************************************************************************************************
 *	        Define section					                       		   					       *
 ***************************************************************************************************/
#define LED_0 BIT0
#define LED_OUT P1OUT
#define LED_DIR P1DIR


/***************************************************************************************************
 *	        Prototype section					                       							   *
 ***************************************************************************************************/
uint8_t compareArrays(uint8_t *data_transmit_buffer, uint8_t *data_memory_buffer, uint8_t data_slots);
void Print_Error(uint8 error_code);

/***************************************************************************************************
 *	        Global Variable section  				                            *
 ***************************************************************************************************/
uint8 exit_code = 0;		// Exit code number that is set after function exits
uint8 payload_length;

int request = 0;
int model_output = 0;
int data_recive_number = 0;

char data_slots = 6;
int process = 0;
char data_in;

uint8 data_memory_buffer[6];
uint8 data_transmit_buffer[6];

/***************************************************************************************************
 *         Main section                                                                            *
 ***************************************************************************************************/
void main(void) {

	WDTCTL = WDTPW | WDTHOLD;

		if(CALBC1_1MHZ==0xFF)
		{
			while(1);
		}
		DCOCTL = 0;
		BCSCTL1 = CALBC1_1MHZ;
		DCOCTL = CALDCO_1MHZ;

//	    LED_DIR = (LED_0 ); //LEDs are set as output
//	    LED_OUT &= ~(LED_0 ); //LEDs are turned off

		P1SEL  |= BIT1 + BIT2;	 	//Enable module function on pin
		P1SEL2 |= BIT1 + BIT2;  	//Select second module; P1.1 = RXD, P1.2=TXD

		UCA0CTL1 |= UCSSEL_2;    	// Set clock source SMCLK
		UCA0BR0 = 104;           	// 1MHz 9600 (N=BRCLK/baudrate)
		UCA0BR1 = 0;             	// 1MHz 9600
		UCA0MCTL = UCBRS0;       	// Modulation UCBRSx = 1
		UCA0CTL1 &= ~UCSWRST;    	// **Initialize USCI state machine**

		IE2 |= UCA0RXIE;         	//enable RX interrupt

		__enable_interrupt();
//	uint8 cntr = 0;

	// Initialize system
	Print_Error(System_Init());

	while (1) {

		if (process == 1)
		{
				request = compareArrays(data_transmit_buffer, data_memory_buffer, data_slots);
//				request = critical_request(model_output);
				process = 0;
		}
		if (request == 1)
		{
			payload_length = 0;

			// Construct the packet
			// This is the place where you can put your own data to send
			TxPacket[payload_length++] = PKT_CTRL | PKT_CTRL_REQUEST;
			TxPacket[payload_length++] = PKT_TYPE_VOLTAGE;
			TxPacket[payload_length++] = data_transmit_buffer[0];
			TxPacket[payload_length++] = data_transmit_buffer[1];
			TxPacket[payload_length++] = data_transmit_buffer[2];
			TxPacket[payload_length++] = data_transmit_buffer[3];
			TxPacket[payload_length++] = data_transmit_buffer[4];
			TxPacket[payload_length++] = data_transmit_buffer[5];

			// Send data over RF and get the exit code to check for errors
			exit_code = Radio_Send_Data(TxPacket, payload_length, ADDR_REMOTE, PAYLOAD_ENC_ON, PCKT_ACK_ON);

			// Add some delay (around 2sec)
			__delay_cycles(5000000*SYSTEM_SPEED_MHZ);

			request = 0;
		}
	}
}

/* END: main */

uint8_t compareArrays(uint8_t *data_transmit_buffer, uint8_t *data_memory_buffer, uint8_t data_slots)
{
	int data_compare_number = 0;
	int found = 0;
	for(data_compare_number = 0; data_compare_number < data_slots; data_compare_number++)
	{
		if ((data_transmit_buffer[data_compare_number] ^ data_memory_buffer[data_compare_number]) !=0 )
		{
			found = 1;
		}

	}

	if(found==1){
			P1OUT ^= LED_0;
			__delay_cycles(3000);
			P1OUT ^= LED_0;
			__delay_cycles(3000);
			P1OUT ^= LED_0;
			__delay_cycles(3000);
			P1OUT ^= LED_0;
			int data_copy_number = 0;
			for (data_copy_number = 0; data_copy_number < data_slots; data_copy_number++)
						{
							data_transmit_buffer[data_copy_number] = data_memory_buffer[data_copy_number];
						}
			return 1;
		} else {
			return 0;
		}

}

#pragma vector=USCIAB0RX_VECTOR		//interrupt service for recieving data
__interrupt void USCI0RX_ISR(void)
{
	data_in = UCA0RXBUF;

	if (data_in == 0xFF)
	{
		data_recive_number = 0;
		//P1OUT |= LED_0;
	} else {
		if (data_recive_number < data_slots)
		{
			data_memory_buffer[data_recive_number++] = data_in;
		}
		if (data_recive_number == data_slots)
		{
			process = 1;
			data_recive_number = 0;
		}
	}
	_NOP();

}


// *************************************************************************************************
// @fn          Print_Error
// @brief       Print code that is set when function exits. If error code is 0 then nothin is prnted
//				out and packet is received or sent correctly
// @param       uint8 error_code		Error code number that is set by the function
// @return      none
// *************************************************************************************************
void Print_Error(uint8 error_code) {

	// Print out error code only if it is not 0
	if (error_code) {
		UART_Send_Data("Error code: ");
		UART0_Send_ByteToChar(&error_code);
		UART_Send_Data("\r\n");
		error_code = 0;
	}

	// If packet size has wrong length then reset radio module
	if (error_code == ERR_RX_WRONG_LENGTH) {
		Radio_Init(RF_DATA_RATE, TX_POWER, RF_CHANNEL);
	}

	exit_code = 0;
}
/***************************************************************************************************
 *         History of changes                                                                       *
 ***************************************************************************************************/

//[Week 13] Took the given RF code and started to modify it to meet our needs
//[Week 14] Got the data from one MSP to another and from there to the server by implementing UART interrupt 
//[Week 15] Added compare functionalities, which made it possible to send values only when something changes in the aquarium.
