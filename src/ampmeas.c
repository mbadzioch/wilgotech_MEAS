/*=======================================================================================*
 * @file    ampmeas.c
 * @author  Marcin Badzioch
 * @version 1.0
 * @date    02-09-2017
 * @brief   Source file for Amplitude measurement module
 *======================================================================================*/

/**
 * @addtogroup Ampmeas Description
 * @{
 * @brief  Amplitude measurement module
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
#include "ampmeas.h"
/*----------------------------- LOCAL OBJECT-LIKE MACROS -------------------------------*/
#define TEMP_CALLIB 80
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
static void ADC_Port_Config(void);
static void ADC_Config(void);
static int16_t ADC_TempLinCalc(uint16_t adc);

static void ADC_CapacitorVoltageMeasure(void);
static void ADC_ResistorVoltageMeasure(void);
static void ADC_WheatTemperatureMeasure(void);
static void ADC_BatteryVoltageMeasure(void);

/*======================================================================================*/
/*                         ####### OBJECT DEFINITIONS #######                           */
/*======================================================================================*/
/*--------------------------------- EXPORTED OBJECTS -----------------------------------*/

ampmeas_filtered_data_T ampmeas_filtered_data;

/*---------------------------------- LOCAL OBJECTS -------------------------------------*/
/*int16_t tempTab[][2]={{4009,-200},
		{4007,-195},{4004,-190},{4001,-185},{3999,-180},{3996,-175},{3993,-170},
		{3990,-165},{3987,-160},{3984,-155},{3981,-150},{3978,-145},{3975,-140},{3972,-135},{3968,-130},{3965,-125},{3961,-120},{3958,-115},{3954,-110},{3950,-105},
		{3946,-100},{3942,-95},{3938,-90},{3934,-85},{3930,-80},{3926,-75},{3921,-70},{3917,-65},{3912,-60},{3907,-55},{3903,-50},{3898,-45},{3893,-40},{3887,-35},
		{3882,-30},{3877,-25},{3871,-20},{3866,-15},{3860,-10},{3854,-5},{3848,0},{3842,5},{3836,10},{3830,15},{3824,20},{3817,25},{3811,30},{3804,35},{3797,40},{3790,45},
		{3783,50},{3775,55},{3768,60},{3760,65},{3753,70},{3745,75},{3737,80},{3729,85},{3721,90},{3712,95},{3704,100},{3695,105},{3686,110},{3677,115},{3668,120},{3659,125},
		{3649,130},{3640,135},{3630,140},{3620,145},{3610,150},{3600,155},{3590,160},{3579,165},{3569,170},{3558,175},{3547,180},{3536,185},{3524,190},{3513,195},{3501,200},
		{3490,205},{3478,210},{3466,215},{3453,220},{3441,225},{3429,230},{3416,235},{3403,240},{3390,245},{3377,250},{3364,255},{3350,260},{3337,265},{3323,270},{3309,275},
		{3295,280},{3280,285},{3266,290},{3252,295},{3237,300},{3222,305},{3207,310},{3192,315},{3177,320},{3161,325},{3146,330},{3130,335},{3114,340},{3098,345},{3082,350},
		{3066,355},{3050,360},{3033,365},{3017,370},{3000,375},{2983,380},{2966,385},{2949,390},{2932,395},{2915,400},{2898,405},{2880,410},{2863,415},{2845,420},{2827,425},
		{2809,430},{2791,435},{2773,440},{2755,445},{2737,450},{2719,455},{2701,460},{2682,465},{2664,470},{2645,475},{2627,480},{2608,485},{2589,490},{2571,495},{2552,500}};*/




/*======================================================================================*/
/*                  ####### EXPORTED FUNCTIONS DEFINITIONS #######                      */
/*======================================================================================*/
ampmeas_resp_E AmpMeas_Init(void)
{
	ADC_Port_Config();
	ADC_Config();
	ADC_BatteryVoltageMeasure();
	return AMP_OK;
}
ampmeas_resp_E AmpMeas_Start(void)
{
	return AMP_NOTUSED;
}
ampmeas_resp_E AmpMeas_Stop(void)
{
	return AMP_NOTUSED;
}
ampmeas_resp_E AmpMeas_Set(void)
{
	return AMP_NOTUSED;
}
ampmeas_resp_E AmpMeas_Get(void)
{
	ADC_CapacitorVoltageMeasure();
	ADC_ResistorVoltageMeasure();
	ADC_WheatTemperatureMeasure();

	return AMP_OK;
}
/*======================================================================================*/
/*                   ####### LOCAL FUNCTIONS DEFINITIONS #######                        */
/*======================================================================================*/
/*
 * Port config:
 * 		PB14 - Amplitude C
 * 		PB15 - Amplitude R
 * 		PA0  - Thermistor
 */
static void ADC_Port_Config(void)
{
	GPIO_InitTypeDef			GPIO_InitStructure;

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);

	GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
}
static void ADC_Config(void)
{
	ADC_InitTypeDef      	 	ADC_InitStructure;
	ADC_CommonInitTypeDef 		ADC_CommonInitStructure;


	RCC_ADCCLKConfig(RCC_ADC12PLLCLK_Div2);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_ADC12, ENABLE);

	ADC_VoltageRegulatorCmd(ADC1, ENABLE);
	ADC_VoltageRegulatorCmd(ADC2, ENABLE);
	Delay_ms(1);
	ADC_DeInit(ADC1);
	ADC_DeInit(ADC2);
	ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;
	ADC_CommonInitStructure.ADC_Clock = ADC_Clock_AsynClkMode;
	ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;
	ADC_CommonInitStructure.ADC_DMAMode = ADC_DMAMode_OneShot;
	ADC_CommonInitStructure.ADC_TwoSamplingDelay = 2;

	ADC_CommonInit(ADC1, &ADC_CommonInitStructure);

	ADC_InitStructure.ADC_ContinuousConvMode = ADC_ContinuousConvMode_Disable;
	ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
	ADC_InitStructure.ADC_ExternalTrigConvEvent = ADC_ExternalTrigConvEvent_0;
	ADC_InitStructure.ADC_ExternalTrigEventEdge = ADC_ExternalTrigEventEdge_None;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_OverrunMode = ADC_OverrunMode_Disable;
	ADC_InitStructure.ADC_AutoInjMode = ADC_AutoInjec_Disable;
	ADC_InitStructure.ADC_NbrOfRegChannel = 1;
	ADC_Init(ADC1, &ADC_InitStructure);

	ADC_Cmd(ADC1, ENABLE);

	ADC_SelectCalibrationMode(ADC1, ADC_CalibrationMode_Single);
	ADC_StartCalibration(ADC1);


	ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;
	ADC_CommonInitStructure.ADC_Clock = ADC_Clock_AsynClkMode;
	ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;
	ADC_CommonInitStructure.ADC_DMAMode = ADC_DMAMode_OneShot;
	ADC_CommonInitStructure.ADC_TwoSamplingDelay = 2;

	ADC_CommonInit(ADC2, &ADC_CommonInitStructure);

	ADC_InitStructure.ADC_ContinuousConvMode = ADC_ContinuousConvMode_Disable;
	ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
	ADC_InitStructure.ADC_ExternalTrigConvEvent = ADC_ExternalTrigConvEvent_0;
	ADC_InitStructure.ADC_ExternalTrigEventEdge = ADC_ExternalTrigEventEdge_None;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_OverrunMode = ADC_OverrunMode_Disable;
	ADC_InitStructure.ADC_AutoInjMode = ADC_AutoInjec_Disable;
	ADC_InitStructure.ADC_NbrOfRegChannel = 1;
	ADC_Init(ADC2, &ADC_InitStructure);

	ADC_Cmd(ADC2, ENABLE);
	ADC_SelectCalibrationMode(ADC2, ADC_CalibrationMode_Single);
	ADC_StartCalibration(ADC2);

	/* wait for ADRDY */
	while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_RDY));
	while(!ADC_GetFlagStatus(ADC2, ADC_FLAG_RDY));

}
void ADC_Interrput_Config(void)
{
	NVIC_InitTypeDef		   	NVIC_InitStructure;

	NVIC_InitStructure.NVIC_IRQChannel = ADC1_2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=0;
	NVIC_Init(&NVIC_InitStructure);
}
static void ADC_CapacitorVoltageMeasure(void)
{
	uint32_t ampCAcc=0;

	ADC_RegularChannelConfig(ADC2, ADC_Channel_14, 1, ADC_SampleTime_601Cycles5);
	while(!ADC_GetFlagStatus(ADC2, ADC_FLAG_RDY));
	for(uint8_t i=0;i<32;i++){
		ADC_StartConversion(ADC2);
		while(ADC_GetFlagStatus(ADC2, ADC_FLAG_EOC) == RESET);
		ampCAcc = ampCAcc + ADC_GetConversionValue(ADC2);
	}

	ampmeas_filtered_data.amplitudeCapacitor =  ampCAcc >> 5;
}

static void ADC_ResistorVoltageMeasure(void)
{
	uint32_t ampRAcc=0;
	ADC_RegularChannelConfig(ADC2, ADC_Channel_15, 1, ADC_SampleTime_601Cycles5);
	while(!ADC_GetFlagStatus(ADC2, ADC_FLAG_RDY));
	for(uint8_t i=0;i<32;i++){
		ADC_StartConversion(ADC2);
		while(ADC_GetFlagStatus(ADC2, ADC_FLAG_EOC) == RESET);
		ampRAcc = ampRAcc + ADC_GetConversionValue(ADC2);
	}

	ampmeas_filtered_data.amplitudeResistor =  ampRAcc >> 5;
}

static int16_t ADC_TempLinCalc(uint16_t adc)
{
	double Temp;
	Temp=adc;
	Temp=-(Temp-3143);
	Temp=(10*Temp)/44;

	return (int16_t)Temp;
}

static void ADC_WheatTemperatureMeasure(void)
{
	uint32_t tempT=0;
	ADC_RegularChannelConfig(ADC1, ADC_Channel_1, 1, ADC_SampleTime_601Cycles5);
	while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_RDY));
	for(uint8_t i=0;i<32;i++){
		ADC_StartConversion(ADC1);
		while(ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET);
		tempT = tempT + ADC_GetConversionValue(ADC1);
	}
	ampmeas_filtered_data.wheatTemperature =  ADC_TempLinCalc((tempT>>5)+TEMP_CALLIB);
}
/*
 * Only at startup?
 */

static void ADC_BatteryVoltageMeasure(void)
{
	uint32_t batMeas=0;
	ADC_VbatCmd(ADC1, ENABLE);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_17, 1, ADC_SampleTime_601Cycles5);
	while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_RDY));
	ADC_StartConversion(ADC1);
	while(ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET);
	ADC_VbatCmd(ADC1, DISABLE);
	batMeas = ADC_GetConversionValue(ADC1);
	batMeas = (batMeas*6600)/4096;

	ampmeas_filtered_data.batteryVoltage = batMeas;
}
/*
 * Not needed
 */
//void TempInternalMeasure(measOtherS* measStartup)
//{
//	double tempMeas=0;
//	ADC_TempSensorCmd(ADC1, ENABLE);
//	ADC_RegularChannelConfig(ADC1, ADC_Channel_16, 1, ADC_SampleTime_601Cycles5);
//	while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_RDY));
//	ADC_StartConversion(ADC1);
//	while(ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET);
//	ADC_TempSensorCmd(ADC1, DISABLE);
//	tempMeas = ADC_GetConversionValue(ADC1);
//	tempMeas = (tempMeas*3.3)/4096;
//	tempMeas = ((1.43 - tempMeas)/ 0.0043)+25;
//	measStartup->tempProc = (uint16_t)(10*tempMeas);
//}
/**
 * @} end of group Ampmeas
 */
