#ifndef UART_H_
#define UART_H_

/***************************************************************************************************
 *	        Include section					                       		   					       *
 ***************************************************************************************************/
#include "system.h"
#include "stdint.h"
/***************************************************************************************************
 *	        Define section					                       		   					       *
 ***************************************************************************************************/
/*The size of the UART receive buffer.  Set smaller if RAM is in short supply.
Set larger if larger data chunks are to be received, or if the application
can't process incoming data very often   */
#define RXBUF_SIZE  (12)

/* The threshold within bcUartRcvBuf at which main() will be awakened.  Must
be less than BC_RXBUF_SIZE.  A value of '1' will alert main() whenever
even a single byte is received.  If no wake is desired, set to
BC_RXBUF_SIZE+1     */
#define RX_WAKE_THRESH  (1)

/***************************************************************************************************
 *	        Prototype section					                       							   *
 ***************************************************************************************************/

void UART_Init(void);
void UART_Send_Data(const char *str);
void UART_Send_Byte(unsigned char c);
unsigned int UartReceiveBytesInBuffer(unsigned char* buf);
void UartSendLen(unsigned char * buf, unsigned char len);

/***************************************************************************************************
 *	        Global Variable section  				                            				   *
 ***************************************************************************************************/



#endif /* UART_H_ */
