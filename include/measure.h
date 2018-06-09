
/*=======================================================================================*
 * @file    measure.h
 * @author  Marcin Badzioch
 * @version 1.0
 * @date    19-05-2018
 * @brief   Header file for universal measurement module
 *
 *          This file contains API of universal measurement module
 *======================================================================================*/
/*----------------------- DEFINE TO PREVENT RECURSIVE INCLUSION ------------------------*/

#ifndef MEASURE_H_
#define MEASURE_H_

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

typedef enum {MEAS_IDLE,MEAS_FLOW,MEAS_MOIST,MEAS_FLAPCLOSE,MEAS_FLAPOPEN,MEAS_ERROR
				} measure_state_T;
/*------------------------------- STRUCT AND UNIONS ------------------------------------*/
typedef struct{
	uint8_t idle_flag;
	uint8_t flow_rec_num;
	uint8_t moist_rec_num;
}measure_set_T;

typedef struct{
	uint16_t timestamp;
	uint16_t moist;
	uint16_t flow;
	int16_t  temp_z;
	int16_t  temp_in;
	int16_t  temp_out;
	uint8_t hum_in;
	uint8_t hum_out;
	uint16_t phase;
	uint16_t amp_r;
	uint16_t amp_c;
}measure_record_T;
/*======================================================================================*/
/*                    ####### EXPORTED OBJECT DECLARATIONS #######                      */
/*======================================================================================*/

/*======================================================================================*/
/*                   ####### EXPORTED FUNCTIONS PROTOTYPES #######                      */
/*======================================================================================*/
void Measure_Init(void);
void Measure_Main(void);
void Measure_Set(measure_set_T measure_set_S);
void Measure_CallibSequence(uint8_t mode);
/*======================================================================================*/
/*                          ####### INLINE FUNCTIONS #######                            */
/*======================================================================================*/

/**
 * @} end of group Measure
 */

#ifdef __cplusplus
}
#endif
#endif /* MEASURE_H_ */
