/*
 * led.h
 *
 *  Created on: 28 kwi 2016
 *      Author: Marcin
 */

#ifndef LED_H_
#define LED_H_



typedef enum {LED_SLOW,LED_MEDIUM,LED_FAST,LED_ONEBLINKS,LED_THREEBLINKS,LED_FIVEBLINKS,LED_ON,LED_OFF}blinkLedE;
blinkLedE blinkingFreq;


void LED_Green(blinkLedE blinkingFreq);
void LED_Red(blinkLedE blinkingFreq);
void LED_Blue(blinkLedE blinkingFreq);
void LED_Init(void);


#endif /* LED_H_ */
