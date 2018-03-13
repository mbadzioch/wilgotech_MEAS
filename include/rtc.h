/*
 * rtc.h
 *
 *  Created on: 25 kwi 2016
 *      Author: Marcin
 */

#ifndef RTC_H_
#define RTC_H_

#include <stdio.h>
#include <stdlib.h>
#include <stm32f30x_conf.h>
#include <stm32f30x.h>
#include "main.h"
#include "integer.h"


#define RTC_SYNC_PREDIV 0xFF;
#define RTC_ASYNC_PREDIV 0x7F


#define RTC_DEF_HOUR	19
#define RTC_DEF_MINUTE	49
#define RTC_DEF_SECOND	0
#define RTC_DEF_YEAR	16
#define RTC_DEF_MONTH	5
#define RTC_DEF_WEEKDAY	5
#define RTC_DEF_DATE	6


void RTC_Config(sTimeS* sTime);
void RTC_SetDateTime(sTimeS* sTime);
void RTC_GetDateTime(sTimeS* sTime);
DWORD get_fattime (void);
#endif /* RTC_H_ */
