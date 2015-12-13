/*
 * main.h
 *
 *  Created  on: 06.02.2015
 *  Author: Mairo Leier, Maksim Gorev
 *
 *  Version: 0.3		15.10.2015
 */
#define radioRx
#define radioTx
/***************************************************************************************************
 *	        Include section					                       		   					       *
 ***************************************************************************************************/
#include "MSP_FPS_GT511C3.hpp"
#include <stdio.h>

extern "C" {
	#include "system.h"
	#include "network.h"
	#include "general.h"

	// Drivers
	#include "uart.h"
	#include "spi.h"
	#include "radio.h"
	#include "LCD_lib.h"

	// Network
	#include "nwk_security.h"
	#include "nwk_radio.h"
}

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
	uint8 fingerNumber = 0, lastValidFinger = 0, i;
	uint8 cntr, len, var, doorStatus = CLOSE;
	char bufferFingerNumber[5];

	if((fingerNumber = System_Init()) == EXIT_NO_ERROR)
		lcdword("Starting ...");
	else
	{
		lcdword("Init failed !");
		lcdcursor(0,1);
		sprintf(bufferFingerNumber,"%d",fingerNumber);
	}

	FPS_GT511C3 fps(4, 3);
	fps.UseSerialDebug = false;
	fps.Open();
	while(!fps.SetLED(true));
	P1OUT |= BIT6;

	lcdclear();
	lcdcursor(0,0);
	lcdword("Waiting for");
	lcdcursor(0,1);
	lcdword("finger   ");

	while(1)
	{
		if(fps.IsPressFinger())                                  //check if a finger is pressed because it can't be done with interrupts
		{
			fps.CaptureFinger(false);
			fingerNumber = fps.Identify1_N();
			sprintf(bufferFingerNumber,"%d",fingerNumber);

			lcdclear();
			//if a fingerprint has been found into the database
			if(fingerNumber < 20)
			{
				//display the number
				lcdcursor(0,0);
				lcdword("Finger        ");
				lcdcursor(0,1);
				lcdword(bufferFingerNumber);
				lastValidFinger = fingerNumber;

				#ifdef radioTx
				//send tp the server the finger ID
				payload_length = 0;

				// Clear Tx packet buffer
				cntr = 0;
				for (cntr=RF_BUFFER_SIZE; cntr > 0; cntr--)
					TxPacket[cntr] = 0;

				// Construct the packet
				// This is the place where you can put your own data to send
				TxPacket[payload_length++] = PKT_CTRL | PKT_CTRL_REQUEST;
				TxPacket[payload_length++] = PKT_TYPE_VOLTAGE;
				TxPacket[payload_length++] = 'S';
				TxPacket[payload_length++] = lastValidFinger;

				/* NWK level data sending */
				lcdcursor(0,0);
				lcdword("Sending ...  ");
				i = 0;
				while(!Radio_Send_Data(TxPacket, payload_length, ADDR_REMOTE, PAYLOAD_ENC_ON, PCKT_ACK_ON) || i < 5)
					i++;

				Radio_Set_Mode(RADIO_RX);
				#endif
			}
			else
			{
				lcdcursor(0,0);
				lcdword("Not found   ");
			}
		}
		else
		{
			lcdcursor(0,0);
			lcdword("Waiting ...  ");
			lcdcursor(0,1);
			lcdword("           ");
			P1OUT |= BIT0;
			__delay_cycles(50000);
			P1OUT &= ~BIT0;
			__delay_cycles(50000);

			#ifdef radioRx
			//Radio part
			if (exit_code = Radio_Receive_Data(RxPacket, &len, 100, &rssi_rx)) {

			} else {
				lcdcursor(0,0);
				lcdword("Msg received");

				__delay_cycles(1000000*SYSTEM_SPEED_MHZ);

				//check if msg received is the door status
				if(RxPacket[5] == 'R')
				{
					doorStatus = RxPacket[6];

					lcdcursor(0,0);
					if(doorStatus)
						lcdword("Door open  ");
					else
						lcdword("Door close  ");
				}

				// Clear received packet buffer
				for (var = RF_BUFFER_SIZE-1; var > 0; var--)
						RxPacket[var] = 0;

				// Set radio into RX mode
				Radio_Set_Mode(RADIO_RX);
			}
			#endif
		}
	}
}		/* END: main */
