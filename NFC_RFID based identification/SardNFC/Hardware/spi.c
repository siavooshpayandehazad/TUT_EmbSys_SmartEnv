/*
 * {spi.c}
 *
 * {SPI Interface Functions}
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

#include "spi.h"
#include "trf7970.h"

//===============================================================

u08_t    temp = 0;

extern u08_t direct_mode;

//===============================================================

void SpiStartCondition(void);
void SpiStopCondition(void);
void SpiUsciSet(void);
void SpiUsciDisable(void);

//===============================================================
// NAME: void SpiDirectCommand (u08_t *pbuf)
//
// BRIEF: Is used in SPI mode to transmit a Direct Command to
// reader chip.
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
// 07Dec2010    RP    integrated wait while busy loops
//===============================================================

void
SpiDirectCommand(u08_t *pbuf)
{
  // set Address/Command Word Bit Distribution to command
  *pbuf = (0x80 | *pbuf);           // command
  *pbuf = (0x9f &*pbuf);            // command code
  
  SLAVE_SELECT_LOW;                 // Start SPI Mode

  // USCI_B0 TX buffer ready?
  while (!(IFG2 & UCB0TXIFG));
  
  // Transmit data
  UCB0TXBUF = *pbuf;
  
  // Wait while data is transmitted
  while(UCB0STAT & UCBUSY);
  
  temp = UCB0RXBUF;
  
  SLAVE_SELECT_HIGH;
}

//===============================================================
// NAME: void SpiDirectMode (void)
//
// BRIEF: Is used in SPI mode to start Direct Mode.
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
// 07Dec2010    RP    integrated wait while busy loops
//===============================================================

void
SpiDirectMode(void)
{
  u08_t command [2];
  
  command[0] = CHIP_STATE_CONTROL;
  command[1] = CHIP_STATE_CONTROL;
  SpiReadSingle(&command[1],1);
  command[1] |= 0x60;                        // RF on and BIT 6 in Chip Status Control Register set
  SpiWriteSingle(command, 2);
}

//===============================================================
// NAME: void SpiRawWrite (u08_t *pbuf, u08_t length)
//
// BRIEF: Is used in SPI mode to write direct to the reader chip.
//
// INPUTS:
//    Parameters:
//        u08_t        *pbuf        raw data
//        u08_t        length        number of data bytes
//
// OUTPUTS:
//
// PROCESS:    [1] send raw data to reader chip
//
// CHANGE:
// DATE          WHO    DETAIL
// 24Nov2010    RP    Original Code
// 07Dec2010    RP    integrated wait while busy loops
//===============================================================

void
SpiRawWrite(u08_t *pbuf, u08_t length)
{
  //Start SPI Mode
  SLAVE_SELECT_LOW;
  
  while(length > 0)
  {
    // USCI_B0 TX buffer ready?
    while (!(IFG2 & UCB0TXIFG));
    
    // Transmit data
    UCB0TXBUF = *pbuf;
    
    while(UCB0STAT & UCBUSY);
    temp=UCB0RXBUF;
    
    pbuf++;
    length--;
  }
  
  // Check if SPI state machine is still busy
  while(UCB0STAT & UCBUSY);
  
  // Stop SPI Mode
  SLAVE_SELECT_HIGH;  
  
}

//===============================================================
// NAME: void SpiReadCont (u08_t *pbuf, u08_t length)
//
// BRIEF: Is used in SPI mode to read a specified number of
// reader chip registers from a specified address upwards.
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
// 07Dec2010    RP    integrated wait while busy loops
//===============================================================

void
SpiReadCont(u08_t *pbuf, u08_t length)
{
  uint16 timeout = 0xFF;
	//Start SPI Mode
  SLAVE_SELECT_LOW;
  
  // Address/Command Word Bit Distribution
  *pbuf = (0x60 | *pbuf);                     // address, read, continuous
  *pbuf = (0x7f &*pbuf);                        // register address
  
  // USCI_B0 TX buffer ready?
  while (!(IFG2 & UCB0TXIFG));

  // Transmit data
  UCB0TXBUF = *pbuf;
  
  while(UCB0STAT & UCBUSY);
  temp = UCB0RXBUF;
  
  while(UCB0STAT & UCBUSY);
  
  // Receive initiated by a dummy TX write
  while(length > 0)
  {
      // USCI_B0 TX buffer ready?
      while (!(IFG2 & UCB0TXIFG));
    
      //Receive initiated by a dummy TX write
      UCB0TXBUF = 0xFF;
    
      while(!(IFG2 & UCB0RXIFG) && timeout)
    	  timeout--;
    
      *pbuf = UCB0RXBUF;
      pbuf++;
      length--;
  }
  
  // Wait for SPI state machine to stop being busy
  while(UCB0STAT & UCBUSY);

  // Stop SPI Mode
  SLAVE_SELECT_HIGH;
}

//===============================================================
// NAME: void SpiReadSingle (u08_t *pbuf, u08_t number)
//
// BRIEF: Is used in SPI mode to read specified reader chip
// registers.
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
// 07Dec2010    RP    integrated wait while busy loops
//===============================================================

void
SpiReadSingle(u08_t *pbuf, u08_t number)
{
  // Start SPI Mode
  SLAVE_SELECT_LOW;
  
  while(number > 0)
  {
    // Address/Command Word Bit Distribution
    *pbuf = (0x40 | *pbuf);             // address, read, single
    *pbuf = (0x5f & *pbuf);                // register address
    
    // USCI_B0 TX buffer ready?
    while (!(IFG2 & UCB0TXIFG));
    
    // Transmit data
    UCB0TXBUF = *pbuf;
    
    while(UCB0STAT & UCBUSY);
    temp=UCB0RXBUF;
    
    // Ensure SPI state macihne is not busy
    while(UCB0STAT & UCBUSY);

    // USCI_B0 TX buffer ready?
    while (!(IFG2 & UCB0TXIFG));

    // Receive initiated by a dummy TX write
    UCB0TXBUF = 0x00;
    
    // USCI_B0 RX buffer ready?
    while (!(IFG2 & UCB0RXIFG));
    __no_operation();
    __no_operation();
    *pbuf = UCB0RXBUF;
    pbuf++;
    number--;
    
    // Ensure SPI state machine is not busy
    while(UCB0STAT & UCBUSY);

  }
  // Ensure SPI state machine is not busy
  while(UCB0STAT & UCBUSY);
  // Stop SPI Mode
  SLAVE_SELECT_HIGH;
}

//===============================================================
// Settings for SPI Mode                                        ;
// 02DEC2010    RP    Original Code
//===============================================================

void
SpiSetup(void)
{
  ENABLE_PORT_SET;
  
  IRQ_PIN_SET;
  IRQ_EDGE_SET;                                // rising edge interrupt
  
  SpiUsciSet();                                // Set the USART
  
  LED_ALL_OFF;
  LED_PORT_SET;
}

//===============================================================
// 02DEC2010    RP    Original Code
//===============================================================

void
SpiStartCondition(void)                    //Make the SCLK High
{
  SPI_PORT_FUNC1 &= ~SPI_CLK;
  SPI_DIR |= SPI_CLK;
  SPI_OUT |= SPI_CLK;
}

//******************************************************************************
//
//
//
//******************************************************************************
void SpiStopCondition(void)
{
  //Make the SCLK Low
  SPI_PORT_FUNC1 |= SPI_CLK;
  SPI_OUT &= ~SPI_CLK;
  SPI_DIR |= SPI_CLK;
}

//===============================================================
// NAME: void SpiUsciSet (void)
//
// BRIEF: Is used to set USCI B0 for SPI communication
//
// INPUTS:
//
// OUTPUTS:
//
// PROCESS:    [1] make settings
//
// CHANGE:
// DATE          WHO    DETAIL
// 24Nov2010    RP    Original Code
// 07Dec2010    RP    reduced SPI clock frequency
//===============================================================

void
SpiUsciSet(void)                                    //Uses USCI_B0
{
  UCB0CTL1 |= UCSWRST;                            // Enable SW reset

#ifdef TRF7970A
  UCB0CTL0 |=  UCMSB + UCMST + UCSYNC;    // 3-pin, 8-bit SPI master
#endif
  
  UCB0CTL1 |= UCSSEL_2;                            // SMCLK
  
  UCB0BR0 |= 4;                                    // SPI clock frequency = SMCLK / 4
  UCB0BR1 = 0;
  
  // GPIO Config
  SPI_PORT_FUNC1 |= SPI_CLK + SPI_MISO + SPI_MOSI;
  SPI_PORT_FUNC2 |= SPI_CLK + SPI_MISO + SPI_MOSI;
  
  SLAVE_SELECT_PORT_SET;                            // P3.0 - Slave Select
  SLAVE_SELECT_HIGH;                                // Slave Select - inactive ( high)
  
  UCB0CTL1 &= ~UCSWRST;                            // **Initialize USCI state machine**
}


//===============================================================
// NAME: void SpiUsciDisable (void)
//
// BRIEF: Is used to disable the USCI B0 for SPI communication
//
// INPUTS:
//
// OUTPUTS:
//
// PROCESS:    [1] make settings
//
// CHANGE:
// DATE          WHO    DETAIL
// 24Nov2010    RP    Original Code
// 07Dec2010    RP    reduced SPI clock frequency
//===============================================================

void
SpiUsciDisable(void)                                    //Uses USCI_B0
{
  // GPIO Config
  SPI_PORT_FUNC1 &= ~(SPI_CLK + SPI_MISO + SPI_MOSI);
  
  // TO_DO : MOVE USCI specific init to Trf7970BoosterPack
  P1SEL2 &= ~(SPI_CLK + SPI_MISO + SPI_MOSI);
  
  UCB0CTL1 |= UCSWRST;                            // **Initialize USCI state machine**
}




//===============================================================
// NAME: void SpiWriteCont (u08_t *pbuf, u08_t length)
//
// BRIEF: Is used in SPI mode to write to a specific number of
// reader chip registers from a specific address upwards.
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
// 24Nov2010    RP    Original Code
// 07Dec2010    RP    integrated wait while busy loops
//===============================================================

void
SpiWriteCont(u08_t *pbuf, u08_t length)
{
  SLAVE_SELECT_LOW;                         // Start SPI Mode
  // Address/Command Word Bit Distribution
  *pbuf = (0x20 | *pbuf);                 // address, write, continuous
  *pbuf = (0x3f &*pbuf);                    // register address
  while(length > 0)
  {
    while (!(IFG2 & UCB0TXIFG))            // USCI_B0 TX buffer ready?
    {
    }
    UCB0TXBUF = *pbuf;                    // Previous data to TX, RX
    while(UCB0STAT & UCBUSY)
    {
    }
    temp = UCB0RXBUF;
    
    pbuf++;
    length--;
  }
  while(UCB0STAT & UCBUSY)
  {
  }
  SLAVE_SELECT_HIGH;                         // Stop SPI Mode
}

//===============================================================
// NAME: void SpiWriteSingle (u08_t *pbuf, u08_t length)
//
// BRIEF: Is used in SPI mode to write to a specified reader chip
// registers.
//
// INPUTS:
//    u08_t    *pbuf    addresses of the registers followed by the
//                    contends to write
//    u08_t    length    number of registers * 2
//
// OUTPUTS:
//
// PROCESS:    [1] write to the registers
//
// CHANGE:
// DATE          WHO    DETAIL
// 24Nov2010    RP    Original Code
// 07Dec2010    RP    integrated wait while busy loops
//===============================================================

void
SpiWriteSingle(u08_t *pbuf, u08_t length)
{
  u08_t    i = 0;
  
  SLAVE_SELECT_LOW;                         // Start SPI Mode
  
  while(length > 0)
  {
    // Address/Command Word Bit Distribution
    // address, write, single (fist 3 bits = 0)
    *pbuf = (0x1f &*pbuf);                // register address
    for(i = 0; i < 2; i++)
    {
      while (!(IFG2 & UCB0TXIFG))        // USCI_B0 TX buffer ready?
      {
      }
      UCB0TXBUF = *pbuf;              // Previous data to TX, RX
      
      while(UCB0STAT & UCBUSY);
      
      temp = UCB0RXBUF;
      
      pbuf++;
      length--;
    }
  }
  
  while(UCB0STAT & UCBUSY);
  
  if(direct_mode == 0x00)
  {
    SLAVE_SELECT_HIGH;                     // Stop SPI Mode
  }
}
