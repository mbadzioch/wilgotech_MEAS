/*
 * rtc.c
 *
 *  Created on: 25 kwi 2016
 *      Author: Marcin
 */


#include "rtc.h"

RTC_InitTypeDef RTC_InitStructure;
RTC_TimeTypeDef RTC_TimeStructure;
RTC_DateTypeDef RTC_DateStructure;

void RTC_Initialization(void);
void RTC_DefaultTime(sTimeS* sTime);

void RTC_Config(sTimeS* sTime)
{
	if (RTC_ReadBackupRegister(RTC_BKP_DR0) != 0x32F2)
	{
		/* RTC configuration  */
		RTC_Initialization();
		/* Configure the RTC data register and RTC prescaler */
		RTC_InitStructure.RTC_AsynchPrediv = RTC_ASYNC_PREDIV;
		RTC_InitStructure.RTC_SynchPrediv = RTC_SYNC_PREDIV;
		RTC_InitStructure.RTC_HourFormat = RTC_HourFormat_24;

		RTC_DefaultTime(sTime);
		RTC_SetDateTime(sTime);

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
void RTC_Initialization(void)
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
void RTC_SetDateTime(sTimeS* sTime)
{
	RTC_TimeTypeDef time;
	RTC_DateTypeDef date;

	time.RTC_H12=RTC_H12_AM;
	time.RTC_Hours=sTime->hours;
	time.RTC_Minutes=sTime->minutes;
	time.RTC_Seconds=sTime->seconds;
	date.RTC_Year=sTime->year;
	date.RTC_Month=sTime->month;
	//date.RTC_WeekDay=sTime->weekday;
	date.RTC_Date=sTime->date;

	RTC_SetTime(RTC_Format_BIN,&time);
	RTC_SetDate(RTC_Format_BIN,&date);
}
void RTC_GetDateTime(sTimeS* sTime)
{
	RTC_TimeTypeDef time;
	RTC_DateTypeDef date;

	RTC_GetTime(RTC_Format_BIN,&time);
	RTC_GetDate(RTC_Format_BIN,&date);

	sTime->hours=time.RTC_Hours;
	sTime->minutes=time.RTC_Minutes;
	sTime->seconds=time.RTC_Seconds;
	sTime->year=date.RTC_Year;
	sTime->month=date.RTC_Month;
	//sTime->weekday=date.RTC_WeekDay;
	sTime->date=date.RTC_Date;
	sTime->hms = time.RTC_Hours*3600+time.RTC_Minutes*60+time.RTC_Seconds;

}
void RTC_DefaultTime(sTimeS* sTime)
{
	sTime->hours=RTC_DEF_HOUR;
	sTime->minutes=RTC_DEF_MINUTE;
	sTime->seconds=RTC_DEF_SECOND;
	sTime->year=RTC_DEF_YEAR;
	sTime->month=RTC_DEF_MONTH;
	//sTime->weekday=RTC_DEF_WEEKDAY;
	sTime->date=RTC_DEF_DATE;
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

