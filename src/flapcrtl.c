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
static void FLAP_SwPinConfig(void);
/*======================================================================================*/
/*                         ####### OBJECT DEFINITIONS #######                           */
/*======================================================================================*/
/*--------------------------------- EXPORTED OBJECTS -----------------------------------*/

/*---------------------------------- LOCAL OBJECTS -------------------------------------*/

/*======================================================================================*/
/*                  ####### EXPORTED FUNCTIONS DEFINITIONS #######                      */
/*======================================================================================*/

void FLAP_Init(void)
{
	FLAP_SwPinConfig();
}

flap_state_T FLAP_Control(flap_cmd_T flap_cmd_set)
{
	if(flap_cmd_set == FLAP_CMD_OPEN){
		return FLAP_OPENED;
	}
	else if(flap_cmd_set == FLAP_CMD_CLOSE){
		return FLAP_CLOSED;
	}
	return FLAP_ERROR;
}

/*======================================================================================*/
/*                   ####### LOCAL FUNCTIONS DEFINITIONS #######                        */
/*======================================================================================*/
/*
 * Returns:
 * 			0 - Closing / Opening
 * 			1 - Close / Open success
 * 			2 - Opening Failed
 * 			3 - Closing Failed
 */
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


/*
 *  1 - alarm deactivate
 *  0 - countdown deactivation time
 */
void FLAP_AlarmDeactivate(uint8_t state)
{
	static uint8_t downCnt=0;
	if(state!=0){
		GPIO_SetBits(GPIOA,GPIO_Pin_9); // Deaktywacja syreny
		//flapAlarmDeactivateFlag=1;
		downCnt=45;
	}
	else if(--downCnt == 0){
		GPIO_ResetBits(GPIOA,GPIO_Pin_9); // Aktywacja syreny
		//flapAlarmDeactivateFlag=0;
	}
}

void Measure_FlapCtrlConfig(void)
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

static void FLAP_SwPinConfig(void)
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

	GPIO_ResetBits(GPIOC,GPIO_Pin_6);
	GPIO_ResetBits(GPIOC,GPIO_Pin_7);

	GPIO_SetBits(GPIOC,GPIO_Pin_6);
	Delay_ms(5);
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


/**
 * @} end of group flapcrtl.c
 */
