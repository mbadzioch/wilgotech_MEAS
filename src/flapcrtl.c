/*=======================================================================================*
 * @file    flapcrtl.c
 * @author  Marcin
 * @version 1.0
 * @date    17 maj 2018
 * @brief   
 *======================================================================================*/

/**
 * @addtogroup flapcrtl.c Description
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
#include "flapcrtl.h"
/*----------------------------- LOCAL OBJECT-LIKE MACROS -------------------------------*/

//TODO: Ustawić czas tak by blokował także alarm (także docelowo musi być przesyłane info
// 		do displaya

#define FLAP_MOVE_TIME 3 // 15 sec

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
static void FLAP_SetPinOpen(void);
static void FLAP_SetPinClose(void);
static void FLAP_ResetPins(void);
static void FLAP_PinConfig(void);
static void FLAP_CVD(void);
static void FLAP_SetAlarm(uint8_t state);
/*======================================================================================*/
/*                         ####### OBJECT DEFINITIONS #######                           */
/*======================================================================================*/
/*--------------------------------- EXPORTED OBJECTS -----------------------------------*/
/*---------------------------------- LOCAL OBJECTS -------------------------------------*/
flap_state_T flap_state;
uint8_t flap_tim;
/*======================================================================================*/
/*                  ####### EXPORTED FUNCTIONS DEFINITIONS #######                      */
/*======================================================================================*/

void FLAP_Init(void)
{
	FLAP_CVD();
	FLAP_PinConfig();

	Timer_Register(&flap_tim,FLAP_MOVE_TIME,timerOpt_AUTOSTOP);
	Timer_Stop(&flap_tim);
	flap_state=FLAP_INIT;
}

flap_state_T FLAP_Control(flap_cmd_T flap_cmd_set)
{

	switch(flap_state){
	case FLAP_INIT:
		if(flap_cmd_set == FLAP_CMD_OPEN){
			FLAP_SetAlarm(0);
			FLAP_SetPinOpen();
			Timer_Reset(&flap_tim);
			flap_state = FLAP_OPENING;
		}
		else{
			FLAP_SetAlarm(0);
			FLAP_SetPinClose();
			Timer_Reset(&flap_tim);
			flap_state = FLAP_CLOSING;
		}
		break;
	case FLAP_OPENING:
		if(Timer_Check(&flap_tim) == 1){
			FLAP_SetAlarm(1);
			FLAP_ResetPins();
			flap_state = FLAP_OPENED;
		}
		break;
	case FLAP_OPENED:
		if(flap_cmd_set == FLAP_CMD_CLOSE){
			FLAP_SetAlarm(0);
			FLAP_SetPinClose();
			Timer_Reset(&flap_tim);
			flap_state = FLAP_CLOSING;
		}
		break;
	case FLAP_CLOSING:
		if(Timer_Check(&flap_tim) == 1){
			FLAP_SetAlarm(1);
			FLAP_ResetPins();
			flap_state = FLAP_CLOSED;
		}
		break;
	case FLAP_CLOSED:
		if(flap_cmd_set == FLAP_CMD_OPEN){
			FLAP_SetAlarm(0);
			FLAP_SetPinOpen();
			Timer_Reset(&flap_tim);
			flap_state = FLAP_OPENING;
		}
		break;
	default:
		break;
	}

	return flap_state;
}

/*======================================================================================*/
/*                   ####### LOCAL FUNCTIONS DEFINITIONS #######                        */
/*======================================================================================*/

static void FLAP_SetPinOpen(void)
{
	GPIO_SetBits(GPIOC,GPIO_Pin_2);
}
static void FLAP_SetPinClose(void)
{
	GPIO_SetBits(GPIOC,GPIO_Pin_2|GPIO_Pin_3);
}
static void FLAP_ResetPins(void)
{
	GPIO_ResetBits(GPIOC,GPIO_Pin_2|GPIO_Pin_3);
}

/*
 *  1 - activate
 *  0 - deactivate
 */
static void FLAP_SetAlarm(uint8_t state)
{
	if(state == 1){
		GPIO_ResetBits(GPIOA,GPIO_Pin_9);
	}
	else if(state == 0){
		GPIO_SetBits(GPIOA,GPIO_Pin_9);
	}
}

static void FLAP_PinConfig(void)
{
	GPIO_InitTypeDef		   GPIO_InitStructure;
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

static void FLAP_CVD(void)
{
	GPIO_InitTypeDef		   GPIO_InitStructure;
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


	GPIO_SetBits(GPIOC,GPIO_Pin_6);
	Delay_ms(50);
	GPIO_ResetBits(GPIOC,GPIO_Pin_6);
}

//void Measure_SwitchCVD(void)
//{
//	GPIO_ResetBits(GPIOC,GPIO_Pin_6);
//	GPIO_ResetBits(GPIOC,GPIO_Pin_7);
//
//	GPIO_SetBits(GPIOC,GPIO_Pin_7);
//	Delay_ms(5);
//	GPIO_ResetBits(GPIOC,GPIO_Pin_7);
//}
//void Measure_SwitchNormal(void)
//{
//	GPIO_ResetBits(GPIOC,GPIO_Pin_6);
//	GPIO_ResetBits(GPIOC,GPIO_Pin_7);
//
//	GPIO_SetBits(GPIOC,GPIO_Pin_6);
//	Delay_ms(5);
//	GPIO_ResetBits(GPIOC,GPIO_Pin_6);
//}

///*
// * Returns:
// * 			0 - Closing / Opening
// * 			1 - Close / Open success
// * 			2 - Opening Failed
// * 			3 - Closing Failed
// */
//uint8_t Measure_FlapCtrl(flapCmdE flapCmd,uint8_t sec)
//{
//	static uint8_t startSec=0,resetFlag=1;
//
//	switch(flapState){
//		case OPENED:
//			GPIO_ResetBits(GPIOC,GPIO_Pin_2|GPIO_Pin_3);
//			if(flapCmd == CLOSE){
//				GPIO_SetBits(GPIOC,GPIO_Pin_2|GPIO_Pin_3);
//				startSec=sec;
//				flapState = CLOSING;
//				FLAP_AlarmDeactivate(1);
//				return 0;
//			}
//			if(resetFlag!=0){
//				GPIO_SetBits(GPIOC,GPIO_Pin_2);
//				Delay_ms(200);
//				if(GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_0)==0){
//					GPIO_ResetBits(GPIOC,GPIO_Pin_2|GPIO_Pin_3);
//					resetFlag=0;
//				}
//				else{
//					flapState = OPENING;
//					startSec=sec;
//					return 0;
//				}
//			}
//			return 1;
//			break;
//		case CLOSED:
//			GPIO_ResetBits(GPIOC,GPIO_Pin_2|GPIO_Pin_3);
//			if(flapCmd == OPEN){
//				GPIO_SetBits(GPIOC,GPIO_Pin_2);
//				startSec=sec;
//				flapState = OPENING;
//				return 0;
//			}
//			return 1;
//			break;
//		case OPENING:
//			if(GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_0)==0){
//				flapState = OPENED;
//				resetFlag=0;
//			}
//			else{
//				Delay_ms(9);
//				if(GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_0)==0){
//					flapState = OPENED;
//					resetFlag=0;
//				}
//			}
//			if(abs(sec-startSec)>FLAP_OPEN_TIMEOUT){
//				GPIO_ResetBits(GPIOC,GPIO_Pin_2|GPIO_Pin_3);
//				if(resetFlag!=0){
//					resetFlag=0;
//					flapState = OPENED;
//					return 1;
//				}
//				flapState=CLOSED;
//				return 2;
//			}
//			return 0;
//		case CLOSING:
//			if(GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_1)==0){
//				flapState = CLOSED;
//			}
//			else{
//				Delay_ms(9);
//				if(GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_1)==0){
//					flapState = CLOSED;
//				}
//			}
//			if(abs(sec-startSec)>FLAP_OPEN_TIMEOUT){
//				GPIO_ResetBits(GPIOC,GPIO_Pin_2|GPIO_Pin_3);
//				flapState=OPENED;
//				return 3;
//			}
//			return 0;
//		default:
//			break;
//	}
//	return 0;
//}

/**
 * @} end of group flapcrtl.c
 */
