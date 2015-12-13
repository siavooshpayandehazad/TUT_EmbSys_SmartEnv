/*
 * {main.c}
 *
 * {main application}
 *
 */
#include <NFC.h>
#include "system.h"
#include "network.h"
#include "general.h"

// Drivers
#include "uart.h"
#include "soft_spi.h"
#include "radio.h"

// Network
#include "nwk_security.h"
#include "nwk_radio.h"

u08_t stand_alone_flag = 1;
u08_t enable = 0;
uint8 MasterMode = 0;
uint8 sysStatus = 0;

void Print_Error(uint8 error_code);
void Action(void);

void main(void)
{

	uint8 CardPacket [16]={1,2,3,4,5,6,7,8,9,0};
	uint8 StatusPacket [16];
	uint8 RSSI, error, len;
	uint8 sendStatus = 0, lockStatus = 6;


	// WDT ~350ms, ACLK=1.5kHz, interval timer
	WDTCTL = WDT_ADLY_16;

	// Configure CPU clock
	System_Set_Speed(SYSTEM_SPEED_MHZ);

	// Enable WDT interrupt
	IE1 |= WDTIE;
	McuDelayMillisecond(2);
	NFC_Init();

	// Ports  *P1.0 Should be reassigned to RELAY, P1.1 to GREEN LED, P1.2 RED LED
	//LED_PORT_DIR|= (LED_G);				//Set LED1 pin as output
	//LED_PORT_OUT&= ~(LED_G);				// Set LED1 pin to LOW
	LED_PORT_DIR|= (LED_R);				//Set LED1 pin as output
	LED_PORT_OUT|= (LED_R);				// Set LED1 pin to LOW
	BUZZER_PORT_DIR|= (BUZZER);   //Set BUZZER pin as output
	BUZZER_PORT_OUT&= ~(BUZZER);    // Set BUZZER pin to LOW
	//RELAY_PORT_DIR|= (RELAY);   //Set Relay pin as output
	//RELAY_PORT_OUT&= ~(RELAY);    // Set Relay pin to LOW
	RF_RESET_OUT();							// Set RF module reset pin as output

	// Configure UART
	UART_Init();		//
	SPI_Init();			// RF module interface

	McuDelayMillisecond(5);
	UART_Send_Data("Reader enabled.\n\r");
	McuDelayMillisecond(2);

	Print_Error(Radio_Init(RF_DATA_RATE, TX_POWER, RF_CHANNEL));	// Initialize RF module with 2kbps speed
	Print_Error(Radio_Set_Channel(RF_CHANNEL));

	// General enable interrupts
	__bis_SR_register(GIE);

	// launchpad LED1
	//P1DIR |= BIT0;

	while(1)
	{
		// Enter LPM3
		//__bis_SR_register(LPM3_bits);
		if(NFC_Read())
		{
			uint8 retries;
			uint8 i;
			Print_Card();

			for ( i=0 ;i<10;i++)
			{
				CardPacket[i+2]= Card_UID[i];
			}
			CardPacket[1]='C';

			error = Radio_Send_Data(CardPacket, 12, ADDR_SERVER, PAYLOAD_ENC_OFF, PCKT_ACK_ON);
			Print_Error(error);
			if(error == ERR_NO_ACK)
			{
				for(retries = 5;retries;retries--)
				{
					UART_Send_Data("NWK ACK ERR");
					for ( i=0 ;i<10;i++)
					{
						CardPacket[i+2]= Card_UID[i];
					}
					CardPacket[1]='C';
					if(!(Radio_Send_Data(CardPacket, 12, ADDR_SERVER, PAYLOAD_ENC_ON, PCKT_ACK_ON)))
						break;
				 }
			}

			// Set radio into RX mode
			Radio_Set_Mode(RADIO_RX);
			while(NFC_Read());
		}
		if(sendStatus)
		{
			uint8 retries;
			uint8 i;

			StatusPacket[1]='S';
			StatusPacket[2]=RSSI;
			StatusPacket[3]=Card_RSSI;
			StatusPacket[4]=lockStatus;
			StatusPacket[5]=sysStatus;

			error = Radio_Send_Data(StatusPacket, 12, ADDR_SERVER, PAYLOAD_ENC_ON, PCKT_ACK_ON);
			Print_Error(error);
			if(error == ERR_NO_ACK)
			{
				for(retries = 5;retries;retries--)
				{
					UART_Send_Data("NWK ACK ERR");
					StatusPacket[1]='S';
					StatusPacket[2]=RSSI;
					StatusPacket[3]=Card_RSSI;
					StatusPacket[4]=lockStatus;
					StatusPacket[5]=sysStatus;
					if(!(Radio_Send_Data(StatusPacket, 12, ADDR_SERVER, PAYLOAD_ENC_ON, PCKT_ACK_ON)))
						break;
				 }
			}

			// Set radio into RX mode
			Radio_Set_Mode(RADIO_RX);
			sendStatus = 0;
		}

		// Receive data, wait until 500ms until timeout and continue with other tasks
		if (error = Radio_Receive_Data(RxPacket, &len, 1, &rssi_rx))
		{
			// If error
			if (error == ERR_RX_WRONG_LENGTH)
			{
				UART_Send_Data("DBG RF ReInit!\r\n");
				Radio_Init(RF_DATA_RATE, TX_POWER, RF_CHANNEL);
			}
		}
		else
		{// No error
			uint8 var;
			// See the unencrypted data that was received
			UART_Send_Data("\r\nGot packet");
			UART_Send_Data("\r\nLength:");
			UART0_Send_ByteToChar(&(RxPacket[0]));
			UART_Send_Data("\r\nTo:");
			UART0_Send_ByteToChar(&(RxPacket[1]));
			UART_Send_Data("\r\nFrom:");
			UART0_Send_ByteToChar(&(RxPacket[2]));
			UART_Send_Data("\r\nMessage:");
			for (var = 5; var < len; ++var)
			{
				UART_Send_Byte(RxPacket[var]);
			}

			// Print the RSSI values (NOISE LEVEL | RECEIVED_SIGNAL_SRENGTH)
			UART_Send_Data("\r\nDBG RSSI rx: ");
			UART0_Send_ByteToChar(&(rssi_rx));

			if(RxPacket[5] == 'R')
			{
				UART_Send_Data("Status Request\r\n");
				sendStatus = 1;
			}
			if(RxPacket[5] == 'L')
			{
				UART_Send_Data("Changing lock\r\n");
				//Do lock stuff here
				Action();
			}
		}
	}
}

// Watchdog Timer interrupt service routine
#pragma vector=WDT_VECTOR
__interrupt void watchdog_timer(void)
{
   //exit LPM3
   __bic_SR_register_on_exit(LPM3_bits);
}

void Print_Error(uint8 error_code)
{
	// Print out error code only if it is not 0
	if (error_code)
	{
		UART_Send_Data("Error code: ");
		UART0_Send_ByteToChar(&error_code);
		UART_Send_Data("\r\n");
		error_code = 0;
	}

	// If packet size has wrong length then reset radio module
	if (error_code == ERR_RX_WRONG_LENGTH)
		Radio_Init(RF_DATA_RATE, TX_POWER, RF_CHANNEL);
}

#define DOOR_STATUS 1
#define MASTER_MODE 2

void Action(void)
{
	//Open door
	if (RxPacket[6] == '1'&& !(sysStatus & DOOR_STATUS))
	{
		//LED_PORT_OUT &= ~(LED_R);  //RED LED OFF
		//LED_PORT_OUT |= (LED_G);  //GREEN LED ON
		//RELAY_PORT_OUT |= (RELAY);  //RELAY ON
		BUZZER_PORT_OUT |= (BUZZER); //BUZZER ON
		McuDelayMillisecond(100);  //Delay 0.5s
		BUZZER_PORT_OUT &= ~(BUZZER); //BUZZER OFF
		sysStatus |= DOOR_STATUS;

	}

	//Close door
	else if  (RxPacket[6] != '1'&& (sysStatus & DOOR_STATUS))
	{
		//LED_PORT_OUT |= (LED_R);  //RED LED ON
		//LED_PORT_OUT &= ~(LED_G);  //GREEN LED OFF
		//RELAY_PORT_OUT &= ~(RELAY);  //RELAY OFF
		BUZZER_PORT_OUT |= (BUZZER); //BUZZER ON
		McuDelayMillisecond(300);  //Delay 0.5s
		BUZZER_PORT_OUT &= ~(BUZZER); //BUZZER OFF
		sysStatus &= ~DOOR_STATUS;
	}

	//Master Mode Enabled
	if (RxPacket[7] == '1' && !(sysStatus & MASTER_MODE))
	{
		int i;
		LED_PORT_OUT |= (LED_R);  //RED LED ON
		//LED_PORT_OUT |= (LED_G);  //GREEN LED ON
		for(i=0;i<4;i++)
		{
			BUZZER_PORT_OUT |= (BUZZER); //BUZZER ON
			McuDelayMillisecond(50);  //Delay 0.5s
			BUZZER_PORT_OUT &= ~(BUZZER); //BUZZER OFF
			McuDelayMillisecond(200);  //Delay 0.5s
		}
		sysStatus |= MASTER_MODE;

	}
	else if (RxPacket[7] == '0' && (sysStatus & MASTER_MODE))
	{
		int i;
		LED_PORT_OUT &= ~(LED_R);  //GREEN LED OFF
		//LED_PORT_OUT |= (LED_R);  //RED LED ON
		for(i=0;i<4;i++)
		{
			BUZZER_PORT_OUT |= (BUZZER); //BUZZER ON
			McuDelayMillisecond(50);  //Delay 0.5s
			BUZZER_PORT_OUT &= ~(BUZZER); //BUZZER OFF
			McuDelayMillisecond(200);  //Delay 0.5s
		}
		sysStatus &= ~MASTER_MODE;

	}



	//WRONG CARD
	if (RxPacket[8] == '1')
	{
		uint8 j;

		//LED_PORT_OUT &= ~(LED_G);  //GREEN LED OFF
		for(j=0 ;j<3;j++)
		{
			//LED_PORT_OUT |= (LED_R);  //RED LED ON
			BUZZER_PORT_OUT |= (BUZZER); //BUZZER ON
			McuDelayMillisecond(500);  //Delay 0.5s
			BUZZER_PORT_OUT &= ~(BUZZER); //BUZZER OFF
			//LED_PORT_OUT &= ~(LED_R);  //RED LED OFF
			McuDelayMillisecond(500);  //Delay 0.5s
		}
	}
}
