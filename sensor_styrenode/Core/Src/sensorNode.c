/* Includes ------------------------------------------------------------------*/
#include "sensorNode.h"
#include "gpio.h"
#include "main.h"
#include "stm32f303xc.h"
#include <stdint.h>


/* Include peripheral drivers -----------------------------------------------*/
#include "adc.h"
#include "tim.h"
#include "gpio.h"
#include "usart.h"
#include "dma.h"
#include "pid.h"
#include "lookup.h"

/* Variabler ----------------------------------------------------------------*/
static volatile uint32_t tid = 0;
static volatile uint16_t adc_mV = 0;
static volatile uint16_t avstand_mm = 0;
static volatile uint8_t start_sending_flag = 0;
static uint8_t usart3_SensorNode_Rx_buf[3];


void SensorNode_Init(void)
{
  /* Initialisering av perifere enheter for sensorNode ----------------*/
  ADC3_Init();
  GPIO_Init();
  TIM4_Init();
  TIM7_Init();
  USART3_Init();

  // LED 3 angir oppsett som sensorNode
  LL_GPIO_SetOutputPin(LED3_GPIO_PORT, LED3_PIN);

  /* Oppstart av perifere enheter for sensorNode ----------------------*/
  TIM4_Start_PWM(); // Start av PWM 4000 Hz for sette knekkfrekvens(40 HZ) filter.
  ADC3_Calibrate(); // Kalibrering av ADC3
  // Start ADC conversion TRGO tim7.
  ADC3_StartConversion_TRGO(); TIM7_Start_TRGO();

  /* Start USART3 DMA receive */
  USART_StartRx_DMA(USART3, usart3_SensorNode_Rx_buf, sizeof(usart3_SensorNode_Rx_buf));
  
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
  // Konverter ADC verdi til mV og deretter til mm avstand(lookup)
  uint16_t bit_value = LL_ADC_REG_ReadConversionData12(ADC3);
  adc_mV = (bit_value * 3000) / 4096;
  avstand_mm = lookup[(adc_mV)];
  LL_ADC_ClearFlag_EOC(ADC3);

  // Send data Til styreNode
  if(!start_sending_flag) return;
  USART_Tx_Tid_Avstand(USART3, tid, avstand_mm);
  tid++;
}

void USART_RxDMAComplete_Callback_SensorNode(USART_TypeDef *USARTx, uint8_t *buf, uint16_t len)
{
  if (len >= 3 && buf[0] == 0xAA && buf[2] == 0x55)  // Skjekker gyldig start/stop pakke
  {
    uint8_t start_stop_byte = buf[1];
    if (start_stop_byte == 0x01) // Start
    { 
      start_sending_flag = 1; 
      TIM4_Start_PWM(); // Start av PWM 4000 Hz for sette knekkfrekvens(40 HZ) filter.
      ADC3_Calibrate(); // Kalibrering av ADC3
      // Start ADC conversion TRGO tim7.
      ADC3_StartConversion_TRGO(); TIM7_Start_TRGO();
      LL_GPIO_SetOutputPin(LED7_GPIO_PORT, LED7_PIN);    
    }
    else if (start_stop_byte == 0x00) // Stopp
    {
      start_sending_flag = 0;
      TIM4_Stopp_PWM(); // Stopp av PWM 4000 Hz for sette knekkfrekvens(40 HZ) filter.
      ADC3_StopConversion_TRGO(); TIM7_Stopp_TRGO();
      tid = 0;
      LL_GPIO_ResetOutputPin(LED7_GPIO_PORT, LED7_PIN);
    }
  }
  /* Start ny receive USART3 DMA */
  USART_StartRx_DMA(USART3, usart3_SensorNode_Rx_buf, sizeof(usart3_SensorNode_Rx_buf));
}