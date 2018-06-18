/*
 * rtc.h
 *
 *  Created on: 25 kwi 2016
 *      Author: Marcin
 */

#ifndef RTC_H_
#define RTC_H_

#include <stdio.h>
#include "integer.h"

typedef struct{
	uint8_t seconds;     /*!< Seconds parameter, from 00 to 59 */
	uint8_t minutes;     /*!< Minutes parameter, from 00 to 59 */
	uint8_t hours;       /*!< Hours parameter, 24Hour mode, 00 to 23 */
	uint8_t date;        /*!< Date in a month, 1 to 31 */
	uint8_t month;       /*!< Month in a year, 1 to 12 */
	uint8_t year;        /*!< Year parameter, 00 to 99, 00 is 2000 and 99 is 2099 */
	uint8_t weekday;     // Day in week ( 0 = Monday)
	uint16_t timestamp;	 // Timestamp format: Weekday 0 - 6 | Hour 0 - 24 | Minute 0 - 59
}rtc_time_T;


#define RTC_SYNC_PREDIV 0xFF;
#define RTC_ASYNC_PREDIV 0x7F


#define RTC_DEF_HOUR	10
#define RTC_DEF_MINUTE	38
#define RTC_DEF_SECOND	0
#define RTC_DEF_YEAR	18
#define RTC_DEF_MONTH	6
#define RTC_DEF_WEEKDAY	5 // 0 - Monday
#define RTC_DEF_DATE	9


void RTC_Initialize();
void RTC_SetDateTime();
void RTC_GetDateTime();
void RTC_GetCurrentSecond();
DWORD get_fattime (void);


#endif /* RTC_H_ */
