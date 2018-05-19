/*=======================================================================================*
 * @file    thmeas.h
 * @author  Marcin Badzioch
 * @version 1.0
 * @date    02-09-2017
 * @brief   Header file for temperature humidity measurement module
 *
 *          This file contains API of air temperature and humidity measurement module
 *======================================================================================*/
/*----------------------- DEFINE TO PREVENT RECURSIVE INCLUSION ------------------------*/


#ifndef THMEAS_H_
#define THMEAS_H_
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup Thmeas Description
 * @{
 * @brief Thversal measurement module
 */

/*======================================================================================*/
/*                       ####### PREPROCESSOR DIRECTIVES #######                        */
/*======================================================================================*/
/*-------------------------------- INCLUDE DIRECTIVES ----------------------------------*/
#include "stdio.h"
/*----------------------------- LOCAL OBJECT-LIKE MACROS -------------------------------*/

/*---------------------------- LOCAL FUNCTION-LIKE MACROS ------------------------------*/

/*======================================================================================*/
/*                     ####### EXPORTED TYPE DECLARATIONS #######                       */
/*======================================================================================*/
/*---------------------------- ALL TYPE DECLARATIONS -----------------------------------*/

/*------------------------------------- ENUMS ------------------------------------------*/
typedef enum
{
	TH_OK = 0,
	TH_TIMEOUT,
	TH_NO_SENS_IN,
	TH_NO_SENS_OUT,
} thmeas_resp_E;
/*------------------------------- STRUCT AND THONS ------------------------------------*/
typedef struct{
	uint16_t humIn,humOut;
	int16_t tempIn,tempOut;
	uint16_t tempRaw,humRaw;
	uint8_t sensInPresentFlag,sensOutPresentFlag;
}thmeas_data_T;
/*======================================================================================*/
/*                    ####### EXPORTED OBJECT DECLARATIONS #######                      */
/*======================================================================================*/

/*======================================================================================*/
/*                   ####### EXPORTED FUNCTIONS PROTOTYPES #######                      */
/*======================================================================================*/
thmeas_resp_E ThMeas_Init(void);
thmeas_resp_E ThMeas_Get(void);
/*======================================================================================*/
/*                          ####### INLINE FUNCTIONS #######                            */
/*======================================================================================*/

/**
 * @} end of group Thmeas
 */

#ifdef __cplusplus
}
#endif
#endif /* THMEAS_H_ */




