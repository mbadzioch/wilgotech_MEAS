/*=======================================================================================*
 * @file    kom.c
 * @author  Marcin
 * @version 1.0
 * @date    22 maj 2018
 * @brief   
 *======================================================================================*/

/**
 * @addtogroup kom.c Description
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
#include "kom.h"
/*----------------------------- LOCAL OBJECT-LIKE MACROS -------------------------------*/
#define KOM_RCV_BUF_SIZE 255
#define KOM_PREFIX 0x01

#define KOM_FRAME_TIMEOUT_MS 20 // 20ms - timeout for frame receiving

/*---------------------------- LOCAL FUNCTION-LIKE MACROS ------------------------------*/

/*======================================================================================*/
/*                      ####### LOCAL TYPE DECLARATIONS #######                         */
/*======================================================================================*/
/*---------------------------- ALL TYPE DECLARATIONS -----------------------------------*/

/*-------------------------------- OTHER TYPEDEFS --------------------------------------*/

/*------------------------------------- ENUMS ------------------------------------------*/
typedef enum{KOM_IDLE,KOM_HEADER,KOM_RCV,KOM_APP} kom_state_T;

typedef enum{KOM_RESP_OK,KOM_RESP_NOK} kom_resp_T;
/*------------------------------- STRUCT AND UNIONS ------------------------------------*/

/*======================================================================================*/
/*                    ####### LOCAL FUNCTIONS PROTOTYPES #######                        */
/*======================================================================================*/
static kom_state_T KOM_Idle(void);
static kom_state_T KOM_Header(void);
static kom_state_T KOM_Rcv(void);
static kom_state_T KOM_App(void);
static kom_resp_T KOM_CheckCRC(void);
static kom_resp_T KOM_ParseData(void);
/*======================================================================================*/
/*                         ####### OBJECT DEFINITIONS #######                           */
/*======================================================================================*/
/*--------------------------------- EXPORTED OBJECTS -----------------------------------*/

/*---------------------------------- LOCAL OBJECTS -------------------------------------*/
kom_state_T kom_state;

volatile uint8_t kom_rcv_buf[KOM_RCV_BUF_SIZE];
volatile uint8_t kom_frame_cnt=0;
volatile uint8_t kom_frame_len=0;


uint8_t kom_timer;
/*======================================================================================*/
/*                  ####### EXPORTED FUNCTIONS DEFINITIONS #######                      */
/*======================================================================================*/

void KOM_Init(void)
{
	Timer_Register(&kom_timer,KOM_FRAME_TIMEOUT_MS,timerOpt_AUTOSTOP);
	Timer_Stop(&kom_timer);

	kom_state = KOM_IDLE;
}
void KOM_Main(void)
{
	switch(kom_state){
	case KOM_IDLE:
		kom_state = KOM_Idle();
		break;
	case KOM_HEADER:
		kom_state = KOM_Header();
		break;
	case KOM_RCV:
		kom_state = KOM_Rcv();
		break;
	case KOM_APP:
		kom_state = KOM_App();
		break;
	default:
		break;
	}

}

/*======================================================================================*/
/*                   ####### LOCAL FUNCTIONS DEFINITIONS #######                        */
/*======================================================================================*/

static kom_state_T KOM_Idle(void)
{
	if(kom_frame_cnt > 1){
		return KOM_HEADER;
	}
	return KOM_IDLE;
}
static kom_state_T KOM_Header(void)
{
	if(kom_rcv_buf[0] == KOM_PREFIX){
		kom_frame_len = kom_rcv_buf[1];
		Timer_Reset(&kom_timer);
		return KOM_RCV;
	}
	KOM_BufClr();
	KOM_SendNAK();
	return KOM_IDLE;

}
static kom_state_T KOM_Rcv(void)
{
	if(Timer_Check(&kom_timer) == 1){
		KOM_BufClr();
		KOM_SendNAK();
		return KOM_IDLE;
	}
	else if(kom_frame_cnt >= kom_frame_len-1){
		if(KOM_CheckCRC() == KOM_RESP_OK){
			return KOM_APP;
		}
		else{
			KOM_BufClr();
			KOM_SendNAK();
			return KOM_IDLE;
		}
	}
	return KOM_RCV;
}

static kom_state_T KOM_App(void)
{
	if(KOM_ParseData() == KOM_RESP_OK){
		KOM_SendAK();
	}
	else{
		KOM_SendNAK();
	}
	KOM_BufClr();

	return KOM_IDLE;
}

static kom_resp_T KOM_CheckCRC(void)
{
	return KOM_RESP_OK;
	return KOM_RESP_NOK;
}

void KOM_ConfigRcvHandler(kom_config_data_T* config)
{
}

static kom_resp_T KOM_ParseData(void)
{
	kom_record_data_T data;

	switch(kom_rcv_buf[3]){
	case 'D': // DATA
		{
			for(uint8_t i=0; i<kom_frame_len-3;i++){

			//	&(data+i) = kom_rcv_buf[i+3];
			}
			//data.flow = ((uint16_t)kom_rcv_buf[3]<<8) + kom_rcv_buf[4];
			break;
		}
	case 'F': // CONFIG
	{
		KOM_ConfigRcvHandler((kom_config_data_T*)&kom_rcv_buf[4]);
		// wpisujesz z bufora do struktury
		break;
	}
	case 'C': // COMMAND
		break;
	default:
		return KOM_RESP_NOK;
		break;
	}
	return KOM_RESP_OK;

}
void Interrupt(void)
{
	if(kom_frame_cnt < KOM_RCV_BUF_SIZE-1){
		kom_rcv_buf[kom_frame_cnt++] = RCV();
	}
}
/**
 * @} end of group kom.c
 */
