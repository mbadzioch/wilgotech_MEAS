/*
 * bt.h
 *
 *  Created on: 2 gru 2015
 *      Author: Marcin
 */

#ifndef INCLUDE_BT_H_
#define INCLUDE_BT_H_

#include <stdio.h>
#include <stdlib.h>
#include <stm32f30x.h>
#include "delay.h"


char cBuf[128];
void PC_Send(volatile char *s);
void PC_UartInit(void);


#endif /* INCLUDE_BT_H_ */
