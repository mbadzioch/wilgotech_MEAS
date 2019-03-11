/*=======================================================================================*
 * @file    measure.c
 * @author  Marcin Badzioch
 * @version 1.0
 * @date    19-05-2018
 * @brief   Source file for Measurement module
 *======================================================================================*/

/**
 * @addtogroup Measure Description
 * @{
 * @brief  Measurement module
 */

/*======================================================================================*/
/*                       ####### PREPROCESSOR DIRECTIVES #######                        */
/*======================================================================================*/
/*-------------------------------- INCLUDE DIRECTIVES ----------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <stm32f30x_conf.h>
#include <measure.h>
#include "thmeas.h"
#include "phasemeas.h"
#include "ampmeas.h"
#include "timer.h"
#include "flapcrtl.h"
#include "debugkom.h"
#include "sinus.h"
#include "led.h"
#include "rtc.h"
#include "memory.h"
/*----------------------------- LOCAL OBJECT-LIKE MACROS -------------------------------*/
#define MEAS_VAL_TIME_S 2  //  2s
#define MEAS_REC_TIME_S 60 //  60s

#define MEAS_FLOW_REC_NUM_INIT 15   // 15 x REC_TIME
#define MEAS_MOIST_REC_NUM_INIT 3  // 3 x REC_TIME

/*---------------------------- LOCAL FUNCTION-LIKE MACROS ------------------------------*/

/*======================================================================================*/
/*                      ####### LOCAL TYPE DECLARATIONS #######                         */
/*======================================================================================*/
/*---------------------------- ALL TYPE DECLARATIONS -----------------------------------*/

/*-------------------------------- OTHER TYPEDEFS --------------------------------------*/

/*------------------------------------- ENUMS ------------------------------------------*/

typedef enum {ACC_PHASE=0,ACC_AMP_R,ACC_AMP_C,ACC_TEMP_Z} measure_acc_data_T;
typedef enum {CALC_FLOW,CALC_MOIST} measure_calc_T;
typedef enum {OUT_SEND, OUT_SAVESEND} measures_out_T;
/*------------------------------- STRUCT AND UNIONS ------------------------------------*/

/*======================================================================================*/
/*                    ####### LOCAL FUNCTIONS PROTOTYPES #######                        */
/*======================================================================================*/

				// STATE MACHINE - State functions:
static measure_state_T Measure_Idle(void);
static measure_state_T Measure_FlapOpen(void);
static measure_state_T Measure_Flow(void);
static measure_state_T Measure_FlapClose(void);
static measure_state_T Measure_Moist(void);
static uint8_t Measure_RtcTimer(uint8_t resetFlag);
				// Sub routines:
static void Measure_AccumulateResults(void);
static void Measure_AccClear(void);
static void Measure_CalcRec(measure_calc_T calc_type);

static void Measure_Output(measures_out_T out_type);
/*======================================================================================*/
/*                         ####### OBJECT DEFINITIONS #######                           */
/*======================================================================================*/
/*--------------------------------- EXPORTED OBJECTS -----------------------------------*/
uint8_t measure_idle_flag=0;

measure_set_T measure_settings;
measure_record_T measure_current_record;

extern ampmeas_filtered_data_T ampmeas_filtered_data;
extern phasemeas_filtered_T phasemeas_filtered_data;
extern thmeas_data_T thmeas_data;
extern rtc_time_T rtc_current_time;

/*---------------------------------- LOCAL OBJECTS -------------------------------------*/
measure_state_T measure_state;
measure_set_T measure_settings;


//uint8_t recTim,curValTim;

uint16_t acc_cnt;
uint32_t measure_data_acc[4]; // Acc for Phase, AmpR, AmpC, TempZ

// TODO: DEBUG
char cbuf[64];
/*======================================================================================*/
/*                  ####### EXPORTED FUNCTIONS DEFINITIONS #######                      */
/*======================================================================================*/
void Measure_Init(void)
{
	SIN_Init();
	PhaMeas_Init();
	AmpMeas_Init();
	ThMeas_Init();
	LED_Init();
	FLAP_Init();

	//Timer_Register(&curValTim,MEAS_VAL_TIME_MS,timerOpt_AUTORESET);
	//Timer_Register(&recTim,MEAS_REC_TIME_MS,timerOpt_AUTORESET);

	measure_state=MEAS_IDLE;

	measure_settings.idle_flag=0;
	measure_settings.flow_rec_num=MEAS_FLOW_REC_NUM_INIT;
	measure_settings.moist_rec_num=MEAS_MOIST_REC_NUM_INIT;

	measure_data_acc[ACC_PHASE]=0;
	measure_data_acc[ACC_AMP_C]=0;
	measure_data_acc[ACC_AMP_R]=0;
	measure_data_acc[ACC_TEMP_Z]=0;
}

void Measure_Set(measure_set_T measure_set_S)
{
	measure_settings.idle_flag = measure_set_S.idle_flag;
	measure_settings.flow_rec_num = measure_set_S.flow_rec_num;
	measure_settings.moist_rec_num = measure_set_S.moist_rec_num;
}

void Measure_Main()
{
	switch(measure_state){

	case MEAS_IDLE:
		LED_Green(LED_OFF);
		measure_state = Measure_Idle();
		break;
	case MEAS_FLAPOPEN:
		LED_Green(LED_FAST);
		measure_state = Measure_FlapOpen();
		break;
	case MEAS_FLOW:
		LED_Green(LED_MEDIUM);
		measure_state = Measure_Flow();
		break;
	case MEAS_FLAPCLOSE:
		LED_Green(LED_FAST);
		measure_state = Measure_FlapClose();
		break;
	case MEAS_MOIST:
		LED_Green(LED_MEDIUM);
		measure_state = Measure_Moist();
		break;
	case MEAS_ERROR:
		LED_Green(LED_SLOW);
		break;
	default:
		break;

	}
}

/*======================================================================================*/
/*                   ####### LOCAL FUNCTIONS DEFINITIONS #######                        */
/*======================================================================================*/

static measure_state_T Measure_Idle(void)
{
	if(measure_settings.idle_flag == 1){
		return MEAS_IDLE;
	}
	// TODO: DEBUG
	PC_Debug("OPENING\n\r");
	return MEAS_FLAPOPEN;
}
static measure_state_T Measure_FlapOpen(void)
{
	flap_state_T flap_state;
	flap_state = FLAP_Control(FLAP_CMD_OPEN);

	if(flap_state == FLAP_OPENING){
		return MEAS_FLAPOPEN;
	}
	else if(flap_state == FLAP_OPENED){
		if(measure_idle_flag==1){
			return MEAS_IDLE;
		}
		Measure_RtcTimer(1);
		// TODO: DEBUG
		PC_Debug("OPENED\n\r");
		return MEAS_FLOW;
	}
	return MEAS_ERROR;
}
static measure_state_T Measure_Flow(void)
{
	static uint8_t flow_rec_cnt;

	Measure_AccumulateResults();

	switch(Measure_RtcTimer(0)){
	case 0:
		break;
	case 1:
		Measure_CalcRec(CALC_FLOW);
		Measure_Output(OUT_SEND);
		break;
	case 2:
		Measure_CalcRec(CALC_FLOW);
		Measure_Output(OUT_SAVESEND);
		Measure_AccClear();
		flow_rec_cnt++;
		break;
	default:
		break;
	}

	if(flow_rec_cnt >= measure_settings.flow_rec_num){
		flow_rec_cnt=0;
		// TODO: DEBUG
		PC_Debug("CLOSING\n\r");
		return MEAS_FLAPCLOSE;
	}
	return MEAS_FLOW;
}
static measure_state_T Measure_FlapClose(void)
{
	flap_state_T flap_state;
	flap_state = FLAP_Control(FLAP_CMD_CLOSE);

	if(flap_state == FLAP_CLOSING){
		return MEAS_FLAPCLOSE;
	}
	else if(flap_state == FLAP_CLOSED){
		Measure_RtcTimer(1);
		// TODO: DEBUG
		PC_Debug("CLOSED\n\r");
		return MEAS_MOIST;
	}
	return MEAS_ERROR;
}
static measure_state_T Measure_Moist(void)
{
	static uint8_t moist_rec_cnt;

	Measure_AccumulateResults();

	switch(Measure_RtcTimer(0)){
	case 0:
		break;
	case 1:
		Measure_CalcRec(CALC_MOIST);
		Measure_Output(OUT_SEND);
		break;
	case 2:
		Measure_CalcRec(CALC_MOIST);
		Measure_Output(OUT_SAVESEND);
		Measure_AccClear();
		moist_rec_cnt++;
		break;
	default:
		break;
	}

	if(moist_rec_cnt >= measure_settings.moist_rec_num){
		moist_rec_cnt=0;
		// TODO: DEBUG
		PC_Debug("OPENING\n\r");
		return MEAS_FLAPOPEN;
	}
	return MEAS_MOIST;
}

static void Measure_AccumulateResults(void)
{

	AmpMeas_Get();
	PhaMeas_Get();


	if(acc_cnt < 65000){
		acc_cnt++;
		measure_data_acc[ACC_PHASE] += phasemeas_filtered_data.phase;
		measure_data_acc[ACC_AMP_C] += ampmeas_filtered_data.amplitudeCapacitor;
		measure_data_acc[ACC_AMP_R] += ampmeas_filtered_data.amplitudeResistor;
		measure_data_acc[ACC_TEMP_Z] += ampmeas_filtered_data.wheatTemperature;
	}
}
static void Measure_AccClear(void)
{
	sprintf(cbuf,"CNT: %d",acc_cnt);

	measure_data_acc[ACC_PHASE]=0;
	measure_data_acc[ACC_AMP_C]=0;
	measure_data_acc[ACC_AMP_R]=0;
	measure_data_acc[ACC_TEMP_Z]=0;
	acc_cnt=0;
}
static void Measure_CalcRec(measure_calc_T calc_type)
{
	ThMeas_Get();
	RTC_GetDateTime();

	measure_current_record.timestamp = rtc_current_time.timestamp;

	measure_current_record.phase = measure_data_acc[ACC_PHASE] / acc_cnt;
	measure_current_record.amp_c = measure_data_acc[ACC_AMP_C] / acc_cnt;
	measure_current_record.amp_r = measure_data_acc[ACC_AMP_R] / acc_cnt;
	measure_current_record.temp_z = measure_data_acc[ACC_TEMP_Z] / acc_cnt;

	measure_current_record.temp_in = thmeas_data.tempIn;
	measure_current_record.hum_in = thmeas_data.humIn;
	measure_current_record.temp_out = thmeas_data.tempOut;
	measure_current_record.hum_out = thmeas_data.humOut;

	if(calc_type == CALC_FLOW){
		measure_current_record.flow = measure_current_record.phase;
	}
	else if(calc_type == CALC_MOIST){
		measure_current_record.moist = measure_current_record.phase;
	}
}

static void Measure_Output(measures_out_T out_type)
{
	if(out_type == OUT_SEND){
		// TODO: DEBUG
		sprintf(cbuf,"SEND: %d %d:%d\n\rF:%d %d C:%d R:%d Tz:%d Ti:%d Hi:%d To:%d Ho:%d %d %d\n\n\r",measure_current_record.timestamp/10000,
				(measure_current_record.timestamp%10000)/100,measure_current_record.timestamp%100,
				phasemeas_filtered_data.phase,measure_current_record.phase,
				measure_current_record.amp_c,measure_current_record.amp_r,
				measure_current_record.temp_z,measure_current_record.temp_in,
				measure_current_record.hum_in,measure_current_record.temp_out,
				measure_current_record.hum_out,measure_current_record.flow,
				measure_current_record.moist);
		PC_Debug(cbuf);
	}
	else if(out_type == OUT_SAVESEND){
		// TODO: DEBUG
		sprintf(cbuf,"SAVE_SEND: %d\n\r%d %d %d %d %d %d %d %d %d %d\n\n\r",measure_current_record.timestamp,
				measure_current_record.phase,
				measure_current_record.amp_c,measure_current_record.amp_r,
				measure_current_record.temp_z,measure_current_record.temp_in,
				measure_current_record.hum_in,measure_current_record.temp_out,
				measure_current_record.hum_out,measure_current_record.flow,
				measure_current_record.moist);
		//PC_Debug(cbuf);

		if(Memory_SaveRec() == MEM_OK){
			PC_Debug(cbuf);
		}
		else{
			PC_Debug("MEM_ERR!\n\r");
		}
	}
}

/*
 * RTC Based Timer
 *
 * Returns:
 *
 *  0 - running
 *  1 - send values
 *  2 - save and send record
 *
 */
static uint8_t Measure_RtcTimer(uint8_t resetFlag)
{
	static uint8_t lastSecond;
	static uint8_t valuesCounter = MEAS_VAL_TIME_S;
	static uint8_t recordCounter = MEAS_REC_TIME_S;

	if(resetFlag == 1){
		valuesCounter = MEAS_VAL_TIME_S;
		recordCounter = MEAS_REC_TIME_S;
		return 0;
	}

	RTC_GetCurrentSecond();

	if(lastSecond != rtc_current_time.seconds){
		valuesCounter--;
		recordCounter--;
		lastSecond = rtc_current_time.seconds;
	}

	if(recordCounter == 0){
		valuesCounter = MEAS_VAL_TIME_S;
		recordCounter = MEAS_REC_TIME_S;
		return 2;
	}
	else if(valuesCounter == 0){
		valuesCounter = MEAS_VAL_TIME_S;
		return 1;
	}
	return 0;
}
/*
 * Legacy sequence
 */

//void Measure_Sequence(sTimeS* sTime)
//{
//	static uint32_t startSDHms=0,startMeasHms=0;
//	uint8_t flapResp=0;
//	if(flapAlarmDeactivateFlag){
//		FLAP_AlarmDeactivate(0);
//	}
//	switch(measureState){
//		case INIT:
//			LED_Green(SLOW);
//			Measure_SD_Write(sTime,INFO);
//			measureState = FLAPOPEN;
//			break;
//		case MEASFLOW:
//			LED_Green(ON);
//			measureBuffCnt++;
//			Measure_SwitchNormal();
//			Delay_ms(5);
//			Fazomierz_Measure(&measBuff[measureBuffCnt]);
//			AmpliMeasureR(&measBuff[measureBuffCnt]);
//			AmpliMeasureC(&measBuff[measureBuffCnt]);
//			Measure_SwitchCVD();
//			Delay_ms(5);
//			CVD_MeasureC(&measBuff[measureBuffCnt]);
//			ThermistorMeasure(&measBuff[measureBuffCnt]);
//			TempHum_Measure(&measBuff[measureBuffCnt]);
//			Measure_SaveTimeStamp(sTime,&measBuff[measureBuffCnt]);
//			if((sTime->hms - startSDHms) >= 60*SD_WRITE_INTERVAL){
//				Measure_SD_Write(sTime,FLOW);
//				startSDHms=sTime->hms;
//				measureBuffCnt=0;
//			}
//			if((sTime->hms - startMeasHms) >= 60*FLOW_MEAS_TIME){
//				measureState = FLAPCLOSE;
//			}
//			LED_Green(OFF);
//			break;
//		case MEASMOIST:
//			LED_Green(ON);
//			measureBuffCnt++;
//			Measure_SwitchNormal();
//			Delay_ms(5);
//			Fazomierz_Measure(&measBuff[measureBuffCnt]);
//			AmpliMeasureR(&measBuff[measureBuffCnt]);
//			AmpliMeasureC(&measBuff[measureBuffCnt]);
//			Delay_ms(5);
//			ThermistorMeasure(&measBuff[measureBuffCnt]);
//			TempHum_Measure(&measBuff[measureBuffCnt]);
//			Measure_SaveTimeStamp(sTime,&measBuff[measureBuffCnt]);
//			if((sTime->hms - startSDHms) >= 60*SD_WRITE_INTERVAL){
//				Measure_SD_Write(sTime,MOISTURE);
//				startSDHms=sTime->hms;
//				measureBuffCnt=0;
//			}
//			if((sTime->hms - startMeasHms) >= 60*MOISTURE_MEAS_TIME){
//				measureState = FLAPOPEN;
//			}
//			LED_Green(OFF);
//			break;
//		case FLAPCLOSE:
//			flapResp=Measure_FlapCtrl(CLOSE,sTime->seconds);
//			switch(flapResp){
//				case 0:
//					LED_Green(FAST);
//					break;
//				case 1:
//					LED_Green(ONEBLINKS);
//					LED_Red(OFF);
//					measureState = MEASMOIST;
//					startMeasHms=sTime->hms;
//					startSDHms=sTime->hms;
//
//					break;
//				case 3:
//					measureState = MEASFLOW;
//					startMeasHms=sTime->hms;
//					startSDHms=sTime->hms;
//					LED_Red(MEDIUM);
//					break;
//				default:
//					LED_Red(FAST);
//					break;
//			}
//			break;
//		case FLAPOPEN:
//			flapResp=Measure_FlapCtrl(OPEN,sTime->seconds);
//			switch(flapResp){
//				case 0:
//					LED_Green(FAST);
//					break;
//				case 1:
//					LED_Green(ONEBLINKS);
//					LED_Red(OFF);
//					measureState = MEASFLOW;
//					startMeasHms=sTime->hms;
//					startSDHms=sTime->hms;
//
//					break;
//				case 2:
//					measureState = MEASMOIST;
//					startMeasHms=sTime->hms;
//					startSDHms=sTime->hms;
//
//					Measure_SD_Write(sTime,INFO);
//					LED_Red(MEDIUM);
//					break;
//				default:
//					LED_Red(FAST);
//					break;
//			}
//			break;
//		default:
//			break;
//	}
//
//#if MEAS_PRINT_UART
//	sprintf(cBuf,"fi: %5d, Ac %4d, Ar %4d, C: %5d, Tz: %i, Ti: %i, Hi: %d, To: %d, Ho: %d\n\r",measBuff[measureBuffCnt].faza,measBuff[measureBuffCnt].ampliC,measBuff[measureBuffCnt].ampliR,measBuff[measureBuffCnt].cvdC,measBuff[measureBuffCnt].tempZboza,measBuff[measureBuffCnt].tempIn,measBuff[measureBuffCnt].humIn,measBuff[measureBuffCnt].tempOut,measBuff[measureBuffCnt].humOut);
//	PC_Send(cBuf);
//	switch(measureState){
//	case 0:
//		PC_Send("STATE INIT\n\r");
//		break;
//	case 1:
//		PC_Send("STATE FLOW\n\r");
//		break;
//	case 2:
//		PC_Send("STATE MOISTURE\n\r");
//		break;
//	case 3:
//		PC_Send("STATE FLAPCLOSE\n\r");
//		break;
//	case 4:
//		PC_Send("STATE FLAPOPEN\n\r");
//		break;
//	default:
//		break;
//	}
//	switch(flapState){
//	case 0:
//		PC_Send("FLAP OPENED\n\r");
//		break;
//	case 1:
//		PC_Send("FLAP OPENING\n\r");
//		break;
//	case 2:
//		PC_Send("FLAP CLOSED\n\r");
//		break;
//	case 3:
//		PC_Send("FLAP CLOSING\n\r");
//		break;
//	default:
//		break;
//	}
//#endif
//
//}
///*
// * Test modes:
// * 		0 - Measure and print to UART
// * 		1 - Measure only Normal
// * 		2 - Measure only CVD
// * 		3 - Close Flap
// * 		4 - Open Flap
// * 		5 - IDLE (Measure VBat and print to UART)
// */
//void Measure_CallibSequence(sTimeS* sTime,uint8_t mode)
//{
//	static uint8_t stop=0,firstRun=1;
//	uint8_t writeCnt=0;
//
//	/*if(firstRun){
//		firstRun=0;
//		PC_Send("Date Time,fi,Ac,Ar,C,Tz,Ti,Hi,To,Ho\n\r");
//	}*/
//	switch(mode){
//	case 0:
//		LED_Green(ON);
//		Measure_SwitchNormal();
//		Delay_ms(5);
//		Fazomierz_Measure(&measBuff[writeCnt]);
//		AmpliMeasureR(&measBuff[writeCnt]);
//		AmpliMeasureC(&measBuff[writeCnt]);
//		Measure_SwitchCVD();
//		Delay_ms(5);
//		CVD_MeasureC(&measBuff[writeCnt]);
//		ThermistorMeasure(&measBuff[writeCnt]);
//		TempHum_Measure(&measBuff[writeCnt]);
//		Measure_SaveTimeStamp(sTime,&measBuff[writeCnt]);
//		sprintf(cBuf,"%.2d-%.2d-%2d %.2d:%.2d:%.2d,%u,%u,%u,%u,%i\n",sTime->date,sTime->month,sTime->year,measBuff[writeCnt].hms/3600,(measBuff[writeCnt].hms%3600)/60,measBuff[writeCnt].hms%60,measBuff[writeCnt].faza,measBuff[writeCnt].ampliC,measBuff[writeCnt].ampliR,measBuff[writeCnt].cvdC,measBuff[writeCnt].tempZboza);
//		PC_Send(cBuf);
//		LED_Green(OFF);
//		break;
//	case 1:
//		LED_Green(ON);
//		Measure_SwitchNormal();
//		Delay_ms(5);
//		Fazomierz_Measure(&measBuff[writeCnt]);
//		AmpliMeasureR(&measBuff[writeCnt]);
//		AmpliMeasureC(&measBuff[writeCnt]);
//		Measure_SwitchCVD();
//		Delay_ms(5);
//		CVD_MeasureC(&measBuff[writeCnt]);
//		ThermistorMeasure(&measBuff[writeCnt]);
//		TempHum_Measure(&measBuff[writeCnt]);
//		Measure_SaveTimeStamp(sTime,&measBuff[writeCnt]);
//		sprintf(cBuf,"%.2d-%.2d-%2d %.2d:%.2d:%.2d,fi: %u,Ac: %u,Ar: %u,C: %u,Tz: %i,Ti: %i,Hi: %u,To: %i,Ho: %u\n\r",sTime->date,sTime->month,sTime->year,measBuff[writeCnt].hms/3600,(measBuff[writeCnt].hms%3600)/60,measBuff[writeCnt].hms%60,measBuff[writeCnt].faza,measBuff[writeCnt].ampliC,measBuff[writeCnt].ampliR,measBuff[writeCnt].cvdC,measBuff[writeCnt].tempZboza,measBuff[writeCnt].tempIn,measBuff[writeCnt].humIn,measBuff[writeCnt].tempOut,measBuff[writeCnt].humOut);
//		PC_Send(cBuf);
//		LED_Green(OFF);
//		break;
//	case 3:
//		Measure_SD_Test(sTime);
//		break;
//	default:
//		break;
//	}
//}



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

/**
 * @} end of group Unimeas
 */
