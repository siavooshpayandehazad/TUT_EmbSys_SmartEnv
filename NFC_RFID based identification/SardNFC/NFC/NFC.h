/*
 * NFC.h
 */

#ifndef NFC_NFC_H_
#define NFC_NFC_H_

#include <msp430.h>                         // processor specific header
#include <stdlib.h>                         // general purpose standard library
#include <stdio.h>                          // standard input/output header
#include "iso14443a.h"
#include "iso14443b.h"
#include "iso15693.h"
#include "mcu.h"
#include "Trf7970.h"
#include "types.h"
#include "uart.h"

void NFC_Init(void);
int NFC_Read(void);
void Print_Card(void);

extern u08_t Card_UID[14];
extern u08_t Card_RSSI;


#endif /* NFC_NFC_H_ */
