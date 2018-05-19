/*=======================================================================================*
 * @file    memory.c
 * @author  Marcin
 * @version 1.0
 * @date    17 maj 2018
 * @brief   
 *======================================================================================*/

/**
 * @addtogroup memory.c Description
 * @{
 * @brief  
 */

/*======================================================================================*/
/*                       ####### PREPROCESSOR DIRECTIVES #######                        */
/*======================================================================================*/
/*-------------------------------- INCLUDE DIRECTIVES ----------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <stm32f30x_conf.h>
#include "delay.h"
#include "debugkom.h"
#include "timer.h"
#include "memory.h"
/*----------------------------- LOCAL OBJECT-LIKE MACROS -------------------------------*/

/*---------------------------- LOCAL FUNCTION-LIKE MACROS ------------------------------*/

/*======================================================================================*/
/*                      ####### LOCAL TYPE DECLARATIONS #######                         */
/*======================================================================================*/
/*---------------------------- ALL TYPE DECLARATIONS -----------------------------------*/

/*-------------------------------- OTHER TYPEDEFS --------------------------------------*/

/*------------------------------------- ENUMS ------------------------------------------*/

/*------------------------------- STRUCT AND UNIONS ------------------------------------*/

/*======================================================================================*/
/*                    ####### LOCAL FUNCTIONS PROTOTYPES #######                        */
/*======================================================================================*/

/*======================================================================================*/
/*                         ####### OBJECT DEFINITIONS #######                           */
/*======================================================================================*/
/*--------------------------------- EXPORTED OBJECTS -----------------------------------*/

/*---------------------------------- LOCAL OBJECTS -------------------------------------*/

DWORD AccSize;				/* Work register for fs command */
WORD AccFiles, AccDirs;
FILINFO Finfo;


FATFS FatFs;				/* File system object for each logical drive */
FIL File[4];				/* File objects */
DIR Dir;					/* Directory object */

char *fileNameFlow="przeplyw.txt",*fileNameMoist="wilgo.txt",*fileNameInfo="info.txt",*fileTest="test.txt";


/*======================================================================================*/
/*                  ####### EXPORTED FUNCTIONS DEFINITIONS #######                      */
/*======================================================================================*/
/*


/*======================================================================================*/
/*                   ####### LOCAL FUNCTIONS DEFINITIONS #######                        */
/*======================================================================================*/


void Measure_SaveTimeStamp(sTimeS* sTime, measParamS *measBuff)
{
	measBuff->hms = sTime->hms;
}


/*
 * Format zapisu:
 * 					<time>,<fi>,<Ac>,<Ar>,<C>,<Tz>,<Ti>,<Hi>,<To>,<Ho>
 */
uint8_t Measure_SD_Write(sTimeS* sTime, dataTypeE datatype)
{
	static uint8_t infoHourWrite=0,diskFullFlag=0,firstRunFlag=1;
	uint16_t writeCnt=0;
	uint8_t respSD;
	uint32_t charNum,btwrt;
	LED_Blue(FAST);
	if (GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_2)){	// Card is not in socket
	//	PC_Send("NO CARD!!! \n\r");
		LED_Red(ONEBLINKS);
		LED_Blue(SLOW);
		return 0;
	}
	if(diskFullFlag){
		LED_Blue(MEDIUM);
		LED_Red(MEDIUM);
	//	PC_Send("DISKFULL FLAG SET -.-\n\r");
		//return 0;
	}
	if(firstRunFlag){
		firstRunFlag=0;
		respSD=f_mount(&FatFs, "", 1);
	//	PC_Send("SD FirstRun!\n\r");
		PC_SendResponse(respSD);
		if(respSD!=0)return 0;
	}
	if(f_open(&File[3],fileTest,FA_READ|FA_WRITE|FA_OPEN_ALWAYS)){
		//PC_Send("Test Open Failed\n\r");
		respSD=f_mount(&FatFs, "", 1);
		PC_SendResponse(respSD);
		if(respSD!=0){
			//PC_Send("Mount after Open Failed!\n\r");
			return 0;
		}
	}
	else respSD=f_close(&File[3]);
	PC_SendResponse(respSD);
	//PC_Send("SD Init OK\n\r");
	if((infoHourWrite != sTime->hours) || (datatype == INFO)){
		infoHourWrite = sTime->hours;
		VBATMeasure(&measStartup);
		TempInternalMeasure(&measStartup);

		respSD=f_open(&File[0],fileNameInfo,FA_READ|FA_WRITE|FA_OPEN_ALWAYS);
		PC_SendResponse(respSD);
		respSD = f_lseek(&File[0], f_size(&File[0]));
		PC_SendResponse(respSD);

		charNum=sprintf(cBuf,"%.2d-%.2d-%2d %.2d:%.2d:%.2d Vb: %d, Ti: %i\n",sTime->date,sTime->month,sTime->year,sTime->hours,sTime->minutes,sTime->seconds,measStartup.vBat,measStartup.tempProc);

		respSD=f_write(&File[0],cBuf,charNum,&btwrt);
		if(charNum != btwrt) diskFullFlag=1;
		//sprintf(cBuf,"SD Info Bytes Written: %d",btwrt);
		//PC_Send(cBuf);
		respSD=f_close(&File[0]);
		PC_SendResponse(respSD);
	}
	if(datatype == FLOW){
		respSD=f_open(&File[1],fileNameFlow,FA_READ|FA_WRITE|FA_OPEN_ALWAYS);
		PC_SendResponse(respSD);
		respSD = f_lseek(&File[1], f_size(&File[1]));
		PC_SendResponse(respSD);

		for(writeCnt=1;writeCnt<=measureBuffCnt;writeCnt++){
			charNum=sprintf(cBuf,"%.2d-%.2d-%2d %.2d:%.2d:%.2d,%u,%u,%u,%u,%i,%i,%u,%i,%u\n",sTime->date,sTime->month,sTime->year,measBuff[writeCnt].hms/3600,(measBuff[writeCnt].hms%3600)/60,measBuff[writeCnt].hms%60,measBuff[writeCnt].faza,measBuff[writeCnt].ampliC,measBuff[writeCnt].ampliR,measBuff[writeCnt].cvdC,measBuff[writeCnt].tempZboza,measBuff[writeCnt].tempIn,measBuff[writeCnt].humIn,measBuff[writeCnt].tempOut,measBuff[writeCnt].humOut);
			respSD=f_write(&File[1],cBuf,charNum,&btwrt);
			if(charNum != btwrt){
				diskFullFlag=1;
				break;
			}
		}
		/*sprintf(cBuf,"SD Flow Bytes Written: %d",btwrt);
		PC_Send(cBuf);*/
		respSD=f_close(&File[1]);
		PC_SendResponse(respSD);
	}
	else if(datatype == MOISTURE){
		respSD=f_open(&File[2],fileNameMoist,FA_READ|FA_WRITE|FA_OPEN_ALWAYS);
		PC_SendResponse(respSD);
		respSD = f_lseek(&File[2], f_size(&File[2]));
		PC_SendResponse(respSD);

		for(writeCnt=1;writeCnt<=measureBuffCnt;writeCnt++){
			charNum=sprintf(cBuf,"%.2d-%.2d-%2d %.2d:%.2d:%.2d,%u,%u,%u,%u,%i,%i,%u,%i,%u\n",sTime->date,sTime->month,sTime->year,measBuff[writeCnt].hms/3600,(measBuff[writeCnt].hms%3600)/60,measBuff[writeCnt].hms%60,measBuff[writeCnt].faza,measBuff[writeCnt].ampliC,measBuff[writeCnt].ampliR,measBuff[writeCnt].cvdC,measBuff[writeCnt].tempZboza,measBuff[writeCnt].tempIn,measBuff[writeCnt].humIn,measBuff[writeCnt].tempOut,measBuff[writeCnt].humOut);
			respSD=f_write(&File[2],cBuf,charNum,&btwrt);
			if(charNum != btwrt){
				diskFullFlag=1;
				break;
			}
		}
		/*sprintf(cBuf,"SD Moist Bytes Written: %d",btwrt);
		PC_Send(cBuf);*/
		respSD=f_close(&File[2]);
		PC_SendResponse(respSD);
	}
	LED_Blue(OFF);
	return 1;
}

void PC_SendResponse(uint8_t resp)
{
#if SD_UART_SEND_RESP
		const char *str =
			"OK\0" "DISK_ERR\0" "INT_ERR\0" "NOT_READY\0" "NO_FILE\0" "NO_PATH\0"
			"INVALID_NAME\0" "DENIED\0" "EXIST\0" "INVALID_OBJECT\0" "WRITE_PROTECTED\0"
			"INVALID_DRIVE\0" "NOT_ENABLED\0" "NO_FILE_SYSTEM\0" "MKFS_ABORTED\0" "TIMEOUT\0"
			"LOCKED\0" "NOT_ENOUGH_CORE\0" "TOO_MANY_OPEN_FILES\0";
		FRESULT i;

		for (i = 0; i != resp && *str; i++) {
			while (*str++) ;
		}
		sprintf(cBuf,"Code: %d  %s\n\r",resp,str);
		PC_Send(cBuf);

#endif
}

void Measure_SDDetectConfig(void)
{
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);

	GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_SetBits(GPIOA,GPIO_Pin_2);
}

/**
 * @} end of group memory.c
 */
