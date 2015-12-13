/*
 * {trf7970.c}
 *
 * {TRF7970 Communication functions}
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

/****************************************************************
* FILENAME: Trf7970.c
*
* BRIEF: Contains functions to initialize and execute
* communication the Trf7970 via the selected interface.
*
* Copyright (C) 2010 Texas Instruments, Inc.
*
* AUTHOR(S): Reiser Peter        DATE: 02 DEC 2010
*
* EDITED BY:
* *
*
****************************************************************/

#include "trf7970.h"

//===============================================================

u08_t    command[2];
u08_t    direct_mode = 0;
extern u08_t    buf[300];
extern u08_t    i_reg;
#ifdef ENABLE14443A
	extern u08_t    coll_poss;
#endif
extern u08_t    irq_flag;
extern u08_t    rx_error_flag;
extern s08_t    rxtx_state;
extern u08_t    stand_alone_flag;

//===============================================================

void Trf7970ISR(u08_t *irq_status);

//===============================================================
//                                                              ;
//===============================================================

void
Trf7970CommunicationSetup(void)
{
  if(SPIMODE)                                // SPI mode given by jumper setting
  {
    SpiSetup();
  }

}

//===============================================================
// NAME: void Trf7970DirectCommand (u08_t *pbuf)
//
// BRIEF: Is used to transmit a Direct Command to Trf7970.
//
// INPUTS:
//    Parameters:
//        u08_t        *pbuf        Direct Command
//
// OUTPUTS:
//
// PROCESS:    [1] transmit Direct Command
//
// CHANGE:
// DATE          WHO    DETAIL
// 24Nov2010    RP    Original Code
//===============================================================

void
Trf7970DirectCommand(u08_t *pbuf)
{
  if(SPIMODE)                                // SPI mode given by jumper setting
  {
    SpiDirectCommand(pbuf);
  }

}

//===============================================================
// NAME: void Trf7970DirectMode (void)
//
// BRIEF: Is used to start Direct Mode.
//
// INPUTS:
//
// OUTPUTS:
//
// PROCESS:    [1] start Direct Mode
//
// NOTE: No stop condition
//
// CHANGE:
// DATE          WHO    DETAIL
// 24Nov2010    RP    Original Code
//===============================================================

void
Trf7970DirectMode(void)
{
  direct_mode = 1;

  if(SPIMODE)                                // SPI mode given by jumper setting
  {
    SpiDirectMode();
  }
}

//===============================================================
// 02DEC2010    RP    Original Code
//===============================================================

void
Trf7970DisableSlotCounter(void)
{
  buf[40] = IRQ_MASK;
  buf[41] = IRQ_MASK;                // next slot counter
  Trf7970ReadSingle(&buf[41], 1);
  buf[41] &= 0xFE;                // clear BIT0 in register 0x01
  Trf7970WriteSingle(&buf[40], 2);
}

//===============================================================
// 02DEC2010    RP    Original Code
//===============================================================

void
Trf7970EnableSlotCounter(void)
{
  buf[40] = IRQ_MASK;
  buf[41] = IRQ_MASK;                // next slot counter
  Trf7970ReadSingle (&buf[41], 1);
  buf[41] |= BIT0;                // set BIT0 in register 0x01
  Trf7970WriteSingle(&buf[40], 2);
}

//===============================================================
// 02DEC2010    RP    Original Code
//===============================================================

void
Trf7970InitialSettings(void)
{
  command[0] = SOFT_INIT;
  Trf7970DirectCommand(command);

  command[0] = IDLE;
  Trf7970DirectCommand(command);


  #ifdef TRF7970A
    command[0] = MODULATOR_CONTROL;
    command[1] = 0x01;                    // ASK 100%, no SYS_CLK output
    Trf7970WriteSingle(command, 2);
#endif
}

//===============================================================
// The Interrupt Service Routine determines how the IRQ should  ;
// be handled. The Trf7970 IRQ status register is read to       ;
// determine the cause of the IRQ. Conditions are checked and   ;
// appropriate actions taken.                                   ;
//===============================================================

void
Trf7970ISR(u08_t *irq_status)
{
	if(*irq_status == 0xA0)			// BIT5 and BIT7
	{								// TX active and only 3 bytes left in FIFO
		i_reg = 0x00;
	}

	else if(*irq_status == BIT7)
	{								// TX complete
		i_reg = 0x00;
		Trf7970Reset();				// reset the FIFO after TX
	}

	else if((*irq_status & BIT1) == BIT1)
	{								// collision error
		i_reg = 0x02;				// RX complete

#ifdef ENABLE14443A
		coll_poss = COLLISION_POSITION;
		Trf7970ReadSingle(&coll_poss, 1);
		buf[rxtx_state] = FIFO;					// write the recieved bytes to the correct place of the
													// buffer;
		if (coll_poss >= 0x20)
		{
			Trf7970ReadCont(&buf[rxtx_state], 5);
		}
#endif

		Trf7970StopDecoders();						// reset the FIFO after TX

		Trf7970Reset();

		Trf7970ResetIrqStatus();

		IRQ_CLR;
	}
	else if(*irq_status == BIT6)
	{	// RX flag means that EOF has been recieved
		// and the number of unread bytes is in FIFOstatus regiter
		if(rx_error_flag == 0x02)
		{
			i_reg = 0x02;
			return;
		}

		*irq_status = FIFO_STATUS;
		Trf7970ReadSingle(irq_status, 1);					// determine the number of bytes left in FIFO

		*irq_status = 0x7F & *irq_status;
		buf[rxtx_state] = FIFO;						// write the recieved bytes to the correct place of the buffer

		Trf7970ReadCont(&buf[rxtx_state], *irq_status);
		rxtx_state = rxtx_state + *irq_status;

		Trf7970Reset();					// reset the FIFO after last byte has been read out

		i_reg = 0xFF;					// signal to the recieve funnction that this are the last bytes
	}
	else if(*irq_status == 0x60)
	{									// RX active and 9 bytes allready in FIFO
		i_reg = 0x01;
		buf[rxtx_state] = FIFO;
		Trf7970ReadCont(&buf[rxtx_state], 9);	// read 9 bytes from FIFO
		rxtx_state = rxtx_state + 9;

		if((IRQ_PORT & IRQ_PIN) == IRQ_PIN)			// if IRQ pin high
		{
			Trf7970ReadIrqStatus(irq_status);

			IRQ_CLR;

			if(*irq_status == 0x40)							// end of recieve
			{
				*irq_status = FIFO_STATUS;
				Trf7970ReadSingle(irq_status, 1);					// determine the number of bytes left in FIFO

				*irq_status = 0x7F & *irq_status;
				buf[rxtx_state] = FIFO;						// write the recieved bytes to the correct place of the buffer


				Trf7970ReadCont(&buf[rxtx_state], *irq_status);
				rxtx_state = rxtx_state + *irq_status;

				i_reg = 0xFF;			// signal to the recieve funnction that this are the last bytes
				Trf7970Reset();		// reset the FIFO after last byte has been read out
			}
			else if(*irq_status == 0x50)	// end of recieve and error
			{
				i_reg = 0x02;
			}
		}
		else
		{
			Trf7970ReadIrqStatus(irq_status);

			if(irq_status[0] == 0x00)
			{
				i_reg = 0xFF;
			}
		}
	}
	else if((*irq_status & BIT4) == BIT4)				// CRC error
	{
		if((*irq_status & BIT5) == BIT5)
		{
			i_reg = 0x01;	// RX active
			rx_error_flag = 0x02;
		}
		if((*irq_status & BIT6) == BIT6)			// 4 Bit receive
		{
			buf[200] = FIFO;						// write the recieved bytes to the correct place of the buffer

			Trf7970ReadCont(&buf[200], 1);

			Trf7970Reset();

			i_reg = 0x02;	// end of RX
			rx_error_flag = 0x02;
		}
		else
		{
			i_reg = 0x02;	// end of RX
		}
	}
	else if((*irq_status & BIT2) == BIT2)	// byte framing error
	{
		if((*irq_status & BIT5) == BIT5)
		{
			i_reg = 0x01;	// RX active
			rx_error_flag = 0x02;
		}
		else
			i_reg = 0x02;	// end of RX
	}
	else if((*irq_status == BIT0))
	{						// No response interrupt
		i_reg = 0x00;
	}
	else
	{						// Interrupt register not properly set
		i_reg = 0x02;

		Trf7970StopDecoders();	// reset the FIFO after TX

		Trf7970Reset();

		Trf7970ResetIrqStatus();

		IRQ_CLR;
	}
}                            // Interrupt Service Routine

//===============================================================
// 02DEC2010    RP    Original Code
//===============================================================

#pragma vector = PORT2_VECTOR
__interrupt void
Trf7970PortB(void)                            // interrupt handler
{
  u08_t irq_status[4], iso_control;

  irq_flag = 0x02;

  STOP_COUNTER;                            // stop timer mode

  do
  {
    IRQ_CLR;                            // PORT2 interrupt flag clear

	iso_control = ISO_CONTROL;
	Trf7970ReadSingle(&iso_control, 1);
    Trf7970ReadIrqStatus(irq_status);

	if((iso_control & BIT5) != BIT5)			// RFID mode
	{
		Trf7970ISR(irq_status);
	}

  } while((IRQ_PORT & IRQ_PIN) == IRQ_PIN);
  __bic_SR_register_on_exit(LPM0_bits);
}

//===============================================================
// NAME: void Trf7970RawWrite (u08_t *pbuf, u08_t length)
//
// BRIEF: Is used to write direct to the Trf7970.
//
// INPUTS:
//    Parameters:
//        u08_t        *pbuf        raw data
//        u08_t        length        number of data bytes
//
// OUTPUTS:
//
// PROCESS:    [1] send raw data to Trf7970
//
// CHANGE:
// DATE          WHO    DETAIL
// 24Nov2010    RP    Original Code
//===============================================================

void
Trf7970RawWrite(u08_t *pbuf, u08_t length)
{
  if(SPIMODE)                                // SPI mode given by jumper setting
  {
    SpiRawWrite(pbuf, length);
  }
}

//===============================================================
// 02DEC2010    RP    Original Code
//===============================================================

void
Trf7970ReConfig(void)
{
  if(SPIMODE)
  {
    SpiUsciSet();                   // Set the USART
  }
}

//===============================================================
// NAME: void Trf7970ReadCont (u08_t *pbuf, u08_t length)
//
// BRIEF: Is used to read a specified number of Trf7970 registers
// from a specified address upwards.
//
// INPUTS:
//    Parameters:
//        u08_t        *pbuf        address of first register
//        u08_t        length        number of registers
//
// OUTPUTS:
//
// PROCESS:    [1] read registers
//            [2] write contents to *pbuf
//
// CHANGE:
// DATE          WHO    DETAIL
// 24Nov2010    RP    Original Code
//===============================================================

void
Trf7970ReadCont(u08_t *pbuf, u08_t length)
{
  if(SPIMODE)                                // SPI mode given by jumper setting
  {
    SpiReadCont(pbuf, length);
  }
  else                                    // parallel mode
  {

  }
}

//===============================================================
// 02DEC2010    RP    Original Code
//===============================================================

void
Trf7970ReadIrqStatus(u08_t *pbuf)
{
  *pbuf = IRQ_STATUS;
  *(pbuf + 1) = IRQ_MASK;
  Trf7970ReadCont(pbuf, 2);           // read second reg. as dummy read

}

//===============================================================
// NAME: void Trf7970ReadSingle (u08_t *pbuf, u08_t number)
//
// BRIEF: Is used to read specified Trf7970 registers.
//
// INPUTS:
//    Parameters:
//        u08_t        *pbuf        addresses of the registers
//        u08_t        number        number of the registers
//
// OUTPUTS:
//
// PROCESS:    [1] read registers
//            [2] write contents to *pbuf
//
// CHANGE:
// DATE          WHO    DETAIL
// 24Nov2010    RP    Original Code
//===============================================================

void
Trf7970ReadSingle(u08_t *pbuf, u08_t number)
{
  if(SPIMODE)                                // SPI mode given by jumper setting
  {
    SpiReadSingle(pbuf, number);
  }

}

//===============================================================
// resets FIFO                                                  ;
// 02DEC2010    RP    Original Code
//===============================================================

void
Trf7970Reset(void)

{
  command[0] = RESET;
  Trf7970DirectCommand(command);

}

//===============================================================
// resets IRQ Status                                            ;
// 02DEC2010    RP    Original Code
//===============================================================

void
Trf7970ResetIrqStatus(void)
{
  u08_t irq_status[4];
  irq_status[0] = IRQ_STATUS;
  irq_status[1] = IRQ_MASK;

  if(SPIMODE)                                // SPI mode given by jumper setting
  {
    Trf7970ReadCont(irq_status, 2);        // read second reg. as dummy read
  }

}

//===============================================================
//                                                              ;
// 02DEC2010    RP    Original Code
//===============================================================

void
Trf7970RunDecoders(void)
{
  command[0] = RUN_DECODERS;
  Trf7970DirectCommand(command);
}

//===============================================================
// 02DEC2010    RP    Original Code
//===============================================================

void
Trf7970StopDecoders(void)
{
  command[0] = STOP_DECODERS;
  Trf7970DirectCommand(command);
}

//===============================================================
// 02DEC2010    RP    Original Code
//===============================================================

void
Trf7970TransmitNextSlot(void)
{
  command[0] = TRANSMIT_NEXT_SLOT;
  Trf7970DirectCommand(command);
}

//===============================================================
// 02DEC2010    RP    Original Code
//===============================================================

void
Trf7970TurnRfOff(void)
{
  command[0] = CHIP_STATE_CONTROL;
  command[1] = CHIP_STATE_CONTROL;
  Trf7970ReadSingle(&command[1], 1);
  command[1] &= 0x1F;
  Trf7970WriteSingle(command, 2);
}

//===============================================================
// 02DEC2010    RP    Original Code
//===============================================================

void
Trf7970TurnRfOn(void)
{
  u08_t write[4];
  write[0] = CHIP_STATE_CONTROL;
  write[1] = 0x20;                    // 3.3VDC, Full power out
  Trf7970WriteSingle(write, 2);

}

//===============================================================
// NAME: void SpiWriteCont (u08_t *pbuf, u08_t length)
//
// BRIEF: Is used to write to a specific number of reader chip
// registers from a specific address upwards.
//
// INPUTS:
//    u08_t    *pbuf    address of first register followed by the
//                    contents to write
//    u08_t    length    number of registers + 1
//
// OUTPUTS:
//
// PROCESS:    [1] write to the registers
//
// CHANGE:
// DATE          WHO    DETAIL
//===============================================================

void
Trf7970WriteCont(u08_t *pbuf, u08_t length)
{
  if(SPIMODE)                                // SPI mode given by jumper setting
  {
    SpiWriteCont(pbuf, length);
  }
  else                                    // parallel mode
  {

  }
}

//===============================================================
// 02DEC2010    RP    Original Code
//===============================================================

void
Trf7970WriteIsoControl(u08_t iso_control)
{
  u08_t write[16];

  write[0] = ISO_CONTROL;
  write[1] = iso_control;
  write[1] &= ~BIT5;
  Trf7970WriteSingle(write, 2);

  iso_control &= 0x1F;
  write[0] = IRQ_MASK;
  write[1] = 0x3F;
  Trf7970WriteSingle(write, 2);

#ifdef ENABLE15693                // this standard can be disabled in ISO15693.h
  if (iso_control == 0x02)
  {
    write[0] = 0x20;                  //Continuous Write, starting with register 0x00
    write[1] = 0x20;                  //Value for Chip Status Control Register 0x00, 0x20 = +3.3VDC, full power, etc.
    write[2] = 0x02;                  //Value for ISO Control Register 0x01, 0x02 = high tag data rate, etc.
    write[3] = 0x00;
    write[4] = 0x00;
    write[5] = 0xC1;
    write[6] = 0xBB;
    write[7] = 0x00;
    write[8] = 0x30;
    write[9] = 0x1F;
    write[10] = 0x01;
    write[11] = 0x40;
    write[12] = 0x87;

    Trf7970WriteCont(write, 13);      //writes registers 0x00:0x0B
  }
#endif

#ifdef ENABLE14443A            // this standard can be disabled in ISO14443A.h
  if (iso_control == 0x08)
  {
    write[0] = 0x20;
    write[1] = 0x20;    //full power out, 3.3VDC
    write[2] = 0x88;
    write[3] = 0x00;
    write[4] = 0x00;
    write[5] = 0xC1;
    write[6] = 0xBB;
    write[7] = 0x20;
    write[8] = 0x0E;
    write[9] = 0x07;
    write[10] = 0x21;
    write[11] = 0x20;
    write[12] = 0x87;

    Trf7970WriteCont(write, 13);
  }
#endif

#ifdef ENABLE14443B            // this standard can be disabled in ISO14443B.h
  if (iso_control == 0x0C)
  {
    write[0] = 0x20;
    write[1] = 0x20;  //full power out, 3.3VDC
    write[2] = 0x0C;
    write[3] = 0x00;
    write[4] = 0x00;
    write[5] = 0xC1;
    write[6] = 0xBB;
    write[7] = 0x00;
    write[8] = 0x0D;
    write[9] = 0x07;
    write[10] = 0x03;
    write[11] = 0x00;
    write[12] = 0x87;

    Trf7970WriteCont(write, 13);
  }
#endif


}

//===============================================================
// NAME: void Trf7970WriteSingle (u08_t *pbuf, u08_t length)
//
// BRIEF: Is used to write to specified reader chip registers.
//
// INPUTS:
//    u08_t    *pbuf    addresses of the registers followed by the
//                    contents to write
//    u08_t    length    number of registers * 2
//
// OUTPUTS:
//
// PROCESS:    [1] write to the registers
//
// CHANGE:
// DATE          WHO    DETAIL
// 24Nov2010    RP    Original Code
//===============================================================

void
Trf7970WriteSingle(u08_t *pbuf, u08_t length)
{
  if(SPIMODE)                                // SPI mode given by jumper setting
  {
    SpiWriteSingle(pbuf, length);
  }
  else                                    // parallel mode
  {

  }
}
