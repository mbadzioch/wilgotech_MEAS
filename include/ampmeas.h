/*=======================================================================================*
 * @file    Ampmeas.h
 * @author  Marcin Badzioch
 * @version 1.0
 * @date    02-09-2017
 * @brief   Header file for Amplitude measurement module
 *
 *          This file contains API of Amplitude measurement module
 *======================================================================================*/
/*----------------------- DEFINE TO PREVENT RECURSIVE INCLUSION ------------------------*/


#ifndef AmpMEAS_H_
#define AmpMEAS_H_
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup Ampmeas Description
 * @{
 * @brief Amplitude measurement module
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
typedef enum{
	AMP_AVG = 0x01,
	AMP_SET_DONE = 0x80
}ampMeasOptE;

typedef enum{
	AMP_OK = 0,
	AMP_TIMEOUT,
	AMP_NOSET,
	AMP_ERR2
}ampMeasResp;
/*------------------------------- STRUCT AND UNIONS ------------------------------------*/
typedef struct{
	uint16_t period;
	ampMeasOptE ampMeasOpt;
}ampMeasSetS;

typedef struct{
	double measVal;
}ampMeasGetS;
/*======================================================================================*/
/*                    ####### EXPORTED OBJECT DECLARATIONS #######                      */
/*======================================================================================*/

/*======================================================================================*/
/*                   ####### EXPORTED FUNCTIONS PROTOTYPES #######                      */
/*======================================================================================*/
ampMeasResp AmpMeas_Init(void);
ampMeasResp AmpMeas_Start(void);
ampMeasResp AmpMeas_Stop(void);
ampMeasResp AmpMeas_Set(ampMeasSetS* ampMeasSet);
ampMeasResp AmpMeas_Get(ampMeasGetS* ampMeasGet);
/*======================================================================================*/
/*                          ####### INLINE FUNCTIONS #######                            */
/*======================================================================================*/

/**
 * @} end of group Ampmeas
 */

#ifdef __cplusplus
}
#endif
#endif /* AmpMEAS_H_ */




