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
#include "ff.h"
#include "measure.h"
#include "led.h"
/*----------------------------- LOCAL OBJECT-LIKE MACROS -------------------------------*/
#define DEBUG_SD 0
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
static void Memory_SD_PinConfig(void);
static void Memory_DebugResponse(uint8_t resp);
static memory_resp_T Memory_SaveRecBin(void);
static memory_resp_T Memory_SaveRecTxt(void);
static memory_resp_T Memory_SaveCfgBin(void);
static memory_resp_T Memory_SaveCfgTxt(void);
static memory_resp_T Memory_GetCfg(void);
static memory_resp_T Memory_GetRecord(void);
static memory_resp_T Memory_OpenFile(const char* filename);
/*======================================================================================*/
/*                         ####### OBJECT DEFINITIONS #######                           */
/*======================================================================================*/
/*--------------------------------- EXPORTED OBJECTS -----------------------------------*/
extern measure_record_T measure_current_record;
extern measure_set_T measure_settings;
extern rtc_time_T rtc_current_time;

//TODO: DEBUG

extern debugkom_record_T debug_record;
/*---------------------------------- LOCAL OBJECTS -------------------------------------*/

DWORD AccSize;				/* Work register for fs command */
WORD AccFiles, AccDirs;
FILINFO Finfo;


FATFS FatFs;				/* File system object for each logical drive */
FIL File[4];				/* File objects */
DIR Dir;					/* Directory object */

char *filename_rec_txt="dane.txt",*filename_rec_bin="hist",*filename_cfg_txt="conf.txt",*filename_cfg_bin="cfg";

struct __attribute__((__packed__)){
	uint16_t timestamp;
	uint16_t moist;
	uint16_t flow;
	int16_t  temp_z;
	int16_t  temp_in;
	int16_t  temp_out;
	uint8_t hum_in;
	uint8_t hum_out;
}memory_rec_bin_data;

struct __attribute__((__packed__)){
	uint8_t idle_flag;
	uint8_t flow_rec_num;
	uint8_t moist_rec_num;

}memory_cfg_bin_data;

uint8_t respSD=0,memory_error_flag=0;
uint32_t charNum=0;
UINT btwrt=0;

uint32_t memory_rechist_cnt=0;

char memory_buf_txt[64];

/*======================================================================================*/
/*                  ####### EXPORTED FUNCTIONS DEFINITIONS #######                      */
/*======================================================================================*/


void Memory_Init(void)
{
	Memory_SD_PinConfig();

	//Card not in socket
	if (GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_2) == 1){
		LED_Blue(LED_SLOW);
		memory_error_flag = 1;
	}
	else{
		respSD=f_mount(&FatFs, "", 1);
		Memory_DebugResponse(respSD);

		if(respSD != FR_OK){
			memory_error_flag = 1;
			LED_Blue(LED_SLOW);
		}
	}
}
memory_resp_T Memory_SaveRec(void)
{
	memory_resp_T resp;

	if(memory_error_flag == 1){
		LED_Blue(LED_SLOW);
		return MEM_ERROR;
	}
	LED_Blue(LED_ON);

	resp=Memory_SaveRecBin();
	resp=Memory_SaveRecTxt();
	LED_Blue(LED_OFF);

	return resp;
}
memory_resp_T Memory_SaveCfg(void)
{
	memory_resp_T resp;

	if(memory_error_flag == 1){
		LED_Blue(LED_SLOW);
		return MEM_ERROR;
	}
	LED_Blue(LED_ON);

	resp=Memory_SaveCfgBin();
	resp=Memory_SaveCfgTxt();
	LED_Blue(LED_OFF);
	return resp;
}
memory_resp_T Memory_ReadCfg(void)
{
	memory_resp_T resp;

	if(memory_error_flag == 1){
		LED_Blue(LED_SLOW);
		return MEM_ERROR;
	}
	LED_Blue(LED_ON);
	resp=Memory_GetCfg();
	LED_Blue(LED_OFF);
	return resp;
}
memory_resp_T Memory_ReadRecHistory(void)
{
	memory_resp_T resp;

	if(memory_error_flag == 1){
		LED_Blue(LED_SLOW);
		return MEM_ERROR;
	}
	LED_Blue(LED_ON);

	resp = Memory_GetRecord();

	LED_Blue(LED_OFF);
	if(resp == MEM_RECNEXT){
		memory_rechist_cnt++;
		return MEM_RECNEXT;
	}
	else if(resp == MEM_RECEND){
		memory_rechist_cnt=0;
		return MEM_RECEND;
	}

	return MEM_ERROR;
}



/*======================================================================================*/
/*                   ####### LOCAL FUNCTIONS DEFINITIONS #######                        */
/*======================================================================================*/

static memory_resp_T Memory_SaveRecBin(void)
{
	memory_rec_bin_data.timestamp = measure_current_record.timestamp;
	memory_rec_bin_data.moist = measure_current_record.moist;
	memory_rec_bin_data.flow = measure_current_record.flow;
	memory_rec_bin_data.temp_z = measure_current_record.temp_z;
	memory_rec_bin_data.temp_in = measure_current_record.temp_in;
	memory_rec_bin_data.temp_out = measure_current_record.temp_out;
	memory_rec_bin_data.hum_in = measure_current_record.hum_in;
	memory_rec_bin_data.hum_out = measure_current_record.hum_out;

	Memory_OpenFile(filename_rec_bin);

	respSD = f_lseek(&File[0], f_size(&File[0]));
	Memory_DebugResponse(respSD);

	respSD=f_write(&File[0],&memory_rec_bin_data,sizeof(memory_rec_bin_data),&btwrt);
	Memory_DebugResponse(respSD);

	respSD=f_close(&File[0]);
	Memory_DebugResponse(respSD);

	if(sizeof(memory_rec_bin_data) != btwrt){
		memory_error_flag=1;
		return MEM_ERROR;
	}
	return MEM_OK;
}
static memory_resp_T Memory_SaveRecTxt(void)
{
	Memory_OpenFile(filename_rec_txt);

	respSD = f_lseek(&File[0], f_size(&File[0]));
	Memory_DebugResponse(respSD);

	charNum=sprintf(memory_buf_txt,"%.2d-%.2d-%2d %.2d:%.2d:%.2d,%u,%u,%u,%i,%i,%i,%u,%u,%u,%u,%u\n",
			rtc_current_time.date,rtc_current_time.month,rtc_current_time.year,
			rtc_current_time.hours,rtc_current_time.minutes,rtc_current_time.seconds,
			measure_current_record.timestamp,
			measure_current_record.moist,measure_current_record.flow,
			measure_current_record.temp_z,measure_current_record.temp_in,measure_current_record.temp_out,
			measure_current_record.hum_in,measure_current_record.hum_out,
			measure_current_record.phase,measure_current_record.amp_c,measure_current_record.amp_r
			);

	respSD=f_write(&File[0],&memory_buf_txt,charNum,&btwrt);
	Memory_DebugResponse(respSD);

	respSD=f_close(&File[0]);
	Memory_DebugResponse(respSD);

	if(charNum != btwrt){
		memory_error_flag=1;
		return MEM_ERROR;
	}
	return MEM_OK;
}
static memory_resp_T Memory_SaveCfgBin(void)
{
	memory_cfg_bin_data.idle_flag = measure_settings.idle_flag;
	memory_cfg_bin_data.flow_rec_num = measure_settings.flow_rec_num;
	memory_cfg_bin_data.moist_rec_num = measure_settings.moist_rec_num;

	respSD=f_open(&File[0],filename_cfg_bin,FA_READ|FA_WRITE|FA_OPEN_ALWAYS);
	Memory_DebugResponse(respSD);
	if(respSD != FR_OK){
		memory_error_flag = 1;
		LED_Blue(LED_SLOW);
		return MEM_ERROR;
	}
	respSD=f_write(&File[0],&memory_cfg_bin_data,sizeof(memory_cfg_bin_data),&btwrt);
	Memory_DebugResponse(respSD);

	respSD=f_close(&File[0]);
	Memory_DebugResponse(respSD);

	if(sizeof(memory_cfg_bin_data) != btwrt){
		memory_error_flag=1;
		return MEM_ERROR;
	}
	return MEM_OK;
}
static memory_resp_T Memory_SaveCfgTxt(void)
{
	respSD=f_open(&File[0],filename_cfg_txt,FA_READ|FA_WRITE|FA_OPEN_ALWAYS);
	Memory_DebugResponse(respSD);
	if(respSD != FR_OK){
		memory_error_flag = 1;
		LED_Blue(LED_SLOW);
		return MEM_ERROR;
	}

	respSD = f_lseek(&File[0], f_size(&File[0]));
	Memory_DebugResponse(respSD);

	charNum=sprintf(memory_buf_txt,"%.2d-%.2d-%2d %.2d:%.2d:%.2d,%u,%u,%u\n",
			rtc_current_time.date,rtc_current_time.month,rtc_current_time.year,
			rtc_current_time.hours,rtc_current_time.minutes,rtc_current_time.seconds,
			measure_settings.idle_flag,measure_settings.flow_rec_num,measure_settings.moist_rec_num
			);

	respSD=f_write(&File[0],memory_buf_txt,charNum,&btwrt);
	Memory_DebugResponse(respSD);

	respSD=f_close(&File[0]);
	Memory_DebugResponse(respSD);

	if(charNum != btwrt){
		memory_error_flag=1;
		return MEM_ERROR;
	}
	return MEM_OK;
}

static memory_resp_T Memory_GetCfg(void)
{
	return MEM_OK;
}

static memory_resp_T Memory_GetRecord(void)
{
	if(memory_rechist_cnt == 0){
		Memory_OpenFile(filename_rec_bin);
	}

	if((memory_rechist_cnt * sizeof(memory_rec_bin_data)) >= f_size(&File[0])){
		respSD=f_close(&File[0]);
		Memory_DebugResponse(respSD);
		return MEM_RECEND;
	}

	respSD = f_lseek(&File[0], memory_rechist_cnt * sizeof(memory_rec_bin_data));
	Memory_DebugResponse(respSD);

	respSD=f_read(&File[0],&memory_rec_bin_data,sizeof(memory_rec_bin_data),&btwrt);
	Memory_DebugResponse(respSD);

	debug_record.timestamp = memory_rec_bin_data.timestamp;
	debug_record.moist = memory_rec_bin_data.moist;
	debug_record.flow = memory_rec_bin_data.flow;
	debug_record.temp_z = memory_rec_bin_data.temp_z;
	debug_record.temp_in = memory_rec_bin_data.temp_in;
	debug_record.temp_out = memory_rec_bin_data.temp_out;
	debug_record.hum_in = memory_rec_bin_data.hum_in;
	debug_record.hum_out = memory_rec_bin_data.hum_out;

	if(respSD != FR_OK){
		memory_error_flag = 1;
		LED_Blue(LED_SLOW);
		return MEM_ERROR;
	}

	return MEM_RECNEXT;
}
static memory_resp_T Memory_OpenFile(const char* filename)
{
	respSD=f_open(&File[0],filename,FA_READ|FA_WRITE|FA_OPEN_ALWAYS);
	Memory_DebugResponse(respSD);

	if(respSD != FR_OK){

		respSD=f_mount(&FatFs, "", 1);
		Memory_DebugResponse(respSD);
		if(respSD != FR_OK){
			memory_error_flag = 1;
			LED_Blue(LED_SLOW);
			return MEM_ERROR;
		}
		respSD=f_open(&File[0],filename,FA_READ|FA_WRITE|FA_OPEN_ALWAYS);
		Memory_DebugResponse(respSD);
	}
	return respSD;
}

static void Memory_DebugResponse(uint8_t resp)
{
#if DEBUG_SD
	const char *str =
		"OK\0" "DISK_ERR\0" "INT_ERR\0" "NOT_READY\0" "NO_FILE\0" "NO_PATH\0"
		"INVALID_NAME\0" "DENIED\0" "EXIST\0" "INVALID_OBJECT\0" "WRITE_PROTECTED\0"
		"INVALID_DRIVE\0" "NOT_ENABLED\0" "NO_FILE_SYSTEM\0" "MKFS_ABORTED\0" "TIMEOUT\0"
		"LOCKED\0" "NOT_ENOUGH_CORE\0" "TOO_MANY_OPEN_FILES\0";
	FRESULT i;

	for (i = 0; i != resp && *str; i++) {
		while (*str++) ;
	}
	sprintf(memory_buf_txt,"Code: %d  %s\n\r",resp,str);
	PC_Debug(memory_buf_txt);
#endif
}

static void Memory_SD_PinConfig(void)
{
	GPIO_InitTypeDef		   GPIO_InitStructure;
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

/*
 * Format zapisu:
 * 					<time>,<fi>,<Ac>,<Ar>,<C>,<Tz>,<Ti>,<Hi>,<To>,<Ho>
 */
//uint8_t Measure_SD_Write(sTimeS* sTime, dataTypeE datatype)
//{
//	static uint8_t infoHourWrite=0,diskFullFlag=0,firstRunFlag=1;
//	uint16_t writeCnt=0;
//	uint8_t respSD;
//	uint32_t charNum,btwrt;
//	LED_Blue(FAST);
//	if (GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_2)){	// Card is not in socket
//	//	PC_Send("NO CARD!!! \n\r");
//		LED_Red(ONEBLINKS);
//		LED_Blue(SLOW);
//		return 0;
//	}
//	if(diskFullFlag){
//		LED_Blue(MEDIUM);
//		LED_Red(MEDIUM);
//	//	PC_Send("DISKFULL FLAG SET -.-\n\r");
//		//return 0;
//	}
//	if(firstRunFlag){
//		firstRunFlag=0;
//		respSD=f_mount(&FatFs, "", 1);
//	//	PC_Send("SD FirstRun!\n\r");
//		PC_SendResponse(respSD);
//		if(respSD!=0)return 0;
//	}
//	if(f_open(&File[3],fileTest,FA_READ|FA_WRITE|FA_OPEN_ALWAYS)){
//		//PC_Send("Test Open Failed\n\r");
//		respSD=f_mount(&FatFs, "", 1);
//		PC_SendResponse(respSD);
//		if(respSD!=0){
//			//PC_Send("Mount after Open Failed!\n\r");
//			return 0;
//		}
//	}
//	else respSD=f_close(&File[3]);
//	PC_SendResponse(respSD);
//	//PC_Send("SD Init OK\n\r");
//	if((infoHourWrite != sTime->hours) || (datatype == INFO)){
//		infoHourWrite = sTime->hours;
//		VBATMeasure(&measStartup);
//		TempInternalMeasure(&measStartup);
//
//		respSD=f_open(&File[0],fileNameInfo,FA_READ|FA_WRITE|FA_OPEN_ALWAYS);
//		PC_SendResponse(respSD);
//		respSD = f_lseek(&File[0], f_size(&File[0]));
//		PC_SendResponse(respSD);
//
//		charNum=sprintf(cBuf,"%.2d-%.2d-%2d %.2d:%.2d:%.2d Vb: %d, Ti: %i\n",sTime->date,sTime->month,sTime->year,sTime->hours,sTime->minutes,sTime->seconds,measStartup.vBat,measStartup.tempProc);
//
//		respSD=f_write(&File[0],cBuf,charNum,&btwrt);
//		if(charNum != btwrt) diskFullFlag=1;
//		//sprintf(cBuf,"SD Info Bytes Written: %d",btwrt);
//		//PC_Send(cBuf);
//		respSD=f_close(&File[0]);
//		PC_SendResponse(respSD);
//	}
//	if(datatype == FLOW){
//		respSD=f_open(&File[1],fileNameFlow,FA_READ|FA_WRITE|FA_OPEN_ALWAYS);
//		PC_SendResponse(respSD);
//		respSD = f_lseek(&File[1], f_size(&File[1]));
//		PC_SendResponse(respSD);
//
//		for(writeCnt=1;writeCnt<=measureBuffCnt;writeCnt++){
//			charNum=sprintf(cBuf,"%.2d-%.2d-%2d %.2d:%.2d:%.2d,%u,%u,%u,%u,%i,%i,%u,%i,%u\n",sTime->date,sTime->month,sTime->year,measBuff[writeCnt].hms/3600,(measBuff[writeCnt].hms%3600)/60,measBuff[writeCnt].hms%60,measBuff[writeCnt].faza,measBuff[writeCnt].ampliC,measBuff[writeCnt].ampliR,measBuff[writeCnt].cvdC,measBuff[writeCnt].tempZboza,measBuff[writeCnt].tempIn,measBuff[writeCnt].humIn,measBuff[writeCnt].tempOut,measBuff[writeCnt].humOut);
//			respSD=f_write(&File[1],cBuf,charNum,&btwrt);
//			if(charNum != btwrt){
//				diskFullFlag=1;
//				break;
//			}
//		}
//		/*sprintf(cBuf,"SD Flow Bytes Written: %d",btwrt);
//		PC_Send(cBuf);*/
//		respSD=f_close(&File[1]);
//		PC_SendResponse(respSD);
//	}
//	else if(datatype == MOISTURE){
//		respSD=f_open(&File[2],fileNameMoist,FA_READ|FA_WRITE|FA_OPEN_ALWAYS);
//		PC_SendResponse(respSD);
//		respSD = f_lseek(&File[2], f_size(&File[2]));
//		PC_SendResponse(respSD);
//
//		for(writeCnt=1;writeCnt<=measureBuffCnt;writeCnt++){
//			charNum=sprintf(cBuf,"%.2d-%.2d-%2d %.2d:%.2d:%.2d,%u,%u,%u,%u,%i,%i,%u,%i,%u\n",sTime->date,sTime->month,sTime->year,measBuff[writeCnt].hms/3600,(measBuff[writeCnt].hms%3600)/60,measBuff[writeCnt].hms%60,measBuff[writeCnt].faza,measBuff[writeCnt].ampliC,measBuff[writeCnt].ampliR,measBuff[writeCnt].cvdC,measBuff[writeCnt].tempZboza,measBuff[writeCnt].tempIn,measBuff[writeCnt].humIn,measBuff[writeCnt].tempOut,measBuff[writeCnt].humOut);
//			respSD=f_write(&File[2],cBuf,charNum,&btwrt);
//			if(charNum != btwrt){
//				diskFullFlag=1;
//				break;
//			}
//		}
//		/*sprintf(cBuf,"SD Moist Bytes Written: %d",btwrt);
//		PC_Send(cBuf);*/
//		respSD=f_close(&File[2]);
//		PC_SendResponse(respSD);
//	}
//	LED_Blue(OFF);
//	return 1;
//}

/**
 * @} end of group memory.c
 */
