/*
 * rtc.c
 *
 *  Created on: 25 kwi 2016
 *      Author: Marcin
 */


#include "rtc.h"
#include <stdio.h>
#include <stdlib.h>
#include <stm32f30x_conf.h>
#include <stm32f30x.h>
#include "integer.h"

RTC_InitTypeDef RTC_InitStructure;
RTC_TimeTypeDef RTC_TimeStructure;
RTC_DateTypeDef RTC_DateStructure;

rtc_time_T rtc_current_time;


void RTC_Config(void);
void RTC_DefaultTime();

void RTC_Initialize()
{
	if (RTC_ReadBackupRegister(RTC_BKP_DR0) != 0x32F2)
	{
		/* RTC configuration  */
		RTC_Config();
		/* Configure the RTC data register and RTC prescaler */
		RTC_InitStructure.RTC_AsynchPrediv = RTC_ASYNC_PREDIV;
		RTC_InitStructure.RTC_SynchPrediv = RTC_SYNC_PREDIV;
		RTC_InitStructure.RTC_HourFormat = RTC_HourFormat_24;

		RTC_DefaultTime();
		RTC_SetDateTime();

		/* Indicator for the RTC configuration */
		RTC_WriteBackupRegister(RTC_BKP_DR0, 0x32F2);
	}
	else
	{
		/* Enable the PWR clock */
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);

		/* Allow access to RTC */
		PWR_BackupAccessCmd(ENABLE);

		/* Wait for RTC APB registers synchronisation */
		RTC_WaitForSynchro();
	}
}
void RTC_Config(void)
{
	/* Enable the PWR clock */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);

	/* Allow access to RTC */
	PWR_BackupAccessCmd(ENABLE);

	RCC_BackupResetCmd(ENABLE);
	RCC_BackupResetCmd(DISABLE);

	//LSE
	RCC_LSEConfig(RCC_LSE_ON);
	RCC_LSEDriveConfig(RCC_LSEDrive_Low);
	while(RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET);
	RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);

	/* Enable the RTC Clock */
	RCC_RTCCLKCmd(ENABLE);

	/* Wait for RTC APB registers synchronisation */
	RTC_WaitForSynchro();
}
void RTC_SetDateTime()
{
	RTC_TimeTypeDef time;
	RTC_DateTypeDef date;

	time.RTC_H12=RTC_H12_AM;
	time.RTC_Hours=rtc_current_time.hours;
	time.RTC_Minutes=rtc_current_time.minutes;
	time.RTC_Seconds=rtc_current_time.seconds;
	date.RTC_Year=rtc_current_time.year;
	date.RTC_Month=rtc_current_time.month;
	date.RTC_WeekDay=rtc_current_time.weekday+1; // DEF for ST is 1 = Monday, we need start from 0
	date.RTC_Date=rtc_current_time.date;

	RTC_SetTime(RTC_Format_BIN,&time);
	RTC_SetDate(RTC_Format_BIN,&date);
}
void RTC_GetDateTime()
{
	RTC_TimeTypeDef time;
	RTC_DateTypeDef date;

	RTC_GetTime(RTC_Format_BIN,&time);
	RTC_GetDate(RTC_Format_BIN,&date);

	rtc_current_time.hours=time.RTC_Hours;
	rtc_current_time.minutes=time.RTC_Minutes;
	rtc_current_time.seconds=time.RTC_Seconds;
	rtc_current_time.year=date.RTC_Year;
	rtc_current_time.month=date.RTC_Month;
	rtc_current_time.weekday=date.RTC_WeekDay-1;// DEF for ST is 1 = Monday, we need start from 0
	rtc_current_time.date=date.RTC_Date;

	rtc_current_time.timestamp = (uint16_t)rtc_current_time.weekday*10000;
	rtc_current_time.timestamp += (uint16_t)rtc_current_time.hours*100;
	rtc_current_time.timestamp += (uint16_t)rtc_current_time.minutes;
}
void RTC_GetCurrentSecond()
{
	RTC_TimeTypeDef time;

	RTC_GetTime(RTC_Format_BIN,&time);
	rtc_current_time.seconds=time.RTC_Seconds;
}
void RTC_DefaultTime()
{
	rtc_current_time.hours=RTC_DEF_HOUR;
	rtc_current_time.minutes=RTC_DEF_MINUTE;
	rtc_current_time.seconds=RTC_DEF_SECOND;
	rtc_current_time.year=RTC_DEF_YEAR;
	rtc_current_time.month=RTC_DEF_MONTH;
	rtc_current_time.weekday=RTC_DEF_WEEKDAY;
	rtc_current_time.date=RTC_DEF_DATE;
}

/*---------------------------------------------------------*/
/* User provided RTC function for FatFs module             */
/*---------------------------------------------------------*/
/* This is a real time clock service to be called back     */
/* from FatFs module.                                      */

DWORD get_fattime (void)
{
	RTC_TimeTypeDef time;
	RTC_DateTypeDef date;

	RTC_GetTime(RTC_Format_BIN,&time);
	RTC_GetDate(RTC_Format_BIN,&date);

	return	  ((DWORD)(date.RTC_Year +20) << 25)
			| ((DWORD)date.RTC_Month << 21)
			| ((DWORD)date.RTC_Date << 16)
			| ((DWORD)time.RTC_Hours << 11)
			| ((DWORD)time.RTC_Minutes << 5)
			| ((DWORD)time.RTC_Seconds >> 1);

}

