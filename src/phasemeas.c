/*=======================================================================================*
 * @file    phameas.c
 * @author  Marcin Badzioch
 * @version 1.0
 * @date    02-09-2017
 * @brief   Source file for phaversal measurement module
 *======================================================================================*/

/**
 * @addtogroup Phameas Description
 * @{
 * @brief  Phaversal measurement module
 */

/*======================================================================================*/
/*                       ####### PREPROCESSOR DIRECTIVES #######                        */
/*======================================================================================*/
/*-------------------------------- INCLUDE DIRECTIVES ----------------------------------*/
#include <phasemeas.h>
#include <stdio.h>
#include <stdlib.h>
#include <stm32f30x_conf.h>
#include "delay.h"
#include "debugkom.h"
#include "timer.h"
/*----------------------------- LOCAL OBJECT-LIKE MACROS -------------------------------*/

/*---------------------------- LOCAL FUNCTION-LIKE MACROS ------------------------------*/

/*======================================================================================*/
/*                      ####### LOCAL TYPE DECLARATIONS #######                         */
/*======================================================================================*/
/*---------------------------- ALL TYPE DECLARATIONS -----------------------------------*/

/*-------------------------------- OTHER TYPEDEFS --------------------------------------*/

/*------------------------------------- ENUMS ------------------------------------------*/

/*------------------------------- STRUCT AND PHAONS ------------------------------------*/

/*======================================================================================*/
/*                    ####### LOCAL FUNCTIONS PROTOTYPES #######                        */
/*======================================================================================*/
static void PHA_Measure(void);
static void HRTIM_Port_Config(void);
static void HRTIM_Interrupt_Config(void);
static void HRTIM_Config(void);
/*======================================================================================*/
/*                         ####### OBJECT DEFINITIONS #######                           */
/*======================================================================================*/
/*--------------------------------- EXPORTED OBJECTS -----------------------------------*/

/*---------------------------------- LOCAL OBJECTS -------------------------------------*/

phasemeas_filtered_T phasemeas_filtered_data;

volatile uint32_t fiVal=0;
volatile uint16_t fiSampleCounter=0,fiValA=0,fiValB=0,fiValAvg=0;
volatile uint8_t  fiMeasOnFlag=0;
/*======================================================================================*/
/*                  ####### EXPORTED FUNCTIONS DEFINITIONS #######                      */
/*======================================================================================*/
phameas_resp_E PhaMeas_Init(void)
{
	HRTIM_Port_Config();
	HRTIM_Interrupt_Config();
	HRTIM_Config();

	phasemeas_filtered_data.timeout_flag=0;
	return PHA_OK;
}
phameas_resp_E PhaMeas_Get()
{
	phasemeas_filtered_data.timeout_flag=0;
	PHA_Measure();
	if(phasemeas_filtered_data.timeout_flag==1)return PHA_TIMEOUT;
	else return PHA_OK;
}

/*======================================================================================*/
/*                   ####### LOCAL FUNCTIONS DEFINITIONS #######                        */
/*======================================================================================*/

static void PHA_Measure()
{
	uint32_t fiTimeout=0;
	char cbuf[36];
	fiMeasOnFlag=1;
	fiValAvg=0;
	HRTIM_ITConfig(HRTIM1,0,HRTIM_TIM_IT_CPT2,ENABLE);
	while(fiMeasOnFlag){
		fiTimeout++;
		if(fiTimeout > 500000){
			phasemeas_filtered_data.timeout_flag=1;
			break;
		}
	}
	HRTIM_ITConfig(HRTIM1,0,HRTIM_TIM_IT_CPT2,DISABLE);

	phasemeas_filtered_data.phase = fiValAvg;
}

static void HRTIM_Port_Config(void)
{
	GPIO_InitTypeDef		   			GPIO_InitStructure;

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOC, ENABLE);

	GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_11 | GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	GPIO_PinAFConfig(GPIOC,GPIO_PinSource11,GPIO_AF_3); // EEV Channel 2 - R
	GPIO_PinAFConfig(GPIOC,GPIO_PinSource12,GPIO_AF_3); // EEV Channel 1 - C
}

static void HRTIM_Interrupt_Config(void)
{
	NVIC_InitTypeDef		   		NVIC_InitStructure;

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_3);
	NVIC_InitStructure.NVIC_IRQChannel = HRTIM1_TIMA_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=0;
	NVIC_Init(&NVIC_InitStructure);
}

static void HRTIM_Config(void)
{
	HRTIM_BaseInitTypeDef					HRTIM_BaseInitStructure;
	HRTIM_BasicCaptureChannelCfgTypeDef		HRTIM_CaptureChannelCfgStructure;
	HRTIM_CaptureCfgTypeDef					HRTIM_CaptureCfgStructure;
	HRTIM_TimerEventFilteringCfgTypeDef		HRTIM_TimerEventFiltStructure;
	HRTIM_EventCfgTypeDef					HRTIM_EventCfgStructure;
	HRTIM_TimerCfgTypeDef					HRTIM_TimerCfgStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_HRTIM1,ENABLE);
	RCC_HRTIM1CLKConfig(RCC_HRTIM1CLK_PLLCLK);
	HRTIM_DeInit(HRTIM1);

	HRTIM_DLLCalibrationStart(HRTIM1,HRTIM_CALIBRATIONRATE_910);

	HRTIM_EventCfgStructure.Source = HRTIM_EVENTSRC_1;
	HRTIM_EventCfgStructure.Polarity = HRTIM_EVENTPOLARITY_HIGH;
	HRTIM_EventCfgStructure.Sensitivity = HRTIM_EVENTSENSITIVITY_RISINGEDGE;
	HRTIM_EventCfgStructure.Filter = HRTIM_EVENTFILTER_NONE;
	HRTIM_EventCfgStructure.FastMode = HRTIM_EVENTFASTMODE_ENABLE;
	HRTIM_EventConfig(HRTIM1,HRTIM_EVENT_1,&HRTIM_EventCfgStructure);
	HRTIM_EventConfig(HRTIM1,HRTIM_EVENT_2,&HRTIM_EventCfgStructure);

	HRTIM_TimerEventFiltStructure.Filter = HRTIM_TIMEVENTFILTER_NONE;
	HRTIM_TimerEventFiltStructure.Latch = HRTIM_TIMEVENTLATCH_DISABLED;
	HRTIM_TimerEventFilteringConfig(HRTIM1,0,HRTIM_EVENT_1,&HRTIM_TimerEventFiltStructure);
	HRTIM_TimerEventFilteringConfig(HRTIM1,0,HRTIM_EVENT_2,&HRTIM_TimerEventFiltStructure);

	HRTIM_CaptureCfgStructure.Trigger = HRTIM_CAPTURETRIGGER_EEV_1;
	HRTIM_WaveformCaptureConfig(HRTIM1,0,HRTIM_CAPTUREUNIT_1,&HRTIM_CaptureCfgStructure);
	HRTIM_CaptureCfgStructure.Trigger = HRTIM_CAPTURETRIGGER_EEV_2;
	HRTIM_WaveformCaptureConfig(HRTIM1,0,HRTIM_CAPTUREUNIT_2,&HRTIM_CaptureCfgStructure);

	HRTIM_CaptureChannelCfgStructure.CaptureUnit = HRTIM_CAPTUREUNIT_1;
	HRTIM_CaptureChannelCfgStructure.Event = HRTIM_EVENT_1;
	HRTIM_CaptureChannelCfgStructure.EventFilter = HRTIM_EVENTFILTER_NONE;
	HRTIM_CaptureChannelCfgStructure.EventPolarity = HRTIM_EVENTPOLARITY_HIGH;
	HRTIM_CaptureChannelCfgStructure.EventSensitivity = HRTIM_EVENTSENSITIVITY_RISINGEDGE;
	HRTIM_SimpleCaptureChannelConfig(HRTIM1,0,HRTIM_CAPTUREUNIT_1,&HRTIM_CaptureChannelCfgStructure);

	HRTIM_CaptureChannelCfgStructure.CaptureUnit = HRTIM_CAPTUREUNIT_2;
	HRTIM_CaptureChannelCfgStructure.Event = HRTIM_EVENT_2;
	HRTIM_CaptureChannelCfgStructure.EventFilter = HRTIM_EVENTFILTER_NONE;
	HRTIM_CaptureChannelCfgStructure.EventPolarity = HRTIM_EVENTPOLARITY_HIGH;
	HRTIM_CaptureChannelCfgStructure.EventSensitivity = HRTIM_EVENTSENSITIVITY_RISINGEDGE;
	HRTIM_SimpleCaptureChannelConfig(HRTIM1,0,HRTIM_CAPTUREUNIT_2,&HRTIM_CaptureChannelCfgStructure);

	HRTIM_SimpleCaptureStart(HRTIM1,0,HRTIM_CAPTUREUNIT_1);
	HRTIM_SimpleCaptureStart(HRTIM1,0,HRTIM_CAPTUREUNIT_2);

	HRTIM_TimerCfgStructure.PushPull = HRTIM_TIMPUSHPULLMODE_DISABLED;
	HRTIM_TimerCfgStructure.FaultEnable = HRTIM_TIMFAULTENABLE_NONE;
	HRTIM_TimerCfgStructure.FaultLock = HRTIM_TIMFAULTLOCK_READONLY;
	HRTIM_TimerCfgStructure.DeadTimeInsertion = HRTIM_TIMDEADTIMEINSERTION_DISABLED;
	HRTIM_TimerCfgStructure.DelayedProtectionMode = HRTIM_TIMDELAYEDPROTECTION_DISABLED;
	HRTIM_TimerCfgStructure.UpdateTrigger = HRTIM_TIMUPDATETRIGGER_NONE;
	HRTIM_TimerCfgStructure.ResetTrigger = HRTIM_TIMRESETTRIGGER_EEV_1;
	HRTIM_TimerCfgStructure.ResetTrigger = HRTIM_TIMRESETTRIGGER_NONE;
	HRTIM_TimerCfgStructure.ResetUpdate = HRTIM_TIMUPDATEONRESET_DISABLED;
	HRTIM_WaveformTimerConfig(HRTIM1,0,&HRTIM_TimerCfgStructure);


	HRTIM_BaseInitStructure.Mode = HRTIM_MODE_CONTINOUS;
	HRTIM_BaseInitStructure.PrescalerRatio = HRTIM_PRESCALERRATIO_MUL16;
	HRTIM_BaseInitStructure.Period = 0xFFFD;
	HRTIM_BaseInitStructure.RepetitionCounter = 0xFF;
	HRTIM_SimpleCapture_Init(HRTIM1,0,&HRTIM_BaseInitStructure);

	HRTIM_SimpleBaseStart(HRTIM1, 0);

	//HRTIM_ITConfig(HRTIM1,0,HRTIM_TIM_IT_CPT2,ENABLE);

	//HRTIM_ITConfig(HRTIM1,0,HRTIM_TIM_IT_RST,ENABLE);

}
/*======================================================================================*/
/*                    #######    INTERRUPT ROUTINES   #######                           */
/*======================================================================================*/
void HRTIM_TIMA_IRQHandler()
{
	if(HRTIM_GetITStatus(HRTIM1,0,HRTIM_TIM_IT_CPT2) != RESET){
		fiValB = HRTIM_GetCapturedValue(HRTIM1,0,HRTIM_CAPTUREUNIT_2);
		fiValA = HRTIM_GetCapturedValue(HRTIM1,0,HRTIM_CAPTUREUNIT_1);
		if(fiValB>fiValA){
			if(fiValB-fiValA < 20000){
				fiVal += (fiValB-fiValA);
				fiSampleCounter++;
			}
		}
		if(fiSampleCounter==16384){
			fiSampleCounter=0;
			fiValAvg = fiVal >> 14;
			fiVal=0;
			fiMeasOnFlag=0;
		}
		HRTIM_ClearITPendingBit(HRTIM1,0,HRTIM_TIM_IT_CPT2);
	}
	/*else if(HRTIM_GetITStatus(HRTIM1,0,HRTIM_TIM_IT_RST) != RESET){
		fiTimOvfFlag=1;
		HRTIM_ClearITPendingBit(HRTIM1,0,HRTIM_TIM_IT_RST);
	}*/
}
/**
 * @} end of group Phameas
 */
