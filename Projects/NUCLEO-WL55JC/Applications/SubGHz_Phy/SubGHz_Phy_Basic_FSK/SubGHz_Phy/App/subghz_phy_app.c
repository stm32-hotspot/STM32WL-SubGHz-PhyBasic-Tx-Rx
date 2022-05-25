/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    subghz_phy_app.c
  * @author  MCD Application Team
  * @brief   Application of the SubGHz_Phy Middleware
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "platform.h"
#include "sys_app.h"
#include "subghz_phy_app.h"
#include "radio.h"

/* USER CODE BEGIN Includes */
#include "utilities_def.h"
#include "stm32_seq.h"
#include "stm32_timer.h"
#include "main.h"
#include "stdio.h"
/* USER CODE END Includes */

/* External variables ---------------------------------------------------------*/
/* USER CODE BEGIN EV */
#define RECEIVER
//#define TRANSMITTER

extern UART_HandleTypeDef huart2;
/* USER CODE END EV */

/* Private typedef -----------------------------------------------------------*/

/* USER CODE BEGIN PTD */
static UTIL_TIMER_Object_t timerTransmit;   
static UTIL_TIMER_Object_t timerReceive;    
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define TRANSMIT_PERIOD_MS 2000  /* set Tx period of timer to 2 seconds */    
#define RX_CONTINUOUS_ON      1
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* Radio events function pointer */
static RadioEvents_t RadioEvents;
/* USER CODE BEGIN PV */

uint8_t TxBuffer[PAYLOAD_LEN];         
uint8_t RxBuffer[PAYLOAD_LEN];        

static uint8_t syncword[3] = { 0xC1, 0x94, 0xC1};

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/*!
 * @brief Function to be executed on Radio Tx Done event
 */
static void OnTxDone(void);

/**
  * @brief Function to be executed on Radio Rx Done event
  * @param  payload ptr of buffer received
  * @param  size buffer size
  * @param  rssi
  * @param  LoraSnr_FskCfo
  */
static void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t LoraSnr_FskCfo);

/**
  * @brief Function executed on Radio Tx Timeout event
  */
static void OnTxTimeout(void);

/**
  * @brief Function executed on Radio Rx Timeout event
  */
static void OnRxTimeout(void);

/**
  * @brief Function executed on Radio Rx Error event
  */
static void OnRxError(void);

/* USER CODE BEGIN PFP */
static void Rx_Process(void);  
static void TransmitPacket(void *context);    

/* USER CODE END PFP */

/* Exported functions ---------------------------------------------------------*/
void SubghzApp_Init(void)
{
  /* USER CODE BEGIN SubghzApp_Init_1 */
#ifdef TRANSMITTER
  TxConfigGeneric_t TxConfig;
#endif

#ifdef RECEIVER
  RxConfigGeneric_t RxConfig = {0};
#endif  
  
  /* USER CODE END SubghzApp_Init_1 */

  /* Radio initialization */
  RadioEvents.TxDone = OnTxDone;
  RadioEvents.RxDone = OnRxDone;
  RadioEvents.TxTimeout = OnTxTimeout;
  RadioEvents.RxTimeout = OnRxTimeout;
  RadioEvents.RxError = OnRxError;

  Radio.Init(&RadioEvents);

  /* USER CODE BEGIN SubghzApp_Init_2 */
#ifdef TRANSMITTER 
  
   APP_LOG(TS_ON, VLEVEL_L, "******TRANSMITTER FSK GENERIC******\n\r");
   
  /* fsk modulation */
  TxConfig.fsk.ModulationShaping = RADIO_FSK_MOD_SHAPING_G_BT_05;
  TxConfig.fsk.FrequencyDeviation = FSK_FDEV;
  TxConfig.fsk.BitRate = FSK_DATARATE; 
  TxConfig.fsk.PreambleLen = FSK_PREAMBLE_LENGTH;  
  TxConfig.fsk.SyncWordLength = sizeof(syncword);  
  TxConfig.fsk.SyncWord = syncword;  
  TxConfig.fsk.whiteSeed =  0x0000;  
  TxConfig.fsk.HeaderType  = RADIO_FSK_PACKET_FIXED_LENGTH; 
  TxConfig.fsk.CrcLength = RADIO_FSK_CRC_OFF;     
  TxConfig.fsk.Whitening = RADIO_FSK_DC_FREE_OFF;
  if (0UL != Radio.RadioSetTxGenericConfig(GENERIC_FSK, &TxConfig, TX_OUTPUT_POWER, TX_TIMEOUT_VALUE))
  {
    while (1);
  }

#endif  

#ifdef RECEIVER  

  APP_LOG(TS_ON, VLEVEL_L, "******RECEIVER FSK GENERIC******\n\r"); 
  /* RX Continuous */
  RxConfig.fsk.ModulationShaping = RADIO_FSK_MOD_SHAPING_G_BT_05;
  RxConfig.fsk.Bandwidth = FSK_BANDWIDTH;
  RxConfig.fsk.BitRate = FSK_DATARATE; 
  RxConfig.fsk.PreambleLen = FSK_PREAMBLE_LENGTH; 
  RxConfig.fsk.SyncWordLength = sizeof(syncword); 
  RxConfig.fsk.PreambleMinDetect = RADIO_FSK_PREAMBLE_DETECTOR_16_BITS;
  RxConfig.fsk.SyncWord = syncword; 
  RxConfig.fsk.whiteSeed = 0x0000;  
  RxConfig.fsk.LengthMode = RADIO_FSK_PACKET_FIXED_LENGTH; 
  RxConfig.fsk.CrcLength = RADIO_FSK_CRC_OFF;      
  RxConfig.fsk.Whitening = RADIO_FSK_DC_FREE_OFF;
  RxConfig.fsk.MaxPayloadLength = PAYLOAD_LEN;
  RxConfig.fsk.StopTimerOnPreambleDetect = 0;
  RxConfig.fsk.AddrComp = RADIO_FSK_ADDRESSCOMP_FILT_OFF;
  if (0UL != Radio.RadioSetRxGenericConfig(GENERIC_FSK, &RxConfig, RX_CONTINUOUS_ON, 0))
  {
    while (1);
  }
   
#endif  

  Radio.SetChannel(RF_FREQUENCY);
  

#ifdef TRANSMITTER
  TxBuffer[0] = 'S';
  TxBuffer[1] = 'T';
  TxBuffer[2] = 'M';
  TxBuffer[3] = '3';
  TxBuffer[4] = '2';
  TxBuffer[5] = 'W';
  TxBuffer[6] = 'L';
  TxBuffer[7] = '_';
  TxBuffer[8] = 'T';
  TxBuffer[9] = 'X';

  /* Add delay between TX and RX =
  time Busy_signal is ON in RX opening window */
  HAL_Delay(Radio.GetWakeupTime() + TCXO_WORKAROUND_TIME_MARGIN);

  UTIL_TIMER_Create(&timerTransmit, 0xFFFFFFFFU, UTIL_TIMER_ONESHOT, TransmitPacket, NULL);
  UTIL_TIMER_SetPeriod(&timerTransmit, TRANSMIT_PERIOD_MS);
  UTIL_TIMER_Start(&timerTransmit);  // start transmitting packets every 2s
#endif

#ifdef RECEIVER

  UTIL_SEQ_RegTask((1 << CFG_SEQ_Task_Rx_Process), UTIL_SEQ_RFU, Rx_Process);
  UTIL_SEQ_SetTask((1 << CFG_SEQ_Task_Rx_Process), CFG_SEQ_Prio_0);

#endif
  /* USER CODE END SubghzApp_Init_2 */
}

/* USER CODE BEGIN EF */
static void Rx_Process(void)
{
  APP_LOG(TS_OFF, VLEVEL_L, "\n\r");
  APP_LOG(TS_ON, VLEVEL_L, "Radio Rx\n\r");
  Radio.Rx(0);
}

static void TransmitPacket(void *context)
{
   Radio.Send(TxBuffer, PAYLOAD_LEN);
}
/* USER CODE END EF */

/* Private functions ---------------------------------------------------------*/
static void OnTxDone(void)
{
  /* USER CODE BEGIN OnTxDone */
  APP_LOG(TS_ON, VLEVEL_L, "OnTxDone\n\r");
  UTIL_TIMER_Start(&timerTransmit);  //Transmit packet in 2s
  /* USER CODE END OnTxDone */
}

static void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t LoraSnr_FskCfo)
{
  /* USER CODE BEGIN OnRxDone */
  uint16_t BufferSize; 
  uint8_t i; 
  
  APP_LOG(TS_ON, VLEVEL_L, "OnRxDone\n\r");
  APP_LOG(TS_ON, VLEVEL_L,  "RssiValue=%d dBm, SnrValue=%d\n\r", rssi, LoraSnr_FskCfo);
  
  Radio.Sleep();
  BufferSize = size;
  memcpy(RxBuffer, payload, BufferSize);

  APP_LOG(TS_OFF, VLEVEL_L,  "Payload: %s \n", RxBuffer);
  for (i = 0; i < BufferSize; i++) 
  {
    APP_LOG(TS_OFF, VLEVEL_L,  "%02X ", RxBuffer[i]);
  }
  APP_LOG(TS_OFF, VLEVEL_L,  "\n");
  
  UTIL_SEQ_SetTask((1 << CFG_SEQ_Task_Rx_Process), CFG_SEQ_Prio_0); 
  /* USER CODE END OnRxDone */
}

static void OnTxTimeout(void)
{
  /* USER CODE BEGIN OnTxTimeout */
  /* USER CODE END OnTxTimeout */
}

static void OnRxTimeout(void)
{
  /* USER CODE BEGIN OnRxTimeout */
  /* USER CODE END OnRxTimeout */
}

static void OnRxError(void)
{
  /* USER CODE BEGIN OnRxError */
  /* USER CODE END OnRxError */
}

/* USER CODE BEGIN PrFD */

/* USER CODE END PrFD */
