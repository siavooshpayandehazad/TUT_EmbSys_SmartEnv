#ifndef FINGERPRINTSENSORRADIO_DRIVERS_GPIO_H_
#define FINGERPRINTSENSORRADIO_DRIVERS_GPIO_H_

/***************************************************************************************************
 *	        Include section					                       		   					       *
 ***************************************************************************************************/


/***************************************************************************************************
 *	        Define section					                       		   					       *
 ***************************************************************************************************/

/******** MRF89XA *********/
#define RF_IRQ_PORT_DIR		(P2DIR)
#define RF_IRQ_PORT_IN		(P2IN)
#define RF_IRQ_PORT_REN		(P2REN)
#define RF_IRQ_PORT_IE		(P2IE)
#define RF_IRQ_PORT_IES		(P2IES)
#define RF_IRQ_PORT_IFG		(P2IFG)

#define RF_IRQ0_PIN			BIT0	// P2.0
#define RF_IRQ1_PIN			BIT2	// P2.2

#define RF_RESET_PORT_DIR	(P2DIR)
#define RF_RESET_PORT_OUT	(P2OUT)
#define RF_RESET_PIN		BIT5	// P2.5


/********* SPI ************/
#define SPI_PORT_DIR      	(P3DIR)
#define SPI_PORT_SEL       	(P3SEL)
#define SPI_PORT_OUT       	(P3OUT)

#define SPI_CS_PORT_DIR     (P1DIR)
#define SPI_CS_PORT_OUT     (P1OUT)

#define SPI_MISO_PIN       	BIT1	// P3.1
#define SPI_MOSI_PIN       	BIT0	// P3.0
#define SPI_CLK_PIN        	BIT2	// P3.2
#define SPI_CS_DATA_PIN    	BIT3	// P1.3
#define SPI_CS_CONF_PIN    	BIT4	// P1.4 chip select


/********** LED ***********/
#define LED_PORT_DIR      	(P1DIR)
#define LED_PORT_OUT       	(P1OUT)

#define LED1  	     		BIT0	// P1.0 (RED)
//#define LED2       		BIT6	// P1.6 (GREEN); used by SPI


/******** UART *************/
#define UART_PORT_DIR		(P3DIR)
#define UART_PORT_SEL      	(P3SEL)

#define UART_RXD			BIT4	// P3.4
#define UART_TXD			BIT3	// P3.3


/******** I2C *************/
#define I2C_1602_SCL_PORT_OUT 	P7OUT
#define I2C_1602_SCL_PORT_DIR 	P7DIR
#define I2C_1602_SDA_PORT_OUT 	P7OUT
#define I2C_1602_SDA_PORT_DIR 	P7DIR
#define I2C_1602_SDA_PORT_REN 	P7REN
#define I2C_1602_SDA_PORT_IN 	P7IN
#define I2C_1602_SCL_PIN 		4
#define I2C_1602_SDA_PIN 		0

/******** FPS *************/
#define	FPS_DIR		P8DIR
#define FPS_OUT		P8OUT
#define	FPS_PIN		BIT1	// P8.1


/***************************************************************************************************
 *	        Prototype section					                       							   *
 ***************************************************************************************************/


/***************************************************************************************************
 *	        Global Variable section  				                            				   *
 ***************************************************************************************************/

#endif
