/*=======================================================================================*
 * @file    thmeas.c
 * @author  Marcin Badzioch
 * @version 1.0
 * @date    02-09-2017
 * @brief   Source file for thversal measurement module
 *======================================================================================*/

/**
 * @addtogroup Thmeas Description
 * @{
 * @brief  Temperature and humidity measurement module
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
#include "thmeas.h"
/*----------------------------- LOCAL OBJECT-LIKE MACROS -------------------------------*/
#define I2CTimeout 20000
/*---------------------------- LOCAL FUNCTION-LIKE MACROS ------------------------------*/

/*======================================================================================*/
/*                      ####### LOCAL TYPE DECLARATIONS #######                         */
/*======================================================================================*/
/*---------------------------- ALL TYPE DECLARATIONS -----------------------------------*/

/*-------------------------------- OTHER TYPEDEFS --------------------------------------*/

/*------------------------------------- ENUMS ------------------------------------------*/

/*------------------------------- STRUCT AND THONS ------------------------------------*/

/*======================================================================================*/
/*                    ####### LOCAL FUNCTIONS PROTOTYPES #######                        */
/*======================================================================================*/

static void TH_IC_Config(uint8_t mode);
static void TH_PortConfig(uint8_t mode);
static uint8_t TempHum_FlagWait(uint16_t flag);
static void TH_GetH(void);
static void TH_GetT(void);
static void TH_Measure(void);

/*======================================================================================*/
/*                         ####### OBJECT DEFINITIONS #######                           */
/*======================================================================================*/
/*--------------------------------- EXPORTED OBJECTS -----------------------------------*/
thmeas_data_T thmeas_data;
/*---------------------------------- LOCAL OBJECTS -------------------------------------*/

/*======================================================================================*/
/*                  ####### EXPORTED FUNCTIONS DEFINITIONS #######                      */
/*======================================================================================*/
thmeas_resp_E ThMeas_Init(void)
{
	thmeas_data.sensInPresentFlag=1;
	thmeas_data.sensOutPresentFlag=1;
	TH_IC_Config(1);
	return TH_OK;
}
thmeas_resp_E ThMeas_Get()
{
	TH_Measure();

	if(thmeas_data.sensInPresentFlag==0)return TH_NO_SENS_IN;
	else if(thmeas_data.sensOutPresentFlag==0)return TH_NO_SENS_OUT;
	return TH_OK;
}
/*======================================================================================*/
/*                   ####### LOCAL FUNCTIONS DEFINITIONS #######                        */
/*======================================================================================*/
static void TH_IC_Config(uint8_t mode)
{
	I2C_InitTypeDef			   I2C_InitStructure;

	RCC_I2CCLKConfig(RCC_I2C1CLK_SYSCLK);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);

	I2C_DeInit(I2C1);
	TH_PortConfig(mode);

	I2C_StructInit(&I2C_InitStructure);
	I2C_InitStructure.I2C_Timing = 0xf0115060;
	I2C_InitStructure.I2C_AnalogFilter = I2C_AnalogFilter_Enable;
	I2C_InitStructure.I2C_DigitalFilter = 0x1;
	I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
	I2C_InitStructure.I2C_OwnAddress1 = 0x00;
	I2C_InitStructure.I2C_Ack = I2C_Ack_Disable;
	I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
	I2C_Init(I2C1,&I2C_InitStructure);
	I2C_Cmd(I2C1,ENABLE);
}
/*
 * mode: 0 - Inside
 * 		 1 - Outside
 */
static void TH_PortConfig(uint8_t mode)
{
	GPIO_InitTypeDef		   GPIO_InitStructure;

	switch(mode){
	case 0:
		GPIO_StructInit(&GPIO_InitStructure);
		GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_6 | GPIO_Pin_7;
		GPIO_Init(GPIOB, &GPIO_InitStructure);
		GPIO_PinAFConfig(GPIOB,GPIO_PinSource6,GPIO_AF_0);
		GPIO_PinAFConfig(GPIOB,GPIO_PinSource7,GPIO_AF_0);

		GPIO_StructInit(&GPIO_InitStructure);
		GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_8 | GPIO_Pin_9;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
		GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
		GPIO_Init(GPIOB, &GPIO_InitStructure);
		GPIO_PinAFConfig(GPIOB,GPIO_PinSource8,GPIO_AF_4);
		GPIO_PinAFConfig(GPIOB,GPIO_PinSource9,GPIO_AF_4);
		break;
	case 1:
		GPIO_StructInit(&GPIO_InitStructure);
		GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_8 | GPIO_Pin_9;
		GPIO_Init(GPIOB, &GPIO_InitStructure);
		GPIO_PinAFConfig(GPIOB,GPIO_PinSource8,GPIO_AF_0);
		GPIO_PinAFConfig(GPIOB,GPIO_PinSource9,GPIO_AF_0);

		GPIO_StructInit(&GPIO_InitStructure);
		GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_6 | GPIO_Pin_7;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
		GPIO_Init(GPIOB, &GPIO_InitStructure);
		GPIO_PinAFConfig(GPIOB,GPIO_PinSource6,GPIO_AF_4);
		GPIO_PinAFConfig(GPIOB,GPIO_PinSource7,GPIO_AF_4);
		break;
	default:
		break;
	}
}

static uint8_t TempHum_FlagWait(uint16_t flag)
{
	uint16_t cnt=0;

	while(I2C_GetFlagStatus(I2C1,flag)!= RESET){
		cnt++;
		if(cnt > I2CTimeout){
			return 1;
		}
	}
	return 0;
}
static void TH_GetH(void)
{
	thmeas_data.humRaw=0;
	I2C_TransferHandling(I2C1,0x80,1,I2C_AutoEnd_Mode,I2C_Generate_Start_Write);
	TempHum_FlagWait(I2C_FLAG_TXE);
	I2C_SendData(I2C1,0xe5);
	TempHum_FlagWait(I2C_FLAG_TXE);
	I2C_TransferHandling(I2C1,0x80,2,I2C_SoftEnd_Mode,I2C_Generate_Start_Read);
	TempHum_FlagWait(I2C_FLAG_TXE);
	TempHum_FlagWait(I2C_FLAG_RXNE);
	thmeas_data.humRaw = I2C_ReceiveData(I2C1)<<8;
	I2C_AcknowledgeConfig(I2C1,ENABLE);
	TempHum_FlagWait(I2C_FLAG_RXNE);
	thmeas_data.humRaw |= I2C_ReceiveData(I2C1);
	I2C_GenerateSTOP(I2C1,ENABLE);
}
static void TH_GetT(void)
{
	thmeas_data.tempRaw=0;
	TempHum_FlagWait(I2C_FLAG_TXE);
	I2C_TransferHandling(I2C1,0x80,1,I2C_SoftEnd_Mode,I2C_Generate_Start_Write);
	TempHum_FlagWait(I2C_FLAG_TXE);
	I2C_SendData(I2C1,0xe0);
	TempHum_FlagWait(I2C_FLAG_TXE);
	I2C_TransferHandling(I2C1,0x80,2,I2C_SoftEnd_Mode,I2C_Generate_Start_Read);
	TempHum_FlagWait(I2C_FLAG_TXE);
	TempHum_FlagWait(I2C_FLAG_RXNE);
	thmeas_data.tempRaw = I2C_ReceiveData(I2C1)<<8;
	I2C_AcknowledgeConfig(I2C1,ENABLE);
	TempHum_FlagWait(I2C_FLAG_RXNE);
	thmeas_data.tempRaw |= I2C_ReceiveData(I2C1);
	I2C_GenerateSTOP(I2C1,ENABLE);
}

static void TH_Measure(void)
{
	if(thmeas_data.sensInPresentFlag==1){
		TH_IC_Config(0);
		TH_GetH();
		TH_GetT();
		thmeas_data.humIn = (int16_t)(((((double)thmeas_data.humRaw*125)/65536)-6)*10);
		thmeas_data.tempIn = (int16_t)(((((double)thmeas_data.tempRaw*175.72)/65536)-46.85)*10);
		if((thmeas_data.humRaw==0)&&(thmeas_data.tempRaw==0)){
			thmeas_data.sensInPresentFlag=0;
		}
	}
	if(thmeas_data.sensOutPresentFlag==1){
		TH_IC_Config(1);
		TH_GetH();
		TH_GetT();
		thmeas_data.humOut = (int16_t)(((((double)thmeas_data.humRaw*125)/65536)-6)*10);
		thmeas_data.tempOut = (int16_t)(((((double)thmeas_data.tempRaw*175.72)/65536)-46.85)*10);
		if((thmeas_data.humRaw==0)&&(thmeas_data.tempRaw==0)){
			thmeas_data.sensOutPresentFlag=0;
		}
	}
}

/**
 * @} end of group Thmeas
 */
