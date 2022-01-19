/*!
 * \file      subghz_phy_app.c
 *
 * \brief     Ping-Pong implementation
 *
 * \copyright Revised BSD License, see section \ref LICENSE.
 *
 * \code
 *                ______                              _
 *               / _____)             _              | |
 *              ( (____  _____ ____ _| |_ _____  ____| |__
 *               \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 *               _____) ) ____| | | || |_| ____( (___| | | |
 *              (______/|_____)_|_|_| \__)_____)\____)_| |_|
 *              (C)2013-2017 Semtech
 *
 * \endcode
 *
 * \author    Miguel Luis ( Semtech )
 *
 * \author    Gregory Cristian ( Semtech )
 */
/**
  ******************************************************************************
  *
  *          Portions COPYRIGHT 2020 STMicroelectronics
  *
  * @file    subghz_phy_app.c
  * @author  MCD Application Team
  * @brief   Application of the SubGHz_Phy Middleware
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "platform.h"
#include "sys_app.h"
#include "subghz_phy_app.h"
#include "radio.h"
#include "app_version.h"

/* USER CODE BEGIN Includes */
#include "utilities_def.h"
#include "stm32_seq.h"
#include "stm32_timer.h"
#include "main.h"
#include "stdio.h"

/* USER CODE END Includes */

/* External variables ---------------------------------------------------------*/
/* USER CODE BEGIN EV */
#define TRANSMITTER //RECEIVER //
#define BUFFER_SIZE 	10 /* define payload size.  Edit this in the subghz_phy_app.h */
extern UART_HandleTypeDef huart2;
/* USER CODE END EV */

/* Private typedef -----------------------------------------------------------*/

/* USER CODE BEGIN PTD */
static UTIL_TIMER_Object_t timerTransmit;   //[JT]
static UTIL_TIMER_Object_t timerReceive;    //[JT]
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define TRANSMIT_PERIOD_MS 2000  /* set Tx period of timer to 2 seconds */     //[JT]
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* Radio events function pointer */
static RadioEvents_t RadioEvents;
/* USER CODE BEGIN PV */
uint16_t BufferSize = BUFFER_SIZE;   //[JT]
uint8_t Buffer[BUFFER_SIZE];         //[JT]
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
static void Rx_Process(void);  //[JT]
//static void Tx_Process(void);  //[JT]
static void TransmitPacket(void *context);    //[JT]
/* USER CODE END PFP */

/* Exported functions ---------------------------------------------------------*/
void SubghzApp_Init(void)
{
  /* USER CODE BEGIN SubghzApp_Init_1 */
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
  APP_LOG(TS_ON, VLEVEL_L, "******TRANSMITTER******\n\r");    
  Radio.SetTxConfig(MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                      LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                      LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                      true, 0, 0, LORA_IQ_INVERSION_ON, TX_TIMEOUT_VALUE);  
#endif
#ifdef RECEIVER    
  APP_LOG(TS_ON, VLEVEL_L, "******RECEIVER******\n\r");    
  Radio.SetRxConfig(MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
                    LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
                    LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
                    0, true, 0, 0, LORA_IQ_INVERSION_ON, true);
#endif
  
  Radio.SetMaxPayloadLength(MODEM_LORA, BUFFER_SIZE);  
  Radio.SetChannel(RF_FREQUENCY);
   
#ifdef TRANSMITTER    
  Buffer[0] = 'S';
  Buffer[1] = 'T';
  Buffer[2] = 'M';
  Buffer[3] = '3';
  Buffer[4] = '2';
  Buffer[5] = 'W';
  Buffer[6] = 'L';
  Buffer[7] = '_';
  Buffer[8] = 'T';
  Buffer[9] = 'X';  
  
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
//[JT]
static void Rx_Process(void)
{
  APP_LOG(TS_OFF, VLEVEL_L, "\n\r");    
  APP_LOG(TS_ON, VLEVEL_L, "Radio Rx\n\r");     
  Radio.Rx(0);
}

//static void Tx_Process(void)
//{
//  APP_LOG(TS_ON, VLEVEL_L, "Radio Send\n\r");      
//  Radio.Send(Buffer, BufferSize);
//}

static void TransmitPacket(void *context)
{
  Radio.Send(Buffer, BufferSize);
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
  APP_LOG(TS_ON, VLEVEL_L, "OnRxDone\n\r");
  APP_LOG(TS_ON, VLEVEL_L,  "RssiValue=%d dBm, SnrValue=%d\n\r", rssi, LoraSnr_FskCfo);
  
  Radio.Sleep();
  BufferSize = size;
  memcpy(Buffer, payload, BufferSize);
  
  APP_LOG(TS_OFF, VLEVEL_L,  "%s \n\r", Buffer);
  
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

