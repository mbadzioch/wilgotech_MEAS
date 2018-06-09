/*
 * led.c
 *
 *  Created on: 28 kwi 2016
 *      Author: Marcin
 */
#include <stdio.h>
#include <stdlib.h>
#include <stm32f30x_conf.h>
#include <stm32f30x.h>
#include "led.h"


/*
 * TODO: Przerobić na pracę w oparciu o timer SW, ograniczyć funkcje?
 */

TIM_TimeBaseInitTypeDef    TIM_TimeBaseStructure;
GPIO_InitTypeDef		   GPIO_InitStructure;
NVIC_InitTypeDef		   		NVIC_InitStructure;

uint8_t LED_GreenFreq,LED_BlueFreq,LED_RedFreq,LED_GreenBlinkNum,LED_RedBlinkNum,LED_BlueBlinkNum,LED_GBFlag,LED_RBFlag,LED_BBFlag;

void LED_Init(void)
{
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOC, ENABLE);

	GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	//GPIO_SetBits(GPIOA,GPIO_Pin_8);
	GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_9|GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	/*GPIO_SetBits(GPIOC,GPIO_Pin_9);
	GPIO_SetBits(GPIOC,GPIO_Pin_10);*/

	/*
	 * Use this timer and OC to generate LED PWM signals
	 */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

	TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
	TIM_TimeBaseStructure.TIM_Period = 5000;
	TIM_TimeBaseStructure.TIM_Prescaler = 320;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0x0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=4;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=0;
	NVIC_Init(&NVIC_InitStructure);

	TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE);

	TIM_Cmd(TIM3, ENABLE);
}
/*
 *	LED Control functions
 */
void LED_Green(blinkLedE blinkingFreq)
{
	switch(blinkingFreq){
		case LED_OFF:
			LED_GreenFreq = 0;
			GPIO_ResetBits(GPIOA,GPIO_Pin_8);
			break;
		case LED_ON:
			LED_GreenFreq = 0;
			GPIO_SetBits(GPIOA,GPIO_Pin_8);
			break;
		case LED_SLOW:
			LED_GreenFreq = 100;
			break;
		case LED_MEDIUM:
			LED_GreenFreq = 40;
			break;
		case LED_FAST:
			LED_GreenFreq = 4;
			break;
		case LED_ONEBLINKS:
			LED_GBFlag=1;
			LED_GreenFreq = 7;
			LED_GreenBlinkNum = 1;
			break;
		case LED_THREEBLINKS:
			LED_GBFlag=1;
			LED_GreenFreq = 7;
			LED_GreenBlinkNum = 3;
			break;
		case LED_FIVEBLINKS:
			LED_GBFlag=1;
			LED_GreenFreq = 7;
			LED_GreenBlinkNum = 5;
			break;
		default:
			break;
	}
}
void LED_Red(blinkLedE blinkingFreq)
{
	switch(blinkingFreq){
		case LED_OFF:
			LED_RedFreq = 0;
			GPIO_ResetBits(GPIOC,GPIO_Pin_9);
			break;
		case LED_ON:
			LED_RedFreq = 0;
			GPIO_SetBits(GPIOC,GPIO_Pin_9);
			break;
		case LED_SLOW:
			LED_RedFreq = 100;
			break;
		case LED_MEDIUM:
			LED_RedFreq = 40;
			break;
		case LED_FAST:
			LED_RedFreq = 4;
			break;
		case LED_ONEBLINKS:
			LED_RBFlag=1;
			LED_RedFreq = 7;
			LED_RedBlinkNum = 1;
			break;
		case LED_THREEBLINKS:
			LED_RBFlag=1;
			LED_RedFreq = 7;
			LED_RedBlinkNum = 3;
			break;
		case LED_FIVEBLINKS:
			LED_RBFlag=1;
			LED_RedFreq = 7;
			LED_RedBlinkNum = 5;
			break;
		default:
			break;
	}
}
void LED_Blue(blinkLedE blinkingFreq)
{
	switch(blinkingFreq){
		case LED_OFF:
			LED_BlueFreq = 0;
			GPIO_ResetBits(GPIOC,GPIO_Pin_10);
			break;
		case LED_ON:
			LED_BlueFreq = 0;
			GPIO_SetBits(GPIOC,GPIO_Pin_10);
			break;
		case LED_SLOW:
			LED_BlueFreq = 100;
			break;
		case LED_MEDIUM:
			LED_BlueFreq = 40;
			break;
		case LED_FAST:
			LED_BlueFreq = 4;
			break;
		case LED_ONEBLINKS:
			LED_BBFlag=1;
			LED_BlueFreq = 7;
			LED_BlueBlinkNum = 1;
			break;
		case LED_THREEBLINKS:
			LED_BBFlag=1;
			LED_BlueFreq = 7;
			LED_BlueBlinkNum = 3;
			break;
		case LED_FIVEBLINKS:
			LED_BBFlag=1;
			LED_BlueFreq = 7;
			LED_BlueBlinkNum = 5;
			break;
		default:
			break;
	}
}
void LED_Control(void)
{
	static uint8_t gCnt,rCnt,bCnt;
	if(LED_GreenFreq!=0){
		if(++gCnt>(LED_GreenFreq>>1)){
			GPIO_ResetBits(GPIOA,GPIO_Pin_8);
			if(!LED_GreenBlinkNum&&LED_GBFlag){
				LED_GreenFreq=0;
				LED_GBFlag=0;
			}
			if(gCnt==LED_GreenFreq){
				gCnt=0;
				GPIO_SetBits(GPIOA,GPIO_Pin_8);
				LED_GreenBlinkNum--;
			}
		}
	}
	if(LED_RedFreq!=0){
		if(++rCnt>(LED_RedFreq>>1)){
			GPIO_ResetBits(GPIOC,GPIO_Pin_9);
			if(!LED_RedBlinkNum&&LED_RBFlag){
				LED_RedFreq=0;
				LED_RBFlag=0;
			}
			if(rCnt==LED_RedFreq){
				rCnt=0;
				GPIO_SetBits(GPIOC,GPIO_Pin_9);
				LED_RedBlinkNum--;
			}
		}
	}
	if(LED_BlueFreq!=0){
		if(++bCnt>(LED_BlueFreq>>1)){
			GPIO_ResetBits(GPIOC,GPIO_Pin_10);
			if(!LED_BlueBlinkNum&&LED_BBFlag){
				LED_BlueFreq=0;
				LED_BBFlag=0;
			}
			if(bCnt==LED_BlueFreq){
				bCnt=0;
				GPIO_SetBits(GPIOC,GPIO_Pin_10);
				LED_BlueBlinkNum--;
			}
		}
	}
}
void TIM3_IRQHandler()
{
	if(TIM_GetITStatus(TIM3,TIM_IT_Update) != RESET){
		LED_Control();
		TIM_ClearITPendingBit(TIM3,TIM_IT_Update);
	}
}
