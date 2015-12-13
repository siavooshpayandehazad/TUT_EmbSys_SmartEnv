#ifndef CONFIG_GENERAL_H_
#define CONFIG_GENERAL_H_

/***************************************************************************************************
 *	        Include section					                       		   					       *
 ***************************************************************************************************/

/***************************************************************************************************
 *	        Define section					                       		   					       *
 ***************************************************************************************************/

/********* DEBUG **********/
#define DEBUG				1		// Send all information to the UART
#define DEBUG_RF			1		// Send all RF related data to the UART
#define DEBUG_ERR			1		// Send only error information to the UART


/********* SYSTEM **********/

/* System speed settings. Available options:
 * SYSTEM_SPEED_1MHZ
 * SYSTEM_SPEED_8MHZ
 * SYSTEM_SPEED_12MHZ
 * SYSTEM_SPEED_16MHZ */
#define SYSTEM_SPEED_MHZ	SYSTEM_SPEED_16MHZ		// MCU speed in MHz. Can be between 1 to 16 MHz


#endif 
