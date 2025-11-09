/* Includes ------------------------------------------------------------------*/
#include "sensorNode.h"
#include "gpio.h"
#include "main.h"
#include "stm32f303xc.h"
#include <stdint.h>


/* Include peripheral drivers -----------------------------------------------*/
#include "adc.h"
#include "stm32f3xx_ll_adc.h"
#include "tim.h"
#include "gpio.h"
#include "usart.h"
#include "dma.h"
#include "pid.h"
#include "lookup.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static volatile uint32_t tid = 0;
static volatile uint32_t adc_mV = 0;
static uint8_t usart3_SensorNode_Rx_buf[3];

/* Avstand i mm beregnet fra lookup-tabellen (index = mV) */
static volatile uint16_t avstand_mm = 0;
/* Private function prototypes -----------------------------------------------*/


/* Private functions ---------------------------------------------------------*/

void SensorNode_Init(void)
{
  /* Initialisering av perifere enheter for sensorNode ----------------*/
  GPIO_Init();
  TIM4_Init();
  TIM7_Init();
  ADC3_Init();
  USART3_Init();


  // LED 3 angir oppsett som sensorNode
  LL_GPIO_SetOutputPin(LED3_GPIO_PORT, LED3_PIN);
  LL_ADC_StartCalibration(ADC3, LL_ADC_SINGLE_ENDED);
  while (LL_ADC_IsCalibrationOnGoing(ADC3));

  /* Oppstart av perifere enheter for sensorNode ----------------------*/
  // Start USART3 dma recive
  (void)USART_StartRx_DMA(USART3, usart3_SensorNode_Rx_buf, sizeof(usart3_SensorNode_Rx_buf));
  
}

void SensorNode_Loop(void)
{
  
}


/* SensorNode spesifikke metoder ----------------------------------------*/

uint16_t konverter_mV(uint16_t adc_val)
{   
    const uint32_t VREF_mV = 3000U;
    const uint32_t ADC_MAX = 0x1000;
    uint32_t tmp = ((uint32_t)adc_val * VREF_mV)/ADC_MAX;
    return (uint16_t)(tmp);
}





/* Interrupt Callback --------------------------------------------------- */
void ADC3_EndOfConversion_Callback(void)
{
  tid++;
  uint32_t bit_value = LL_ADC_REG_ReadConversionData12(ADC3);
  adc_mV = (bit_value * 3000) / 4096;
  avstand_mm = lookup[(uint16_t)adc_mV];
  LL_ADC_ClearFlag_EOC(ADC3);

  // Send data Til styreNode
  USART_Tx_Tid_Avstand(USART3, tid, avstand_mm);
}



void USART_RxDMAComplete_Callback_SensorNode(USART_TypeDef *USARTx, uint8_t *buf, uint16_t len)
{
  
  if (len >= 3 && buf[0] == 0xAA && buf[2] == 0x55)
  {
    uint8_t start_stop_byte = buf[1];
    if (start_stop_byte == 0x01)
    {
      // Start ADC conversion TRGO tim7.
      
      ADC3_StartConversion_TRGO();
      TIM7_Start_TRGO();
      // Start av PWM 4000 Hz for sette knekkfrekvens(40 HZ) filter.
      TIM4_Start_PWM(); 
    }
    else if (start_stop_byte == 0x00)
    {
      // Stopp
      ADC3_StopConversion_TRGO();
      TIM7_Stopp_TRGO();
      TIM4_Stopp_PWM();
    }
  }
  // Start ny recive USART
  (void)USART_StartRx_DMA(USART3, usart3_SensorNode_Rx_buf, sizeof(usart3_SensorNode_Rx_buf));


}