/*
 * komunikacja.h
 *
 *  Created on: 12 wrz 2016
 *      Author: Marcin
 */

#ifndef KOMUNIKACJA_H_
#define KOMUNIKACJA_H_

typedef struct{
	uint16_t timestamp;
	uint16_t moist;
	uint16_t flow;
	int16_t  temp_z;
	int16_t  temp_in;
	int16_t  temp_out;
	uint8_t hum_in;
	uint8_t hum_out;
}debugkom_record_T;


#define DEBUG_BUF_LENGTH 256

void Debug_Init(void);

void Debug_Main(void);
void PC_Debug(volatile char *s);


#endif /* KOMUNIKACJA_H_ */
