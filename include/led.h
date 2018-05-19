/*
 * led.h
 *
 *  Created on: 28 kwi 2016
 *      Author: Marcin
 */

#ifndef LED_H_
#define LED_H_



typedef enum {SLOW,MEDIUM,FAST,ONEBLINKS,THREEBLINKS,FIVEBLINKS,ON,OFF}blinkLedE;
blinkLedE blinkingFreq;


void LED_Green(blinkLedE blinkingFreq);
void LED_Red(blinkLedE blinkingFreq);
void LED_Blue(blinkLedE blinkingFreq);
void LED_Init(void);


#endif /* LED_H_ */
