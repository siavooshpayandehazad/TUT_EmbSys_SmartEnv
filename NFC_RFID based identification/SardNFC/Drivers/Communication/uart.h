#ifndef UART_H_
#define UART_H_

/***************************************************************************************************
 *	        Include section					                       		   					       *
 ***************************************************************************************************/
#include "system.h"

/***************************************************************************************************
 *	        Define section					                       		   					       *
 ***************************************************************************************************/
#define MAX_UART_STR_LENGTH 	60		// UART max incoming string size
#define AMARINO_END_STR			0x13

/***************************************************************************************************
 *	        Prototype section					                       							   *
 ***************************************************************************************************/
void UART_Init(void);
void UART_Send_Byte(unsigned char c);
void UART_Send_Data(char *str);
void UART0_Send_ByteToChar(unsigned char bytes[]);
uint8 UART_Receive_Data(uint8 *uart_data_buf);

uint8 uart_buf_size();
uint8 uart_read();

/***************************************************************************************************
 *	        Global Variable section  				                            				   *
 ***************************************************************************************************/
volatile uint8 uart_buf_head;
volatile uint8 uart_buf_tail;
volatile char uart_rcv_buf[MAX_UART_STR_LENGTH];


#endif /* UART_H_ */
