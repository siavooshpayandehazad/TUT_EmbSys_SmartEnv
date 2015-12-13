/*
 * main.h
 *
 *  Created on: 06.02.2015
 *  Modifiedon: 10.04.2015
 *  Author: Mairo Leier, Maksim Gorev
 *
 *  Version: 0.1		10.04.2015
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
uint8 exit_code = 0;		// Exit code number that is set after function exits
uint8 payload_length;
uint8 cntr;


/***************************************************************************************************
 *         Main section                                                                            *
 ***************************************************************************************************/
void main(void) {
	error = 0;

	System_Init(&error);
	__enable_interrupt();

	while (1) {

		uint8 len;
		uint8 rssi_env;
		uint8 rssi_rx;

		// Put radio into RX mode
		Radio_Set_Mode(RADIO_RX);

		// Receive data from Radio
		if (Radio_Rx(RxPacket, &len, 1000, &rssi_rx)) {

			// If error
			if (error == ERR_RX_WRONG_LENGTH) {
				UART_Send_Data("!RF reInit!\r\n");
				Radio_Init(RF_DATA_RATE, TX_POWER, RF_CHANNEL, &error);
			}

		} else { // No error

			LED1_TOGGLE();		// Toggle LED1

			// Print out received packet
			UART_Send_Data("\r\nReceiving:");
			for (cntr = 0; cntr < len; ++cntr) {
				UART0_Send_ByteToChar(&(RxPacket[cntr]));
			}

			// Get RSSI reading
			rssi_env = Radio_Get_RSSI();

			UART_Send_Data("\r\nRSSI env | rx: ");
			UART0_Send_ByteToChar(&(rssi_env));
			UART_Send_Data(" ");
			UART0_Send_ByteToChar(&(rssi_rx));
		}
	}	/* END: while loop */

}		/* END: main */
