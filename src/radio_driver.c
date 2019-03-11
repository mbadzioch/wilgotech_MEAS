/*=======================================================================================*
 * @file    radio_driver.c
 * @author  Damian Pala
 * @date    XX-XX-20XX
 * @brief   This file contains all implementations for XXX module.
 *======================================================================================*/

/**
 * @addtogroup XXX Description
 * @{
 * @brief Module for... .
 */

/*======================================================================================*/
/*                       ####### PREPROCESSOR DIRECTIVES #######                        */
/*======================================================================================*/
/*-------------------------------- INCLUDE DIRECTIVES ----------------------------------*/
#include <stdint.h>
#include <stdbool.h>

#include "radio_driver.h"
#include "cmsis_device.h"
#include "radio_reg.h"

/*----------------------------- LOCAL OBJECT-LIKE MACROS -------------------------------*/
#define RADIO_SYNC_WORD                             0x46FA

#define RADIO_SPI_PORT                              2
#define RADIO_SPI_SCK_PIN                           10
#define RADIO_SPI_MISO_PIN                          11
#define RADIO_SPI_MOSI_PIN                          12
#define RADIO_SPI_SS_PORT                           1
#define RADIO_SPI_SS_PIN                            2
#define RADIO_IRQ_PORT                              1
#define RADIO_IRQ_PIN                               1

#define RD_RW_BM                                    (1 << 7)
#define RADIO_IDLE_STATE                            RADIO_STANDBY_MODE
#define RADIO_TX_STATE                              TX_ON_bm
#define RADIO_RX_STATE                              RX_ON_bm

#define RADIO_STANDBY_MODE                          0x00
#define RADIO_SLEEP_MODE                            EN_WT
#define RADIO_SENSOR_MODE                           EN_LBD
#define RADIO_READY_MODE                            XT_ON_bm
#define RADIO_TUNE_MODE                             PLL_ON_bm

//#define MODULATION_OOK_4800
//#define MODULATION_OOK_0600
#define MODULATION_GFSK_4800
//#define MODULATION_GFSK_1200

/*---------------------------- LOCAL FUNCTION-LIKE MACROS ------------------------------*/
#define RD_ADRESS_AS_WRITE(address)                 ( RD_RW_BM | (address & 0x7F) )
#define RD_ADRESS_AS_READ(address)                  ( ~RD_RW_BM & (address & 0x7F) )

#ifndef __FILENAME__
#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#endif

#if defined(RADIO_LOG) && defined(DEBUG_TRACE)
  #define LOG_PUTS(text)                                    UTRACE_Printf("%s:%d " text, __FILENAME__, __LINE__);
  #define LOG_PRINTF(text, ...)                             UTRACE_Printf("%s:%d " text, __FILENAME__, __LINE__, __VA_ARGS__)
#endif

#ifndef LOG_PUTS
  #define LOG_PUTS(text)
#endif
#ifndef LOG_PRINTF
  #define LOG_PRINTF(text, ...)
#endif

/*======================================================================================*/
/*                      ####### LOCAL TYPE DECLARATIONS #######                         */
/*======================================================================================*/
/*-------------------------------- OTHER TYPEDEFS --------------------------------------*/

/*------------------------------------- ENUMS ------------------------------------------*/

/*------------------------------- STRUCT AND UNIONS ------------------------------------*/

/*======================================================================================*/
/*                         ####### OBJECT DEFINITIONS #######                           */
/*======================================================================================*/
/*--------------------------------- EXPORTED OBJECTS -----------------------------------*/

/*---------------------------------- LOCAL OBJECTS -------------------------------------*/
static bool IsRadioDriverInitialized = false;

/*======================================================================================*/
/*                    ####### LOCAL FUNCTIONS PROTOTYPES #######                        */
/*======================================================================================*/
static void GpioInit(void);
static void SpiInit(void);
static void RadioInterruptInit(void);
static inline void SpiSendData(uint8_t data);
inline uint8_t SpiReceiveData(void);
static inline void RadioClearFifos(void);
static inline void RadioSelectActive(void);
static inline void RadioSelectInactive(void);
static void RadioSwReset(void);

/*======================================================================================*/
/*                  ####### EXPORTED FUNCTIONS DEFINITIONS #######                      */
/*======================================================================================*/
void RD_Init(void)
{
  IsRadioDriverInitialized = false;

  GpioInit();
  SpiInit();
  RadioInterruptInit();
  RD_RadioConfig();

  IsRadioDriverInitialized = true;
}

void RD_RadioSendWriteTransaction(uint8_t address, uint8_t data)
{
  RadioSelectActive();
  SpiSendData(RD_ADRESS_AS_WRITE(address));
  SpiSendData(data);
  RadioSelectInactive();
}

uint8_t RD_RadioSendReadTransaction(uint8_t address)
{
  RadioSelectActive();
  SpiSendData(RD_ADRESS_AS_READ(address));
  SpiSendData(0xFF);
  RadioSelectInactive();

  return SpiReceiveData();
}

void RD_RadioSendWriteBurst(uint8_t address, uint8_t *dataIn, uint16_t size)
{
  RadioSelectActive();
  SpiSendData(RD_ADRESS_AS_WRITE(address));
  for (uint16_t i = 0; i < size; i++)
  {
    SpiSendData(*(dataIn++));
  }
  RadioSelectInactive();
}

void RD_RadioSendReadBurst(uint8_t address, uint8_t *dataOut, uint16_t size)
{
  RadioSelectActive();
  SpiSendData(RD_ADRESS_AS_READ(address));
  for (uint16_t i = 0; i < size; i++)
  {
    SpiSendData(0xFF);
    *(dataOut++) = SpiReceiveData();
  }
  RadioSelectInactive();
}

void RD_RadioTransmitData(uint8_t *dataIn, uint16_t size)
{
  RD_ClearInterruptsStatus();
  RD_RadioClearTxFifo();
  RD_RadioSendWriteTransaction(TRANSMIT_PACKET_LENGTH_3E_REG, size);
  RD_RadioSendWriteBurst(FIFO_ACCESS_7F_REG, dataIn, size);
  RD_RadioMoveToState(RADIO_STATE_TX);
}

void RD_RadioReceiveData(uint8_t *dataOut, uint16_t size)
{
  RD_RadioSendReadBurst(FIFO_ACCESS_7F_REG, dataOut, size);
}

void RD_ClearInterruptsStatus(void)
{
  (void)RD_RadioSendReadTransaction(INTERRUPT_STATUS1_03_REG);
  (void)RD_RadioSendReadTransaction(INTERRUPT_STATUS2_04_REG);
}

void RD_RadioClearTxFifo(void)
{
  RD_RadioSendWriteTransaction(OPERATING_AND_FNC_CTRL2_08_REG, FIFO_CLR_TX_bm);
  RD_RadioSendWriteTransaction(OPERATING_AND_FNC_CTRL2_08_REG, 0x00);
}

void RD_RadioClearRxFifo(void)
{
  RD_RadioSendWriteTransaction(OPERATING_AND_FNC_CTRL2_08_REG, FIFO_CLR_RX_bm);
  RD_RadioSendWriteTransaction(OPERATING_AND_FNC_CTRL2_08_REG, 0x00);
}

void RD_RadioSleep(void)
{
  RD_ClearInterruptsStatus();
  RadioClearFifos();
  RD_RadioMoveToMode(RADIO_MODE_STANDBY);
}

void RD_ReconfigRadioIfNeeded(void)
{
  if(RD_RadioSendReadTransaction(GPIO0_CONFIGURATION_0B_REG) != GPIO_TX_STATE)
  {
    RD_RadioConfig();
    RD_RadioClearRxFifo();
    RD_RadioClearTxFifo();
    RD_RadioMoveToState(RADIO_STATE_RX);
    LOG_PUTS("Radio reconfigurated!\r\n");
  }
  else
  {
    /* Do nothing */
  }
}

void RD_RadioMoveToMode(RD_RadioMode_T mode)
{
  switch(mode)
  {
    case RADIO_MODE_STANDBY:
    {
      RD_RadioSendWriteTransaction(OPERATING_AND_FNC_CTRL1_07_REG, RADIO_STANDBY_MODE);
      break;
    }
    case RADIO_MODE_SLEEP:
    {
      RD_RadioSendWriteTransaction(OPERATING_AND_FNC_CTRL1_07_REG, RADIO_SLEEP_MODE);
      break;
    }
    case RADIO_MODE_SENSOR:
    {
      RD_RadioSendWriteTransaction(OPERATING_AND_FNC_CTRL1_07_REG, RADIO_SENSOR_MODE);
      break;
    }
    case RADIO_MODE_READY:
    {
      RD_RadioSendWriteTransaction(OPERATING_AND_FNC_CTRL1_07_REG, RADIO_READY_MODE);
      break;
    }
    case RADIO_MODE_TUNE:
    {
      RD_RadioSendWriteTransaction(OPERATING_AND_FNC_CTRL1_07_REG, RADIO_TUNE_MODE);
      break;
    }
    default:
      break;
  }
}

void RD_RadioMoveToState(RD_RadioState_T state)
{
  switch(state)
  {
    case RADIO_STATE_IDLE:
    {
      RD_RadioSendWriteTransaction(OPERATING_AND_FNC_CTRL1_07_REG, RADIO_IDLE_STATE);
      break;
    }
    case RADIO_STATE_RX:
    {
      RD_RadioSendWriteTransaction(OPERATING_AND_FNC_CTRL1_07_REG, RADIO_RX_STATE);
      break;
    }
    case RADIO_STATE_TX:
    {
      RD_RadioSendWriteTransaction(OPERATING_AND_FNC_CTRL1_07_REG, RADIO_TX_STATE);
      break;
    }
    default:
      break;
  }
}

inline void RD_RadioInterruptEnable(void)
{
  if (false == IRQ_IsInCriticalSection())
  {
    NVIC_EnableIRQ(RADIO_NVIC_IRQ_CHANNEL);
  }
}

inline void RD_RadioInterruptDisable(void)
{
  NVIC_DisableIRQ(RADIO_NVIC_IRQ_CHANNEL);
}

inline bool RD_IsRadioDriverInitialized(void)
{
  return IsRadioDriverInitialized;
}

void RD_RadioConfig(void)
{
  RadioSwReset();

  RD_RadioSendWriteTransaction(OPERATING_AND_FNC_CTRL1_07_REG, RADIO_READY_MODE);

  RD_RadioSendWriteTransaction(INTERRUPT_ENABLE1_05_REG, EN_CRC_ERROR_bm | EN_PK_RECEIVED_bm | EN_PK_SENT);
  RD_RadioSendWriteTransaction(INTERRUPT_ENABLE2_06_REG, EN_SYNC_WORD_DETECTED_bm | EN_POR_bm);
  RD_RadioSendWriteTransaction(ADC_CONFIGURATION_0F_REG, 0x00);
  RD_RadioSendWriteTransaction(ADC_SENSOR_AMP_OFFSET_10_REG, 0x00);
  RD_RadioSendWriteTransaction(TEMP_SENSOR_CALIB_12_REG, 0x00);
  RD_RadioSendWriteTransaction(TEMP_VALUE_OFFSET_13_REG, 0x00);

  RadioModem_Config_2();

  RD_RadioSendWriteTransaction(RSSI_THRESHOLD_FOR_CLEAR_CH_INDICATOR_27_REG, 0x96); // RSSI threshold for CCA
  RD_RadioSendWriteTransaction(DATA_ACCESS_CTRL_30_REG, 0x8C); // Enabled packet rx, tx handling, enabled crc = 0x8C
  RD_RadioSendWriteTransaction(HEADER_CTRL1_32_REG, 0x33); // Expected broadcast and compare header 1 & 0
  RD_RadioSendWriteTransaction(HEADER_CTRL2_33_REG, 0x42); // Header 3, 2, 1, 0, Sync 3 & 2, packet length included in header = 0x22
  RD_RadioSendWriteTransaction(PREAMBLE_LEN_34_REG, 0x0A);
  RD_RadioSendWriteTransaction(PREAMBLE_DETECTION_CNTRL_35_REG, 0x22);    // 16 bits = 0x22
  RD_RadioSendWriteTransaction(SYNC_WORD3_36_REG, GET_MSB(RADIO_SYNC_WORD)); // Set arbitrarily
  RD_RadioSendWriteTransaction(SYNC_WORD2_37_REG, GET_LSB(RADIO_SYNC_WORD)); // Set arbitrarily
  RD_RadioSendWriteTransaction(SYNC_WORD1_38_REG, 0x00); // Set arbitrarily
  RD_RadioSendWriteTransaction(SYNC_WORD0_39_REG, 0x00); // Set arbitrarily
  RD_RadioSendWriteTransaction(HEADER3_3A_REG, 0x00);
  RD_RadioSendWriteTransaction(HEADER2_3B_REG, 0x00);
  RD_RadioSendWriteTransaction(HEADER1_3C_REG, 0x00);
  RD_RadioSendWriteTransaction(HEADER0_3D_REG, 0x00);
  RD_RadioSendWriteTransaction(TRANSMIT_PACKET_LENGTH_3E_REG, RADIO_DEFAULT_PACKET_LENGTH);
  RD_RadioSendWriteTransaction(CHECK_HEADER3_3F_REG, 0x00);
  RD_RadioSendWriteTransaction(CHECK_HEADER2_40_REG, 0x00);
  RD_RadioSendWriteTransaction(CHECK_HEADER1_41_REG, 0x00);
  RD_RadioSendWriteTransaction(CHECK_HEADER0_42_REG, 0x00);
  RD_RadioSendWriteTransaction(HEADER3_ENABLE_43_REG, 0x00);
  RD_RadioSendWriteTransaction(HEADER2_ENABLE_44_REG, 0x00);
  RD_RadioSendWriteTransaction(HEADER1_ENABLE_45_REG, 0xFF);
  RD_RadioSendWriteTransaction(HEADER0_ENABLE_46_REG, 0xFF);
  RD_RadioSendWriteTransaction(CHARGE_PUMP_CURRENT_TRIM_OVERR_58_REG, 0x80); // Reserved but set in xls configurator
  RD_RadioSendWriteTransaction(TX_POWER_6D_REG, TXPOW_20DBM);
  RD_RadioSendWriteTransaction(MODULATION_MODE_CTRL1_70_REG, 0x2C); // low data rates, manchester preamble polarity and data inv enable = 0x2C
  RD_RadioSendWriteTransaction(FREQ_OFFSET1_73_REG, 0x00);
  RD_RadioSendWriteTransaction(FREQ_OFFSET2_74_REG, 0x00);

  RD_ClearInterruptsStatus();
}

void RD_SetTxHeader4B(uint8_t *byte)
{
  RD_RadioSendWriteTransaction(HEADER3_3A_REG, byte[3]);
  RD_RadioSendWriteTransaction(HEADER2_3B_REG, byte[2]);
  RD_RadioSendWriteTransaction(HEADER1_3C_REG, byte[1]);
  RD_RadioSendWriteTransaction(HEADER0_3D_REG, byte[0]);
}

void RD_SetRxCheckHeader4B(uint8_t *byte)
{
  RD_RadioSendWriteTransaction(CHECK_HEADER3_3F_REG, byte[3]);
  RD_RadioSendWriteTransaction(CHECK_HEADER2_40_REG, byte[2]);
  RD_RadioSendWriteTransaction(CHECK_HEADER1_41_REG, byte[1]);
  RD_RadioSendWriteTransaction(CHECK_HEADER0_42_REG, byte[0]);
}

void RD_GetRxHeader4B(uint8_t *byte)
{
  byte[3] = RD_RadioSendReadTransaction(RECEIVED_HEADER3_47_REG);
  byte[2] = RD_RadioSendReadTransaction(RECEIVED_HEADER2_48_REG);
  byte[1] = RD_RadioSendReadTransaction(RECEIVED_HEADER1_49_REG);
  byte[0] = RD_RadioSendReadTransaction(RECEIVED_HEADER0_4A_REG);
}

/*======================================================================================*/
/*                   ####### LOCAL FUNCTIONS DEFINITIONS #######                        */
/*======================================================================================*/
static void GpioInit(void)
{
  GPIO_InitTypeDef GPIO_InitStruct;

  /* Initialize SPI pins */
  GPIO_StructInit(&GPIO_InitStruct);
  GPIO_InitStruct.GPIO_Mode                   = GPIO_Mode_AF;
  GPIO_InitStruct.GPIO_OType                  = GPIO_OType_PP;
  GPIO_InitStruct.GPIO_Pin                    = PIN_MASK(RADIO_SPI_SCK_PIN) | PIN_MASK(RADIO_SPI_MISO_PIN) | PIN_MASK(RADIO_SPI_MOSI_PIN);
  GPIO_InitStruct.GPIO_PuPd                   = GPIO_PuPd_NOPULL;
  GPIO_InitStruct.GPIO_Speed                  = GPIO_Speed_100MHz;

  RCC_AHB1PeriphClockCmd(RCC_PORT_MASKx(RADIO_SPI_PORT), ENABLE);

  GPIO_Init(GPIOx(RADIO_SPI_PORT), &GPIO_InitStruct);

  GPIO_PinAFConfig(GPIOx(RADIO_SPI_PORT), RADIO_SPI_SCK_PIN, GPIO_AF_SPI3);
  GPIO_PinAFConfig(GPIOx(RADIO_SPI_PORT), RADIO_SPI_MISO_PIN, GPIO_AF_SPI3);
  GPIO_PinAFConfig(GPIOx(RADIO_SPI_PORT), RADIO_SPI_MOSI_PIN, GPIO_AF_SPI3);

  /* Initialize SPI SS pin */
  GPIO_StructInit(&GPIO_InitStruct);
  GPIO_InitStruct.GPIO_Mode                   = GPIO_Mode_OUT;
  GPIO_InitStruct.GPIO_OType                  = GPIO_OType_PP;
  GPIO_InitStruct.GPIO_Pin                    = PIN_MASK(RADIO_SPI_SS_PIN);
  GPIO_InitStruct.GPIO_PuPd                   = GPIO_PuPd_UP;
  GPIO_InitStruct.GPIO_Speed                  = GPIO_Speed_100MHz;

  RCC_AHB1PeriphClockCmd(RCC_PORT_MASKx(RADIO_SPI_SS_PORT), ENABLE);

  GPIO_SetBits(GPIOx(RADIO_SPI_SS_PORT), PIN_MASK(RADIO_SPI_SS_PIN));

  GPIO_Init(GPIOx(RADIO_SPI_SS_PORT), &GPIO_InitStruct);

  /* Initialize radio IRQ pin */
  GPIO_StructInit(&GPIO_InitStruct);
  GPIO_InitStruct.GPIO_Mode                   = GPIO_Mode_IN;
  GPIO_InitStruct.GPIO_OType                  = GPIO_OType_PP;
  GPIO_InitStruct.GPIO_Pin                    = PIN_MASK(RADIO_IRQ_PIN);
  GPIO_InitStruct.GPIO_PuPd                   = GPIO_PuPd_UP;
  GPIO_InitStruct.GPIO_Speed                  = GPIO_Speed_100MHz;

  RCC_AHB1PeriphClockCmd(RCC_PORT_MASKx(RADIO_IRQ_PORT), ENABLE);

  GPIO_Init(GPIOx(RADIO_IRQ_PORT), &GPIO_InitStruct);
}

static void SpiInit(void)
{
  SPI_InitTypeDef SPI_InitStruct;

  SPI_StructInit(&SPI_InitStruct);
  SPI_InitStruct.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
  SPI_InitStruct.SPI_Mode = SPI_Mode_Master;
  SPI_InitStruct.SPI_DataSize = SPI_DataSize_8b;
  SPI_InitStruct.SPI_CPOL = SPI_CPOL_Low;
  SPI_InitStruct.SPI_CPHA = SPI_CPHA_1Edge;
  SPI_InitStruct.SPI_NSS = SPI_NSS_Soft;
  SPI_InitStruct.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_16; // Configure SPI on 42 / 16 = 2,625 MHz
  SPI_InitStruct.SPI_FirstBit = SPI_FirstBit_MSB;

  RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI3, ENABLE);

  SPI_Init(SPI3, &SPI_InitStruct);

  SPI_Cmd(SPI3, ENABLE);
}

static void RadioInterruptInit(void)
{
  EXTI_InitTypeDef EXTI_InitStruct;
  NVIC_InitTypeDef NVIC_InitStruct;

  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

  SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOB, EXTI_PinSource1);

  EXTI_StructInit(&EXTI_InitStruct);
  EXTI_InitStruct.EXTI_Line = EXTI_Line1; // PB1 is connected to EXTI1
  EXTI_InitStruct.EXTI_LineCmd = ENABLE;
  EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Falling;
  EXTI_Init(&EXTI_InitStruct);

  NVIC_InitStruct.NVIC_IRQChannel = RADIO_NVIC_IRQ_CHANNEL;
  NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = RADIO_IRQ_PREEMPTION_PRIORITY;
  NVIC_InitStruct.NVIC_IRQChannelSubPriority = RADIO_IRQ_SUB_PRIORITY;
  NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;

  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
  NVIC_Init(&NVIC_InitStruct);
}

static inline void RadioSelectActive(void)
{
  GPIO_ResetBits(GPIOx(RADIO_SPI_SS_PORT), PIN_MASK(RADIO_SPI_SS_PIN));
}

static inline void RadioSelectInactive(void)
{
  GPIO_SetBits(GPIOx(RADIO_SPI_SS_PORT), PIN_MASK(RADIO_SPI_SS_PIN));
}

static inline void SpiSendData(uint8_t data)
{
  SPI3->DR = data;
  while( !(SPI3->SR & SPI_I2S_FLAG_TXE) );
  while( !(SPI3->SR & SPI_I2S_FLAG_RXNE) );
  while( SPI3->SR & SPI_I2S_FLAG_BSY );
  data = SPI3->DR;
}

inline uint8_t SpiReceiveData(void)
{
  return SPI3->DR;
}

static inline void RadioClearFifos(void)
{
  RD_RadioSendWriteTransaction(OPERATING_AND_FNC_CTRL2_08_REG, FIFO_CLR_TX_bm | FIFO_CLR_RX_bm);
  RD_RadioSendWriteTransaction(OPERATING_AND_FNC_CTRL2_08_REG, 0x00);
}

static void RadioSwReset(void)
{
  RD_RadioInterruptDisable();

  BusyDelay_Ms(1);
  RD_RadioSendWriteTransaction(OPERATING_AND_FNC_CTRL1_07_REG, SW_RES);
  BusyDelay_Ms(60);

  RD_RadioInterruptEnable();
}

/**
 * @}
 */
