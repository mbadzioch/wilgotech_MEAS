/*=======================================================================================*
 * @file    unimeas.h
 * @author  Marcin Badzioch
 * @version 1.0
 * @date    02-09-2017
 * @brief   Header file for universal measurement module
 *
 *          This file contains API of universal measurement module
 *======================================================================================*/
/*----------------------- DEFINE TO PREVENT RECURSIVE INCLUSION ------------------------*/


#ifndef UNIMEAS_H_
#define UNIMEAS_H_
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup Unimeas Description
 * @{
 * @brief Universal measurement module
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
/*typedef enum{
	UNI_AVG = 0x01,
	UNI_SET_DONE = 0x80
}uniMeasOptE;

typedef enum{
	UNI_OK = 0,
	UNI_TIMEOUT,
	UNI_NOSET,
	UNI_ERR2,
}uniMeasResp;*/
/*------------------------------- STRUCT AND UNIONS ------------------------------------*/
/*typedef struct{
	uint16_t period;
	uniMeasOptE uniMeasOpt;
}uniMeasSetS;

typedef struct{
	double measVal;
}uniMeasGetS;*/
/*======================================================================================*/
/*                    ####### EXPORTED OBJECT DECLARATIONS #######                      */
/*======================================================================================*/

/*======================================================================================*/
/*                   ####### EXPORTED FUNCTIONS PROTOTYPES #######                      */
/*======================================================================================*/
/*uniMeasResp UniMeas_Init(void);
uniMeasResp UniMeas_Start(void);
uniMeasResp UniMeas_Stop(void);
uniMeasResp UniMeas_Set(uniMeasSetS* uniMeasSet);
uniMeasResp UniMeas_Get(uniMeasGetS* uniMeasGet);*/
/*======================================================================================*/
/*                          ####### INLINE FUNCTIONS #######                            */
/*======================================================================================*/

/**
 * @} end of group Unimeas
 */

#ifdef __cplusplus
}
#endif
#endif /* UNIMEAS_H_ */




