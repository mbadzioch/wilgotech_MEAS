/*
 * pomiar.c
 *
 *  Created on: 26 kwi 2016
 *      Author: Marcin
 */


#include "pomiar.h"
#include "thsense.h"
#include "fazomierz.h"
#include "amplituda.h"
#include "cvd.h"
#include "ff.h"
#include "diskio.h"
#include "uart.h"
#include "timer.h"
GPIO_InitTypeDef		   GPIO_InitStructure;


DWORD AccSize;				/* Work register for fs command */
WORD AccFiles, AccDirs;
FILINFO Finfo;


FATFS FatFs;				/* File system object for each logical drive */
FIL File[4];				/* File objects */
DIR Dir;					/* Directory object */

char *fileNameFlow="przeplyw.txt",*fileNameMoist="wilgo.txt",*fileNameInfo="info.txt",*fileTest="test.txt";

uint16_t measureBuffCnt=0;
uint8_t flapAlarmDeactivateFlag=0;
void PC_SendResponse(uint8_t resp);

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
void Measure_SwPinConfig(void)
{
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOC, ENABLE);

	GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	GPIO_ResetBits(GPIOC,GPIO_Pin_6);
	GPIO_ResetBits(GPIOC,GPIO_Pin_7);
}
void Measure_FlapCtrlConfig(void)
{
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOC, ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);

	GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_2 | GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_0 | GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
}
/*
 *  1 - alarm deactivate
 *  0 - countdown deactivation time
 */
void FLAP_AlarmDeactivate(uint8_t state)
{
	static uint8_t downCnt=0;
	if(state!=0){
		GPIO_SetBits(GPIOA,GPIO_Pin_9); // Deaktywacja syreny
		flapAlarmDeactivateFlag=1;
		downCnt=45;
	}
	else if(--downCnt == 0){
		GPIO_ResetBits(GPIOA,GPIO_Pin_9); // Aktywacja syreny
		flapAlarmDeactivateFlag=0;
	}
}
/*
 * Returns:
 * 			0 - Closing / Opening
 * 			1 - Close / Open success
 * 			2 - Opening Failed
 * 			3 - Closing Failed
 */
uint8_t Measure_FlapCtrl(flapCmdE flapCmd,uint8_t sec)
{
	static uint8_t startSec=0,resetFlag=1;

	switch(flapState){
		case OPENED:
			GPIO_ResetBits(GPIOC,GPIO_Pin_2|GPIO_Pin_3);
			if(flapCmd == CLOSE){
				GPIO_SetBits(GPIOC,GPIO_Pin_2|GPIO_Pin_3);
				startSec=sec;
				flapState = CLOSING;
				FLAP_AlarmDeactivate(1);
				return 0;
			}
			if(resetFlag!=0){
				GPIO_SetBits(GPIOC,GPIO_Pin_2);
				Delay_ms(200);
				if(GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_0)==0){
					GPIO_ResetBits(GPIOC,GPIO_Pin_2|GPIO_Pin_3);
					resetFlag=0;
				}
				else{
					flapState = OPENING;
					startSec=sec;
					return 0;
				}
			}
			return 1;
			break;
		case CLOSED:
			GPIO_ResetBits(GPIOC,GPIO_Pin_2|GPIO_Pin_3);
			if(flapCmd == OPEN){
				GPIO_SetBits(GPIOC,GPIO_Pin_2);
				startSec=sec;
				flapState = OPENING;
				return 0;
			}
			return 1;
			break;
		case OPENING:
			if(GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_0)==0){
				flapState = OPENED;
				resetFlag=0;
			}
			else{
				Delay_ms(9);
				if(GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_0)==0){
					flapState = OPENED;
					resetFlag=0;
				}
			}
			if(abs(sec-startSec)>FLAP_OPEN_TIMEOUT){
				GPIO_ResetBits(GPIOC,GPIO_Pin_2|GPIO_Pin_3);
				if(resetFlag!=0){
					resetFlag=0;
					flapState = OPENED;
					return 1;
				}
				flapState=CLOSED;
				return 2;
			}
			return 0;
		case CLOSING:
			if(GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_1)==0){
				flapState = CLOSED;
			}
			else{
				Delay_ms(9);
				if(GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_1)==0){
					flapState = CLOSED;
				}
			}
			if(abs(sec-startSec)>FLAP_OPEN_TIMEOUT){
				GPIO_ResetBits(GPIOC,GPIO_Pin_2|GPIO_Pin_3);
				flapState=OPENED;
				return 3;
			}
			return 0;
		default:
			break;
	}
	return 0;
}
void Measure_SwitchCVD(void)
{
	GPIO_ResetBits(GPIOC,GPIO_Pin_6);
	GPIO_ResetBits(GPIOC,GPIO_Pin_7);

	GPIO_SetBits(GPIOC,GPIO_Pin_7);
	Delay_ms(5);
	GPIO_ResetBits(GPIOC,GPIO_Pin_7);
}
void Measure_SwitchNormal(void)
{
	GPIO_ResetBits(GPIOC,GPIO_Pin_6);
	GPIO_ResetBits(GPIOC,GPIO_Pin_7);

	GPIO_SetBits(GPIOC,GPIO_Pin_6);
	Delay_ms(5);
	GPIO_ResetBits(GPIOC,GPIO_Pin_6);
}
void Measure_Config(void)
{
	Sinus_Generator_Config();
	Fazomierz_Config();
	AmpliMeter_Config();
	LED_Config();
	TempHum_Config(1);
	Measure_SwPinConfig();
	Measure_FlapCtrlConfig();
	Measure_SDDetectConfig();
	measureState=INIT;
	flapState=OPENED;
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

void Measure_SaveTimeStamp(sTimeS* sTime, measParamS *measBuff)
{
	measBuff->hms = sTime->hms;
}

void Measure_Sequence(sTimeS* sTime)
{
	static uint32_t startSDHms=0,startMeasHms=0;
	uint8_t flapResp=0;
	if(flapAlarmDeactivateFlag){
		FLAP_AlarmDeactivate(0);
	}
	switch(measureState){
		case INIT:
			LED_Green(SLOW);
			Measure_SD_Write(sTime,INFO);
			measureState = FLAPOPEN;
			break;
		case MEASFLOW:
			LED_Green(ON);
			measureBuffCnt++;
			Measure_SwitchNormal();
			Delay_ms(5);
			Fazomierz_Measure(&measBuff[measureBuffCnt]);
			AmpliMeasureR(&measBuff[measureBuffCnt]);
			AmpliMeasureC(&measBuff[measureBuffCnt]);
			Measure_SwitchCVD();
			Delay_ms(5);
			CVD_MeasureC(&measBuff[measureBuffCnt]);
			ThermistorMeasure(&measBuff[measureBuffCnt]);
			TempHum_Measure(&measBuff[measureBuffCnt]);
			Measure_SaveTimeStamp(sTime,&measBuff[measureBuffCnt]);
			if((sTime->hms - startSDHms) >= 60*SD_WRITE_INTERVAL){
				Measure_SD_Write(sTime,FLOW);
				startSDHms=sTime->hms;
				measureBuffCnt=0;
			}
			if((sTime->hms - startMeasHms) >= 60*FLOW_MEAS_TIME){
				measureState = FLAPCLOSE;
			}
			LED_Green(OFF);
			break;
		case MEASMOIST:
			LED_Green(ON);
			measureBuffCnt++;
			Measure_SwitchNormal();
			Delay_ms(5);
			Fazomierz_Measure(&measBuff[measureBuffCnt]);
			AmpliMeasureR(&measBuff[measureBuffCnt]);
			AmpliMeasureC(&measBuff[measureBuffCnt]);
			Delay_ms(5);
			ThermistorMeasure(&measBuff[measureBuffCnt]);
			TempHum_Measure(&measBuff[measureBuffCnt]);
			Measure_SaveTimeStamp(sTime,&measBuff[measureBuffCnt]);
			if((sTime->hms - startSDHms) >= 60*SD_WRITE_INTERVAL){
				Measure_SD_Write(sTime,MOISTURE);
				startSDHms=sTime->hms;
				measureBuffCnt=0;
			}
			if((sTime->hms - startMeasHms) >= 60*MOISTURE_MEAS_TIME){
				measureState = FLAPOPEN;
			}
			LED_Green(OFF);
			break;
		case FLAPCLOSE:
			flapResp=Measure_FlapCtrl(CLOSE,sTime->seconds);
			switch(flapResp){
				case 0:
					LED_Green(FAST);
					break;
				case 1:
					LED_Green(ONEBLINKS);
					LED_Red(OFF);
					measureState = MEASMOIST;
					startMeasHms=sTime->hms;
					startSDHms=sTime->hms;

					break;
				case 3:
					measureState = MEASFLOW;
					startMeasHms=sTime->hms;
					startSDHms=sTime->hms;
					LED_Red(MEDIUM);
					break;
				default:
					LED_Red(FAST);
					break;
			}
			break;
		case FLAPOPEN:
			flapResp=Measure_FlapCtrl(OPEN,sTime->seconds);
			switch(flapResp){
				case 0:
					LED_Green(FAST);
					break;
				case 1:
					LED_Green(ONEBLINKS);
					LED_Red(OFF);
					measureState = MEASFLOW;
					startMeasHms=sTime->hms;
					startSDHms=sTime->hms;

					break;
				case 2:
					measureState = MEASMOIST;
					startMeasHms=sTime->hms;
					startSDHms=sTime->hms;

					Measure_SD_Write(sTime,INFO);
					LED_Red(MEDIUM);
					break;
				default:
					LED_Red(FAST);
					break;
			}
			break;
		default:
			break;
	}

#if MEAS_PRINT_UART
	sprintf(cBuf,"fi: %5d, Ac %4d, Ar %4d, C: %5d, Tz: %i, Ti: %i, Hi: %d, To: %d, Ho: %d\n\r",measBuff[measureBuffCnt].faza,measBuff[measureBuffCnt].ampliC,measBuff[measureBuffCnt].ampliR,measBuff[measureBuffCnt].cvdC,measBuff[measureBuffCnt].tempZboza,measBuff[measureBuffCnt].tempIn,measBuff[measureBuffCnt].humIn,measBuff[measureBuffCnt].tempOut,measBuff[measureBuffCnt].humOut);
	PC_Send(cBuf);
	switch(measureState){
	case 0:
		PC_Send("STATE INIT\n\r");
		break;
	case 1:
		PC_Send("STATE FLOW\n\r");
		break;
	case 2:
		PC_Send("STATE MOISTURE\n\r");
		break;
	case 3:
		PC_Send("STATE FLAPCLOSE\n\r");
		break;
	case 4:
		PC_Send("STATE FLAPOPEN\n\r");
		break;
	default:
		break;
	}
	switch(flapState){
	case 0:
		PC_Send("FLAP OPENED\n\r");
		break;
	case 1:
		PC_Send("FLAP OPENING\n\r");
		break;
	case 2:
		PC_Send("FLAP CLOSED\n\r");
		break;
	case 3:
		PC_Send("FLAP CLOSING\n\r");
		break;
	default:
		break;
	}
#endif

}
/*
 * Test modes:
 * 		0 - Measure and print to UART
 * 		1 - Measure only Normal
 * 		2 - Measure only CVD
 * 		3 - Close Flap
 * 		4 - Open Flap
 * 		5 - IDLE (Measure VBat and print to UART)
 */
void Measure_CallibSequence(sTimeS* sTime,uint8_t mode)
{
	static uint8_t stop=0,firstRun=1;
	uint8_t writeCnt=0;

	/*if(firstRun){
		firstRun=0;
		PC_Send("Date Time,fi,Ac,Ar,C,Tz,Ti,Hi,To,Ho\n\r");
	}*/
	switch(mode){
	case 0:
		LED_Green(ON);
		Measure_SwitchNormal();
		Delay_ms(5);
		Fazomierz_Measure(&measBuff[writeCnt]);
		AmpliMeasureR(&measBuff[writeCnt]);
		AmpliMeasureC(&measBuff[writeCnt]);
		Measure_SwitchCVD();
		Delay_ms(5);
		CVD_MeasureC(&measBuff[writeCnt]);
		ThermistorMeasure(&measBuff[writeCnt]);
		TempHum_Measure(&measBuff[writeCnt]);
		Measure_SaveTimeStamp(sTime,&measBuff[writeCnt]);
		sprintf(cBuf,"%.2d-%.2d-%2d %.2d:%.2d:%.2d,%u,%u,%u,%u,%i\n",sTime->date,sTime->month,sTime->year,measBuff[writeCnt].hms/3600,(measBuff[writeCnt].hms%3600)/60,measBuff[writeCnt].hms%60,measBuff[writeCnt].faza,measBuff[writeCnt].ampliC,measBuff[writeCnt].ampliR,measBuff[writeCnt].cvdC,measBuff[writeCnt].tempZboza);
		PC_Send(cBuf);
		LED_Green(OFF);
		break;
	case 1:
		LED_Green(ON);
		Measure_SwitchNormal();
		Delay_ms(5);
		Fazomierz_Measure(&measBuff[writeCnt]);
		AmpliMeasureR(&measBuff[writeCnt]);
		AmpliMeasureC(&measBuff[writeCnt]);
		Measure_SwitchCVD();
		Delay_ms(5);
		CVD_MeasureC(&measBuff[writeCnt]);
		ThermistorMeasure(&measBuff[writeCnt]);
		TempHum_Measure(&measBuff[writeCnt]);
		Measure_SaveTimeStamp(sTime,&measBuff[writeCnt]);
		sprintf(cBuf,"%.2d-%.2d-%2d %.2d:%.2d:%.2d,fi: %u,Ac: %u,Ar: %u,C: %u,Tz: %i,Ti: %i,Hi: %u,To: %i,Ho: %u\n\r",sTime->date,sTime->month,sTime->year,measBuff[writeCnt].hms/3600,(measBuff[writeCnt].hms%3600)/60,measBuff[writeCnt].hms%60,measBuff[writeCnt].faza,measBuff[writeCnt].ampliC,measBuff[writeCnt].ampliR,measBuff[writeCnt].cvdC,measBuff[writeCnt].tempZboza,measBuff[writeCnt].tempIn,measBuff[writeCnt].humIn,measBuff[writeCnt].tempOut,measBuff[writeCnt].humOut);
		PC_Send(cBuf);
		LED_Green(OFF);
		break;
	case 3:
		Measure_SD_Test(sTime);
		break;
	default:
		break;
	}
}

void TIM2_IRQHandler()
{
	if(TIM_GetITStatus(TIM2,TIM_IT_Update) != RESET){
		disk_timerproc();
		Timer_Handler();
		TIM_ClearITPendingBit(TIM2,TIM_IT_Update);
	}
}

// OLD

//uint8_t Measure_SD_Test(sTimeS* sTime)
//{
//	static uint8_t infoHourWrite=0,diskFullFlag=0,firstRunFlag=1;
//	uint16_t writeCnt=0;
//	uint8_t respSD;
//	uint32_t charNum,btwrt;
//	//char buff[100];
//	LED_Blue(FAST);
//	if (GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_2)){	// Card is not in socket
//		PC_Send("NO CARD!!! \n\r");
//		LED_Red(ONEBLINKS);
//		LED_Blue(SLOW);
//		return 0;
//	}
//	if(diskFullFlag){
//		LED_Blue(MEDIUM);
//		LED_Red(MEDIUM);
//		PC_Send("DISKFULL FLAG SET -.-\n\r");
//		//return 0;
//	}
//	if(firstRunFlag){
//		firstRunFlag=0;
//		respSD=f_mount(&FatFs, "", 1);
//		PC_Send("SD FirstRun!\n\r");
//		PC_SendResponse(respSD);
//		if(respSD!=0){
//			PC_Send("FirstRun failed!\n\r");
//			return 0;
//		}
//	}
//	if(f_open(&File[3],fileTest,FA_READ|FA_WRITE|FA_OPEN_ALWAYS)){
//		PC_Send("Test Open Failed\n\r");
//		respSD=f_mount(&FatFs, "", 1);
//		PC_SendResponse(respSD);
//		if(respSD!=0){
//			PC_Send("Mount after Open Failed!\n\r");
//			return 0;
//		}
//		PC_Send("Mount after Open Success!\n\r");
//	}
//	else respSD=f_close(&File[3]);
//	PC_SendResponse(respSD);
//	PC_Send("SD Init OK\n\r");
//
//
//	respSD=f_open(&File[1],fileNameFlow,FA_READ|FA_WRITE|FA_OPEN_ALWAYS);
//	PC_SendResponse(respSD);
//	respSD = f_lseek(&File[1], f_size(&File[1]));
//	PC_SendResponse(respSD);
//	measureBuffCnt=0;
//	PC_Send("Mierze\n\r");
//	for(uint16_t i=0;i<120;i++){
//		measureBuffCnt++;
//		measBuff[measureBuffCnt].ampliC = 1230;
//		measBuff[measureBuffCnt].ampliR= 1230;
//		measBuff[measureBuffCnt].cvdC= 1230;
//		measBuff[measureBuffCnt].faza= 1230;
//		measBuff[measureBuffCnt].humIn= 1230;
//		measBuff[measureBuffCnt].humOut= 1230;
//		measBuff[measureBuffCnt].hms= 60;
//		measBuff[measureBuffCnt].tempIn= 1230;
//		measBuff[measureBuffCnt].tempOut= 1230;
//		measBuff[measureBuffCnt].tempZboza= 1230;
//	}
//	PC_Send("Zmierzylem\n\r");
//	for(writeCnt=1;writeCnt<=measureBuffCnt;writeCnt++){
//		charNum=sprintf(cBuf,"%.2d-%.2d-%2d %.2d:%.2d:%.2d,%u,%u,%u,%u,%i,%i,%u,%i,%u\n",sTime->date,sTime->month,sTime->year,sTime->hours,sTime->minutes,sTime->seconds,measBuff[writeCnt].faza,measBuff[writeCnt].ampliC,measBuff[writeCnt].ampliR,measBuff[writeCnt].cvdC,measBuff[writeCnt].tempZboza,measBuff[writeCnt].tempIn,measBuff[writeCnt].humIn,measBuff[writeCnt].tempOut,measBuff[writeCnt].humOut);
//		respSD=f_write(&File[1],&cBuf,charNum,&btwrt);
//		if(charNum != btwrt){
//			PC_SendResponse(respSD);
//			sprintf(cBuf,"Write error: cNum: %d BWr: %d\n\r",charNum,btwrt);
//			PC_Send(cBuf);
//			diskFullFlag=1;
//			break;
//		}
//	}
//	sprintf(cBuf,"Last write: cNum: %d BWr: %d\n\r",charNum,btwrt);
//	PC_Send(cBuf);
//	respSD=f_close(&File[1]);
//	PC_SendResponse(respSD);
//	PC_Send("File written & closed\n\r");
//	LED_Blue(OFF);
//}
