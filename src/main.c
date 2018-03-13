
#include <stdio.h>
#include <stdlib.h>
#include <stm32f30x_conf.h>
#include <stm32f30x.h>
#include <string.h>
#include "delay.h"
#include "pomiar.h"
#include "debugkom.h"
#include "rtc.h"
#include "led.h"
#include "main.h"
#include "timer.h"

/*
 * 	Wykorzystanie TIMERów:
 *
 * 		TIM2 - mmc.c + obsługuje handler timer.c	!! timer 2 - krotnie spowolniono
 * 		TIM3 - led.c
 * 		TIM6 - sinus.c
 */
#define MAIN_TIM_INTERVAL 1000

uint8_t mainTim;

int main(void)
{
	SystemInit();
	DelayInit();
	Debug_Init();
	Measure_Config();
	RTC_Config(&sTime);
	Timer_Init();

	LED_Green(MEDIUM);
	while(1)
	{
		Debug_Main();
//		if(Timer_Check(&mainTim)==1){
//			RTC_GetDateTime(&sTime);
//			sprintf(cBuf,"%.2d-%.2d-%2d %.2d:%.2d:%.2d \n\r",sTime.date,sTime.month,sTime.year,sTime.hours,sTime.minutes,sTime.seconds);
//			PC_Send(cBuf);
//			//Measure_Sequence(&sTime);
//			//Measure_CallibSequence(&sTime,0);
//		}
	}
}


