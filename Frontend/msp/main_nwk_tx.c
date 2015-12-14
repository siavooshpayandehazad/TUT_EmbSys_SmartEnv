/*
 * main.h
 *
 *  Created  on: 06.02.2015
 *  Author: Mairo Leier, Maksim Gorev
 *
 *  Version: 0.3		15.10.2015
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

// Network
#include "nwk_security.h"
#include "nwk_radio.h"

/***************************************************************************************************
 *	        Define section					                       		   					       *
 ***************************************************************************************************/
int data;
int ADDR;

/***************************************************************************************************
 *	        Prototype section					                       							   *
 ***************************************************************************************************/
void Print_Error(uint8 error_code);

#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void) {
	data = UCA0RXBUF;
	if (data == 0x73) {
		SEND = 1;
	}
	__bic_SR_register_on_exit(LPM0_bits);
}

#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void) {
	if (P1IFG & 0x02) {
		P1IFG &= ~(0x02);
		_NOP();
		UART_Send_Data("RX interrpt\r\n");
	}

	// Wake up from low power mode
	__bic_SR_register_on_exit(LPM0_bits);
}

/***************************************************************************************************
 *	        Global Variable section  				                            				   *
 ***************************************************************************************************/
uint8 exit_code = 0;		// Exit code number that is set after function exits
uint8 payload_length;
uint16 counter = 0;

/***************************************************************************************************
 *         Main section                                                                            *
 ***************************************************************************************************/
void main(void) {
	uint8 cntr, len, var;
	uint8 rssi_env, rssi_rx;

	// Initialize system
	Print_Error(System_Init());
	UART_Send_Data("Start transcieveing...");
	P1IES = 0x02;
	P1IE = 0x02;
	SEND = 0;
	RECV = 0;
	// Set radio into RX mode
	Radio_Set_Mode(RADIO_RX);
	while (1) {
		if (RECV) {
			// Receive data, wait until 100ms until timeout and continue with other tasks
			if (exit_code = Radio_Receive_Data(RxPacket, &len, 100, &rssi_rx)) {
				if (exit_code != 0x0B) Print_Error(exit_code);
			} else {
				// Toggle LED1 when data is received
				LED1_TOGGLE();

				counter++;

				for (var = 0; var < len; ++var) {
					UART0_Send_ByteToChar(&(RxPacket[var]));
					UART_Send_Data(" ");
				}
				UART_Send_Data("\r\n");

				// Get environemnt RSSI value (noise level)
				rssi_env = Radio_Get_RSSI();

				// Print the SNR
				//UART_Send_Data("\r\n");
				//UART0_Send_ByteToChar(&(rssi_rx));
				//UART_Send_Data("/");
				//UART0_Send_ByteToChar(&(rssi_env));

				//UART_Send_Data("\r\n");		// Insert new line to separate packets
			}

			// Clear received packet buffer
			for (var = RF_BUFFER_SIZE-1; var > 0; var--)
				RxPacket[var] = 0;
			RECV = 0;
			Radio_Set_Mode(RADIO_RX);
		}

		if (SEND) {
			ADDR = 0x04;
			//Radio_Set_Mode(RADIO_TX);
			payload_length = 0;
			TxPacket[payload_length++] = PKT_CTRL | PKT_CTRL_REQUEST;
			TxPacket[payload_length++] = 0xAA;
			TxPacket[payload_length++] = 0x12;
			TxPacket[payload_length++] = 0x34;
			TxPacket[payload_length++] = 0x56;
			//UART_Send_Data("Sending packet...");
			exit_code = Radio_Send_Data(TxPacket, payload_length, ADDR, PAYLOAD_ENC_OFF, PCKT_ACK_ON);
			UART0_Send_ByteToChar(&exit_code);
			UART_Send_Data("\r\n");
			SEND = 0;
			Radio_Set_Mode(RADIO_RX);
		}

		//__delay_cycles(50000*SYSTEM_SPEED_MHZ);

		// Enter to LPM0 w/ interrupts enabled
		// Module stays in low power mode until interrupt is received from radio and wakes up
		//__bis_SR_register(LPM0_bits + GIE);
	}
}		/* END: main */

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
	}

	// If packet size has wrong length then reset radio module
	if (error_code == ERR_RX_WRONG_LENGTH) {
		Radio_Init(RF_DATA_RATE, TX_POWER, RF_CHANNEL);
	}

	exit_code = 0;
}

