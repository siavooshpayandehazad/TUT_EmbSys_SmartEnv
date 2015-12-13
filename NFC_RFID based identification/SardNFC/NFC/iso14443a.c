/*
 * {iso14443a.c}
 *
 * {ISO14443A Specific Functions & Anti-collision}
 *
 * Copyright (C) 2013 Texas Instruments Incorporated - http://www.ti.com/
 *
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *    Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 *    Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the
 *    distribution.
 *
 *    Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
*/

#include "iso14443a.h"
#include "trf7970.h"
#include "NFC.h"

#define ENABLE_HOST

//===============================================================

u08_t	complete_uid[14];
u08_t	coll_poss = 0;
u08_t	uid_pos = 0, uid_pos1;
extern u08_t buf[300];
extern u08_t i_reg;
extern u08_t irq_flag;
extern u08_t rx_error_flag;
extern s08_t rxtx_state;
extern u08_t stand_alone_flag;
extern u08_t remote_flag;
extern u08_t Tag_Count;
extern u08_t Card_UID[14];
extern u08_t Card_RSSI;

// command
#define REQA	0x26
#define WUPA	0x52

#define NVB_INIT 0x20
#define NVB_FULL 0x70
#define LEVEL1	0x93
#define LEVEL2	0x95
#define LEVEL3	0x97
#define RX_CRC		0x08
#define NO_RX_CRC	0x88

//===============================================================
// NAME: void Iso14443aFindTag(void)
//
// BRIEF: Is used to detect ISO14443A conform tags in stand alone 
// mode.
//
// INPUTS:
//	
// OUTPUTS:
//
// PROCESS:	[1] turn on RF driver
//			[2] do a complete anticollision sequence
//			[3] turn off RF driver
//
// NOTE: If ISO14443A conform Tag is detected, ISO14443A LED will
//       be turned on.
//
// CHANGE:
// DATE  		WHO	DETAIL
// 23Nov2010	RP	Original Code
//===============================================================

void
Iso14443aFindTag(void)
{
	Trf7970TurnRfOn();
	
	Trf7970WriteIsoControl(0x08);
	
	// When a PICC is exposed to an unmodulated operating field
	// it shall be able to accept a quest within 5 ms.
	// PCDs should periodically present an unmodulated field of at least
	// 5,1 ms duration. (ISO14443-3)
	McuDelayMillisecond(6);

	Iso14443aAnticollision(0x00);						// do a complete anticollision sequence as described1

	// in ISO14443-3 standard for type A
	Trf7970TurnRfOff();

	Trf7970ResetIrqStatus();
}

//===============================================================
// NAME: void Iso14443aAnticollision(u08_t reqa)
//
// BRIEF: Is used to start the ISO14443A Anticollision Loop.
//
// INPUTS:
//	Parameters:
//		u08_t		reqa		REQA or WUPA
//
// OUTPUTS:
//
// PROCESS:	[1] send REQA or WUPA command
//			[2] receive ATQA
//			[3] perform bit frame anticollison loop
//
// NOTE: Collisions returned as “(z)”.
//       Timeouts returned as “()”.
//
// CHANGE:
// DATE  		WHO	DETAIL
// 23Nov2010	RP	Original Code
//===============================================================

void
Iso14443aAnticollision(u08_t reqa)
{
	u08_t nvb = NVB_INIT;
    rxtx_state = 1;
    Iso14443_config(NO_RX_CRC);
    McuDelayMillisecond(2);

    Iso14443a_command(WUPA);			//send WUPA (0x52) to wake up all tags

	if(i_reg == 0xFF || i_reg == 0x02)
	{
		uid_pos = 0;
		Iso14443aLoop(0x01, nvb, &buf[40]);			// cascade level 1
		if(stand_alone_flag == 1)
		{
			LED_14443A_ON;
		}
	}
	else
	{
		LED_14443A_OFF;
	}

	Iso14443_config(NO_RX_CRC);
}											// Iso14443aAnticollision

//===============================================================
// NAME: void Iso14443aLoop(u08_t select, u08_t nvb, u08_t
// 			*uid)
//
// BRIEF: Is used to run through the cascade levels.
//
// INPUTS:
//	Parameters:
//		u08_t		select		indicates cascade level
//		u08_t		nvb			number of valid bits
//		u08_t		*uid		known part of UID
//	
// OUTPUTS:
//	Globals:
//		u08_t		complete_uid[14]	stores UID
//
// PROCESS:	(ISO14443-3)
//
// NOTE: Collisions returned as “(z)”.
//       Timeouts returned as “()”.
//
// CHANGE:
// DATE  		WHO	DETAIL
// 23Nov2010	RP	Original Code
//===============================================================

void
Iso14443aLoop(u08_t cascade_level, u08_t nvb, u08_t *uid)
{
	u08_t	i = 0;
	u08_t	nvbytes = 0, nvbits = 0, xbits = 0, found = 0;
	u08_t new_uid[4];
	u08_t select, new_uid1[4], coll_poss1, nvbits1;
	u08_t cascade_level1;
#ifdef ENABLE_HOST
//	u08_t rssi[2];
#endif

	while (cascade_level < 4)
	{
		switch (cascade_level)
		{
		case 1:
			select = 0x93;
			break;
		case 2:
			select = 0x95;
			break;
		case 3:
			select = 0x97;
			break;
		default:
			break;
		}

		if((nvb & 0x0F) != 0x00)
		{
			nvbytes = (nvb >> 4) - 2;			// the number of known valid bytes
			xbits = nvb & 0x07;					// the number of known valid bits

			// Both are used in the UID calculation
			for(i = 0; i < xbits; i++)
			{
				nvbits = (nvbits << 1) + 1;
			}
		}
		rx_error_flag = 0;
		coll_poss = 0x21;
		rxtx_state = 1;							// The response will be stored in buf[1] upwards
		Iso14443aSelectCommand(select, nvb, uid);

		while (coll_poss < 0x20) {
			if(i_reg == 0x00){
				break;
			}
		};
		if (coll_poss == 0x20)
			i_reg = 0x02;						// In case coll_poss=0x20 means didn't receive response
		for(i = 0; i < 5; i++)
		{
			complete_uid[i+uid_pos] = buf[i + 1];
		}
		if(rx_error_flag == 0x02)
		{
			i_reg = 0x02;
		}
		if(i_reg == 0x02 || i_reg == 0x00)		// collision or timeout
		{
			break;
		}
		if(i_reg == 0xff)						// if data received
		{
			for (i=0; i<nvbytes; i++)
				complete_uid[i+uid_pos] = *(uid + i);
			complete_uid[nvbytes+uid_pos] = (buf[1] &~nvbits) | (*(uid + nvbytes) & nvbits);
			for (i=1; i<(5-nvbytes); i++)
				complete_uid[i+nvbytes+uid_pos] = buf[1+i];

			nvb = NVB_FULL;
			rxtx_state = 1;
			Iso14443aSelectCommand(select, nvb, &complete_uid[uid_pos]);
			McuDelayMillisecond(6);
			if (buf[1] & BIT2)				//uid not complete
			{
				cascade_level++;
				uid_pos += 5;
				nvb = NVB_INIT;
			}
			else				// uid completed, print UID to uart.
			{
				#ifdef ENABLE_HOST
				Tag_Count++;
				Copy_UID(cascade_level);
				Copy_RSSI();

// Here is code for Uart sending, lets return it instead somehow
/*
				UartSendCString("ISO14443 type A: ");
				UartPutChar('[');
				switch (cascade_level)
				{
				case 1:
					for (i=0; i<4; i++)
						UartPutByte(complete_uid[i]);
					break;
				case 2:
					for (i=1; i<4; i++)
						UartPutByte(complete_uid[i]);
					for (i=5; i<9; i++)
						UartPutByte(complete_uid[i]);
					break;
				case 3:
					for (i=1; i<4; i++)
						UartPutByte(complete_uid[i]);
					for (i=6; i<9; i++)
						UartPutByte(complete_uid[i]);
					for (i=10; i<14; i++)
						UartPutByte(complete_uid[i]);
					break;
				default:
					break;
				}
				UartPutChar(',');
				rssi[0] = RSSI_LEVELS;			// read RSSI levels
				Trf7970ReadSingle(rssi, 1);
				UartPutByte(rssi[0]);
				UartPutChar(']');
				UartPutCrlf();

*/
				#endif
				if(stand_alone_flag == 1)
					found = 1;
				i_reg = 0x01;					// do nothing after
				break;
			}
		}
		_nop();
	}

	if(i_reg == 0x00)							// timer interrupt
	{	
//		if(stand_alone_flag == 1)
//		{
//			#ifdef ENABLE_HOST
//				UartPutChar('(');
//				UartSendCString("No tag found/Error");
//				UartPutChar(')');
//				UartPutCrlf();
//			#endif
//		}
	}

	if(i_reg == 0x02)									// if collision occured go into anticollision
	{
		for (i=0; i<4; i++)
		{
			new_uid[i] = buf[i+1];
		}
		// a RX interrupt will happen after collision interrupt
		McuDelayMillisecond(5);

		if (coll_poss == 0x60 || coll_poss == 0x70)			// if all of 4 or 5 bytes of last level were in collision
		{
			cascade_level++;
			uid_pos += 5;
			coll_poss = 0x20;
		}
		else
		{
			// combine UCLn collected of last loop
			for (i=0; i<nvbytes; i++)
				new_uid[i] = *(uid + i);
			new_uid[nvbytes] = (new_uid[nvbytes] &~ nvbits) | (*(uid + nvbytes) & nvbits);

		}
		// calculate new parameters
		nvbytes = (coll_poss >> 4) - 2;				// how many bytes was collected
		xbits  = coll_poss & 0x07;					// how many bits was collected
		coll_poss++;
		nvbits = 0;
		for (i = 0; i < xbits; i++)
		{
			nvbits = (nvbits << 1) + 1;				// left shift to make a mask for last broken byte (create all bit 1 belong to how many bit in broken byte)
		}
		nvbits1 = (nvbits << 1) + 1;
		nvbits1 = nvbits1 - nvbits;				// bit_mask1 use to seperate next bit after last broken bit

		new_uid[nvbytes] = new_uid[nvbytes] & nvbits;		// only keep collsion bits

		//back up before loop
		for (i=0; i<=4; i++)
		{
			new_uid1[i] = new_uid[i];
		}
		coll_poss1 = coll_poss;
		uid_pos1 = uid_pos;
		cascade_level1 = cascade_level;

		Iso14443aLoop(cascade_level, coll_poss, new_uid);		// recursive call for anticollision procedure
		McuDelayMillisecond(6);

		Iso14443a_halt();

		i_reg = 0x01;
		Iso14443a_command(WUPA);
		McuDelayMillisecond(6);

		uid_pos = uid_pos1;
		new_uid1[nvbytes] = new_uid1[nvbytes] + nvbits1;
		Iso14443aLoop(cascade_level1, coll_poss1, new_uid1);		// recursive call for anticollision procedure
	}

	if(stand_alone_flag == 1)
	{
		if(found == 1)
		{	
			LED_14443A_ON;
		}
		else
		{	
			LED_14443A_OFF;
		}
	}
}														// Iso14443aLoop

void ISO14443IRQWaitTimeout(u08_t txtimeout, u08_t rxtimeout)
{
	i_reg = 0x01;
	while(i_reg != 0x00)
	{
		McuCounterSet();
		COUNT_VALUE = COUNT_1ms * txtimeout;
		irq_flag = 0x00;
		START_COUNTER;						// start timer up mode
		while(irq_flag == 0x00)				// wait for interrupt
		{
		}
	}										// wait for end of TX
	RESET_COUNTER;

	i_reg = 0x01;
	while(i_reg == 0x01)		// wait for end of RX or timeout
	{
		McuCounterSet();
		COUNT_VALUE = COUNT_1ms * rxtimeout;
		irq_flag = 0x00;
		START_COUNTER;						// start timer up mode
		while(irq_flag == 0x00)
		{
		}									// wait for interrupt
	}
	RESET_COUNTER;
}


void Iso14443aSelectCommand(u08_t select, u08_t nvb, u08_t *uid)
{
	u08_t length;
	Iso14443_config(RX_CRC);

	length = 5 + (nvb >> 4);
	if((nvb & 0x0F) != 0x00)
	{
		length++;
	}

	buf[0] = 0x8F;							// prepare the SELECT command
	if(nvb == 0x70)							// select command, otherwise anticollision command
	{
		buf[1] = 0x91;						// transmit with CRC
	}
	else
	{
		buf[1] = 0x90;
	}
	buf[2] = 0x3D;
	buf[3] = 0x00;
	buf[4] = nvb & 0xF0;					// number of complete bytes
	if((nvb & 0x07) != 0x00)
	{
		buf[4] |= ((nvb & 0x07) << 1) + 1; 	// number of broken bits, last bit is 1 means broken byte
	}
	buf[5] = select;						// can be 0x93, 0x95 or 0x97
	buf[6] = nvb;							// number of valid bits
	buf[7] = *uid;
	buf[8] = *(uid + 1);
	buf[9] = *(uid + 2);
	buf[10] = *(uid + 3);
	buf[11] = *(uid + 4);

	Trf7970RawWrite(&buf[0], length);

	ISO14443IRQWaitTimeout(5,50);
	McuDelayMillisecond(1);
}

void Iso14443a_halt()
{
	Iso14443_config(NO_RX_CRC);

	buf[0] = 0x8F;							// prepare the SELECT command
		buf[1] = 0x90;
	buf[2] = 0x3D;
	buf[3] = 0x00;
	buf[4] = 0x20;					// number of complete bytes
	buf[5] = 0x50;						//halt
	buf[6] = 0x00;							// number of valid bits

	Trf7970RawWrite(&buf[0], 7);

	i_reg = 0x01;
	while(i_reg != 0x00)
	{
		McuCounterSet();
		COUNT_VALUE = COUNT_1ms * 2;		// 2ms for TIMEOUT
		irq_flag = 0x00;
		START_COUNTER;						// start timer up mode
		while(irq_flag == 0x00)				// wait for interrupt
		{
		}
		RESET_COUNTER;
	}										// wait for end of TX
}

void Iso14443a_command(u08_t command)
{
    buf[0] = 0x8F;
    buf[1] = 0x90;
    buf[2] = 0x3D;
    buf[3] = 0x00;
    buf[4] = 0x0F;
	buf[5] = command;

    rxtx_state = 1;

    Trf7970RawWrite(&buf[0], 6);

    IRQ_CLR;								// PORT2 interrupt flag clear
	IRQ_ON;

	ISO14443IRQWaitTimeout(5,50);
}


void Iso14443_config(u08_t crc)
{
	Trf7970WriteIsoControl(crc);
	Trf7970ReadSingle(buf,1);
}

void Copy_UID(u08_t cascade_level)
{
	u08_t	j = 0, i = 0;

	// Delete old data
	for(i=0; i<14; i++)
		Card_UID[i]=0;

	switch (cascade_level)
	{
	case 1:
		for (i=0; i<4; i++)
		{
			Card_UID[j] = complete_uid[i];
			j++;
		}
		break;
	case 2:
		for (i=1; i<4; i++)
		{
			Card_UID[j] = complete_uid[i];
			j++;
		}
		for (i=5; i<9; i++)
		{
			Card_UID[j] = complete_uid[i];
			j++;
		}
		break;
	case 3:
		for (i=1; i<4; i++)
		{
			Card_UID[j] = complete_uid[i];
			j++;
		}
		for (i=6; i<9; i++)
		{
			Card_UID[j] = complete_uid[i];
			j++;
		}
		for (i=10; i<14; i++)
		{
			Card_UID[j] = complete_uid[i];
			j++;
		}
		break;
	default:
		break;
	}

}

void Copy_RSSI(void){
	u08_t rssi[2];
	rssi[0] = RSSI_LEVELS;			// read RSSI levels
	Trf7970ReadSingle(rssi, 1);
	Card_RSSI = rssi[0];
}
