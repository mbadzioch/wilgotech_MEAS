/*
 * sinus.c
 *
 *  Created on: 28 sty 2016
 *      Author: Marcin
 */


#include <stdio.h>
#include <stdlib.h>
#include <stm32f30x_conf.h>
#include"sinus.h"

const uint16_t Sine12bit[64] = {
		2048,2248,2447,2642,2831,3013,3185,3346,3495,3630,3750,3853,3939,4007,4056,4085,4095,4085,4056,4007,3939,3853,3750,3630,3495,3346,3185,3013,2831,2642,2447,2248,
		2048,1847,1648,1453,1264,1082,910,749,600,465,345,242,156,88,39,10,0,10,39,88,156,242,345,465,600,749,910,1082,1264,1453,1648,1847};

/*
 * DEF:
 */
static void TIM_Config(void);
void DAC_Config(void);
void PIN_Config(void);
/*
 * EXPORTED:
 */
void SIN_Init(void)
{
	PIN_Config();
	DAC_Config();
	TIM_Config();
}
void SIN_TurnOn(void)
{
	TIM_Cmd(TIM6, ENABLE);
}
void SIN_TurnOff(void)
{
	TIM_Cmd(TIM6, DISABLE);
}
/*
 * LOCAL:
 */

static void TIM_Config(void)
{
	TIM_TimeBaseInitTypeDef    TIM_TimeBaseStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);

	TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
	TIM_TimeBaseStructure.TIM_Period = 0x14;
	TIM_TimeBaseStructure.TIM_Prescaler = 0x0;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0x0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM6, &TIM_TimeBaseStructure);

	TIM_SelectOutputTrigger(TIM6, TIM_TRGOSource_Update);

	TIM_Cmd(TIM6, ENABLE);
}

void DAC_Config(void)
{
	DAC_InitTypeDef            DAC_InitStructure;
	DMA_InitTypeDef            DMA_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC1, ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

	DAC_DeInit(DAC1);
	DAC_InitStructure.DAC_Trigger = DAC_Trigger_T6_TRGO;
	DAC_InitStructure.DAC_WaveGeneration = DAC_WaveGeneration_None;
	DAC_InitStructure.DAC_Buffer_Switch = DAC_BufferSwitch_Enable;
	DAC_Init(DAC1, DAC_Channel_1, &DAC_InitStructure);

	DAC_Cmd(DAC1, DAC_Channel_1, ENABLE);

	DMA_DeInit(DMA1_Channel3);
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)(&(DAC1->DHR12R1));
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)&Sine12bit;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
	DMA_InitStructure.DMA_BufferSize = 64;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	DMA_Init(DMA1_Channel3, &DMA_InitStructure);

	SYSCFG_DMAChannelRemapConfig(SYSCFG_DMARemap_TIM6DAC1Ch1,ENABLE);

	DMA_Cmd(DMA1_Channel3, ENABLE);

	DAC_DMACmd(DAC1, DAC_Channel_1, ENABLE);
}
void PIN_Config(void)
{
	GPIO_InitTypeDef		   GPIO_InitStructure;

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);

	GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_4;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
}

