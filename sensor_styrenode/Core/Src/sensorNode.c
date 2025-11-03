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

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static volatile uint32_t tid = 0;
static volatile uint16_t adc_mV = 0;
static volatile uint16_t avsand_mm = 0;
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


  //LED 3 angir oppsett som sensorNode
  LL_GPIO_SetOutputPin(LED3_GPIO_PORT, LED3_PIN);

  /* Oppstart av perifere enheter for sensorNode ----------------------*/
  //Start av ADC3
  TIM7_Start_TRGO();
  ADC3_StartConversion_TRGO(); 
  //Start av PWM 4000 Hz knekkfrekvens filter 
  TIM4_Start_PWM();


  
}

void SensorNode_Loop(void)
{
  // Main loop for sensorNode
}


/* SensorNode spesifikke metoder ----------------------------------------*/

uint16_t konverter_mV(uint16_t adc_val)
{
    return (adc_val * 3000) / 4096;
}

uint16_t konverter_mm(uint16_t adc_mV)
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
  return (uint16_t)mm;
}

/* Interrupt Callback --------------------------------------------------- */
void ADC3_EndOfConversion_Callback(void)
{
  tid++;

  // Les ADC-DR (vanligvis clear'er dette EOC)
  adc_mV = (uint16_t)LL_ADC_REG_ReadConversionData12(ADC3);
  avsand_mm = konverter_mm(adc_mV);
  LL_ADC_ClearFlag_EOC(ADC3);

  // Send data via USART3
  USART_Transmit_Tid_Avstand(USART3, tid, avsand_mm);
}



// void USART_RxDMAComplete_Callback(USART_TypeDef *USARTx, uint8_t *buf, uint16_t len)
// {
//   // SensorNode forventer ingen innkommende data, så denne kan være tom
// }