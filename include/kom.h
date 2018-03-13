/*
 * kom.h
 *
 *  Created on: 16 lis 2016
 *      Author: Marcin
 */

#ifndef KOM_H_
#define KOM_H_


typedef struct{
	uint8_t  hour;
	uint8_t  minute;
	uint16_t moisture;
	int16_t  tempWheat;
	int16_t  tempInside;
	int16_t  tempOutside;
	uint16_t humInside;
	uint16_t humOutside;
}recordS;

recordS recordAct;


void Kom_SendMeasures(void);
void Kom_SendRecord(void);
void Kom_RecvHandler(uint8_t* buf);


#endif /* KOM_H_ */
