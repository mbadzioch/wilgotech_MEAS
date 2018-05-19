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
}ampmeas_opt_E;

typedef enum{
	AMP_OK = 0,
	AMP_TIMEOUT,
	AMP_NOSET,
	AMP_NOTUSED
}ampmeas_resp_E;
/*------------------------------- STRUCT AND UNIONS ------------------------------------*/

/*======================================================================================*/
/*                    ####### EXPORTED OBJECT DECLARATIONS #######                      */
/*======================================================================================*/
typedef struct{

	uint16_t amplitudeCapacitor;
	uint16_t amplitudeResistor;
	uint16_t wheatTemperature;
	uint16_t batteryVoltage;

} ampmeas_filtered_data_T;
/*======================================================================================*/
/*                   ####### EXPORTED FUNCTIONS PROTOTYPES #######                      */
/*======================================================================================*/
ampmeas_resp_E AmpMeas_Init(void);
ampmeas_resp_E AmpMeas_Start(void);
ampmeas_resp_E AmpMeas_Stop(void);
ampmeas_resp_E AmpMeas_Set(void);
ampmeas_resp_E AmpMeas_Get(void);
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




