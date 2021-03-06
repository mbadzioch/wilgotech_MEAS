/*=======================================================================================*
 * @file    unimeas.c
 * @author  Marcin Badzioch
 * @version 1.0
 * @date    02-09-2017
 * @brief   Source file for universal measurement module
 *======================================================================================*/

/**
 * @addtogroup Unimeas Description
 * @{
 * @brief  Universal measurement module
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
#include "main.h"
#include "timer.h"
#include "unimeas.h"
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

/*======================================================================================*/
/*                  ####### EXPORTED FUNCTIONS DEFINITIONS #######                      */
/*======================================================================================*/
/*uniMeasResp UniMeas_Init(void)
{
	return UNI_OK;
}
uniMeasResp UniMeas_Start(void)
{
	return UNI_OK;
}
uniMeasResp UniMeas_Stop(void)
{
	return UNI_OK;
}
uniMeasResp UniMeas_Set(uniMeasSetS* uniMeasSet)
{
	return UNI_OK;
}
uniMeasResp UniMeas_Get(uniMeasGetS* uniMeasGet)
{
	return UNI_OK;
}
/*======================================================================================*/
/*                   ####### LOCAL FUNCTIONS DEFINITIONS #######                        */
/*======================================================================================*/


/**
 * @} end of group Unimeas
 */
