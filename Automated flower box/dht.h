#ifndef DHT_H_
#define DHT_H_


/***************************************************************************************************
 *	        Include section					                       		   					       *
 ***************************************************************************************************/


/***************************************************************************************************
 *	        Define section					                       		   					       *
 ***************************************************************************************************/
// DHT Ports
#define DHT_PORT_DIR          P2DIR
#define DHT_PORT_OUT          P2OUT
#define DHT_PORT_IN           P2IN
#define DHT_PORT_IE           P2IE
#define DHT_PORT_IES          P2IES
#define DHT_PORT_IFG          P2IFG
#define DHT_PORT_REN		  P2REN
#define DHT_PIN               BIT3

#define DHT_IN()  			{ DHT_PORT_DIR &= ~DHT_PIN; }
#define DHT_OUT()  			{ DHT_PORT_DIR |=  DHT_PIN; }
#define DHT_HI()        	{ DHT_PORT_OUT |=  DHT_PIN; }
#define DHT_LO()        	{ DHT_PORT_OUT &= ~DHT_PIN; }


// DHT statuses
#define DHT_SUCCESS_STATUS 	 0
#define DHT_CRC_ERROR 		-1
#define DHT_LINK_ERROR 		-2

/***************************************************************************************************
 *	        Prototype section					                       							   *
 ***************************************************************************************************/
void DHT_Init(void);
int8 read_dht(unsigned char *p);

void putc(const char c);
void print_dec2(int n);
void print(const char *s);
void print_hex_byte(unsigned n);


/***************************************************************************************************
 *	        Global Variable section  				                            				   *
 ***************************************************************************************************/

#endif /* DHT_H_ */
