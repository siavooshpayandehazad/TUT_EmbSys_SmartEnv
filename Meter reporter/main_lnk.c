/*
 * main.h
 *
 *  Created on: 06.02.2015
 *  Modifiedon: 10.04.2015
 *  Author: Mairo Leier, Maksim Gorev
 *
 *  Version: 0.2.1		09.10.2015
 */
/***************************************************************************************************
 *	        Include section					                       		   					       *
 ***************************************************************************************************/

#include "system.h"
#include "network.h"
#include "general.h"

// Drivers
#include "uart.h"
#include "spi.h"
#include "radio.h"

/***************************************************************************************************
 *	        Define section					                       		   					       *
 ***************************************************************************************************/


/***************************************************************************************************
 *	        Prototype section					                       							   *
 ***************************************************************************************************/


/***************************************************************************************************
 *	        Global Variable section  				                            				   *
 ***************************************************************************************************/
uint8 error;
uint8 payload_length;
uint8 cntr;


/***************************************************************************************************
 *         Main section                                                                            *
 ***************************************************************************************************/
void main(void) {
	error = 0;

	// Initialize system speed, communication interfaces and RF radio module
	System_Init(&error);

	while (1) {

		payload_length = 0;

		// Construct the packet
		TxPacket[payload_length++] = 0x01;
		TxPacket[payload_length++] = 0x02;
		TxPacket[payload_length++] = 0x03;

		// Send data over radio
		Radio_Tx(TxPacket, payload_length, ADDR_REMOTE, &error);

		// Print out packet that was sent
		UART_Send_Data("\r\nSending:");
		for (cntr = 0; cntr < payload_length; cntr++) {
			UART0_Send_ByteToChar(&TxPacket[cntr]);
		}

		// Add some delay (around 1sec)
		__delay_cycles(2000000*SYSTEM_SPEED_MHZ);
	}

}		/* END: main */
