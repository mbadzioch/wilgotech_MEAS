

#include <stdio.h>
#include <stdlib.h>
#include <stm32f30x_conf.h>
#include <stm32f30x.h>
#include <string.h>
#include "measure.h"
#include "delay.h"
#include "debugkom.h"
#include "rtc.h"
#include "led.h"
#include "timer.h"
#include "memory.h"

/*
 * 	Wykorzystanie TIMERów:
 *
 * 		TIM2 - mmc.c + obsługuje handler timer.c	!! timer 2 - krotnie spowolniono
 * 		TIM3 - led.c
 * 		TIM6 - sinus.c
 */
#define MAIN_TIM_INTERVAL 1000

uint8_t mainTim;


/*
 *
 * Podłaczenie:
 *
 * 	Termometr wewnętrzny - bliżej karty SD
 * 	Termometr zewnętrzny - bliżej trafa
 */

int main(void)
{
	SystemInit();
	DelayInit();
	Timer_Init();
	Debug_Init();
	//Measure_Init();
	RTC_Initialize();
	//Memory_Init();

	while(1)
	{
		Debug_Main();
		Measure_Main();

	}
}


