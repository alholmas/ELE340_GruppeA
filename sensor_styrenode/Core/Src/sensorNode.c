#include "sensorNode.h"
#include "gpio.h"
#include "main.h"
#include "stm32f303xc.h"
#include "stm32f3xx_ll_gpio.h"
#include "stm32f3xx_ll_usart.h"
#include "tim.h"
#include "adc.h"
#include "usart.h"



/* Include peripheral drivers -----------------------------------------------*/
#include "adc.h"
#include "tim.h"
#include "gpio.h"
#include "usart.h"
#include <stdint.h>



// Variabler 
static volatile uint32_t tid = 0;
static volatile uint16_t adc_mV = 0;
static volatile uint16_t avsand_mm = 0;


int Is_sensorNode(void)
{
  // Leser PB11(høy verdi aktiverer )
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);
  LL_GPIO_SetPinMode(GPIOB, LL_GPIO_PIN_11, LL_GPIO_MODE_INPUT);
  LL_GPIO_SetPinPull(GPIOB, LL_GPIO_PIN_11, LL_GPIO_PULL_DOWN);
  return (LL_GPIO_IsInputPinSet(GPIOB, LL_GPIO_PIN_11) != 0);
}

void SensorNode_Init(void)
{
  /* Initialisering av perifere enheter for sensorNode ----------------*/
  TIM4_Init();
  TIM7_Init();
  ADC3_Init();
  USART3_Init();
  USART2_Init();
  GPIO_Init();

  /* Oppstart av perifere enheter for sensorNode ----------------------*/
  // Start av ADC3 og TIM7 for trigging
  TIM7_Start();
  ADC3_StartConversion(); 
  //Start av PWM for å sette knekkfrekvens på filter
  TIM4_Start();

  LL_USART_TransmitData8(USART2, 0x55);
  LL_GPIO_TogglePin(LED3_GPIO_PORT, LED3_PIN); // Indikator pin toggle ved init


  



}

void SensorNode_Loop(void)
{
  // Main loop for sensorNode
}

/* SensorNode spesifikke funksjoner -------------------------------------*/

int konverter_mV(uint16_t adc_val)
{
    return (adc_val * 3000) / 4096;
}

int konverter_mm(uint16_t adc_mV)
{
  // Antagelse: lineær sammenheng, 300 mV => 100 mm, 3000 mV => 1500 mm
  // Juster disse kalibreringspunktene om nødvendig
  const int32_t V1_mV = 300;   // nedre referanse (mV)
  const int32_t MM1   = 100;   // tilsvarende mm ved V1
  const int32_t V2_mV = 3000;  // øvre referanse (mV)
  const int32_t MM2   = 1500;  // tilsvarende mm ved V2

  // Clamp inngangsverdi til kalibrert område
  int32_t v = adc_mV;
  if (v < V1_mV) v = V1_mV;
  if (v > V2_mV) v = V2_mV;

  // Lineær interpolasjon: mm = MM1 + (v - V1) * (MM2 - MM1) / (V2 - V1)
  const int32_t dv = V2_mV - V1_mV;      // 2700
  const int32_t dm = MM2   - MM1;        // 1400
  int32_t mm = MM1 + ( (v - V1_mV) * dm ) / dv;
  return (int)mm;
}

/* Interrupt Callback --------------------------------------------------- */
void ADC3_EndOfConversion_Callback(void)
{
  tid++;
  
  // Les ADC-DR (vanligvis clear'er dette EOC)
  adc_mV = (uint16_t)LL_ADC_REG_ReadConversionData12(ADC3);
  avsand_mm = konverter_mm(adc_mV);
  LL_ADC_ClearFlag_EOC(ADC3);

  USART3_Transmit_sensorNode(tid, avsand_mm);
  USART2_Transmit_styreNode(tid, avsand_mm, adc_mV);
}





uint16_t SensorNode_GetLastAdc(void)
{
  return adc_mV;
}

uint32_t SensorNode_GetLastTid(void)
{
  return tid;
}