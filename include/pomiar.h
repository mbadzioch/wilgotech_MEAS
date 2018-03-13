/*
 * pomiar.h
 *
 *  Created on: 26 kwi 2016
 *      Author: Marcin
 */

#ifndef POMIAR_H_
#define POMIAR_H_

#include <stdio.h>
#include <stdlib.h>
#include <stm32f30x_conf.h>
#include <stm32f30x.h>
#include <string.h>
#include "delay.h"
#include "sinus.h"
#include "rtc.h"
#include "led.h"
#include "main.h"



#define FLOW_MEAS_TIME		16		// In minutes - time between moisture measurements	// Both must be SD_WRITE_INTERVAL multiplication
#define MOISTURE_MEAS_TIME	4		// In minutes - time of moisture measurements > NOT IMPLEMENTED!!!
#define SD_WRITE_INTERVAL	2		// In minutes - time between SD_WRITE
#define SAMPLES_PER_MINUTE  60		// Number of samples per minute

#define SD_UART_SEND_RESP	0 		// Defines if we send response to uart
#define MEAS_PRINT_UART		0		// Defines wheather we send actual measurements to PC via UART

#define FLAP_OPEN_TIMEOUT	30		// In sec

typedef enum {INIT,MEASFLOW,MEASMOIST,FLAPCLOSE,FLAPOPEN}measureStateE;
measureStateE measureState;

typedef enum {OPENED,OPENING,CLOSED,CLOSING}flapStateE;
flapStateE flapState;

typedef enum {OPEN,CLOSE}flapCmdE;
flapCmdE flapCmd;

typedef enum {FLOW,MOISTURE,INFO}dataTypeE;
dataTypeE dataType;

typedef struct{
	uint32_t hms;
	uint16_t faza;
	uint16_t ampliR;
	uint16_t ampliC;
	uint16_t cvdC;
	int16_t  tempZboza;
	int16_t  tempIn;
	int16_t  tempOut;
	uint16_t humIn;
	uint16_t humOut;
}measParamS;
measParamS measBuff[SD_WRITE_INTERVAL*SAMPLES_PER_MINUTE+5];

typedef struct{
	sTimeS	 sTime;
	uint16_t vBat;
	uint16_t tempProc;
}measOtherS;
measOtherS measStartup;


void Measure_Config(void);
void Measure_Sequence(sTimeS* sTime);
void Measure_CallibSequence(sTimeS* sTime,uint8_t mode);
void disk_timerproc();

#endif /* POMIAR_H_ */
