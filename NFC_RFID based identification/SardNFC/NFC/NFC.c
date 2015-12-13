//===============================================================
// Program with hardware USART and SPI communication	        ;
// interface with TRF7970A reader chip.                         ;
//                                                              ;
// PORT1.0 - HEART BEAT LED                                     ;
// PORT1.1 - UART RX                                            ;
// PORT1.2 - UART TX                                            ;
// PORT1.5 - SPI DATACLK                                        ;
// PORT1.6 - SPI MISO (REMOVE LED2 JUMPER)                      ;
// PORT1.7 - SPI MOSI                                           ;
//                                                              ;
// PORT2.0 - IRQ (INTERUPT REQUEST from TRF7970A)               ;
// PORT2.1 - SLAVE SELECT                                       ;
// PORT2.2 - TRF7970A ENABLE                                    ;
// PORT2.3 - ISO14443B LED                                      ;
// PORT2.4 - ISO14443A LED                                      ;
// PORT2.5 - ISO15693  LED                                      ;
//===============================================================

/********** HEADER FILES **********/
//===============================================================
#include <NFC.h>
/********** GLOBAL VARIABLES **********/
//===============================================================
u08_t Card_RSSI;
u08_t Card_UID[14];
u08_t buf[100];					// TX/RX BUFFER FOR TRF7970A
u08_t i_reg = 0x01;             // INTERRUPT REGISTER
volatile u08_t irq_flag = 0x00;
u08_t rx_error_flag = 0x00;
s08_t rxtx_state = 1;           // USED FOR TRANSMIT RECEIVE BYTE COUNT
u08_t host_control_flag = 0;
u08_t Tag_Count;

//===============================================================


void NFC_Init(void)
{
    SLAVE_SELECT_PORT_SET;
	SLAVE_SELECT_HIGH;

	ENABLE_PORT_SET;
	ENABLE_TRF;

	// wait until TRF7970A system clock started
	McuDelayMillisecond(2);

	// settings for communication with TRF7970A
	Trf7970CommunicationSetup();

	// Set Clock Frequency and Modulation
	Trf7970InitialSettings();

	// set the DCO to 8 MHz
	//McuOscSel(1);

	// Re-configure the USART with this external clock
	Trf7970ReConfig();

}

int NFC_Read(void)
{
	Tag_Count = 0;
	// launchpad LED1 - Toggle (heartbeat)
	//P1OUT |= BIT0;

	// Clear IRQ Flags before enabling TRF7970A
	IRQ_CLR;
	IRQ_ON;

	ENABLE_TRF;

	// Must wait at least 4.8 ms to allow TRF7970A to initialize.
	__delay_cycles(40000);


	Iso14443aFindTag();	// Scan for 14443A tags
	IRQ_OFF;
	DISABLE_TRF;

	// Write total number of tags read to UART
	if(Tag_Count > 0){
		//P1OUT &= ~BIT0;
/*		Tag_Count = UartNibble2Ascii(Tag_Count & 0x0F);	// convert to ASCII
		UartSendCString("Tags Found: ");
		UartPutChar(Tag_Count);
		UartPutCrlf();
		UartPutCrlf();
*/
		return 1;
	}
	else
		return 0;
}
void Print_Card(void){
	int i=0;
	for(i=0;i<8;i++)
	{
		UART0_Send_ByteToChar(&Card_UID[i]);
	}
	UART_Send_Data("\n\r");
	UART0_Send_ByteToChar(&Card_RSSI);
	UART_Send_Data("\n\r");


}
