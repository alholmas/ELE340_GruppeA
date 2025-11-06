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

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static volatile uint32_t tid = 0;
static volatile uint16_t adc_mV = 0;
static uint8_t usart3_SensorNode_Rx_buf[3];

// static volatile uint16_t avstand_mm = 0;
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

// uint16_t konverter_mm(uint16_t adc_mV)
// {
//   // Antagelse: lineær sammenheng, 300 mV => 100 mm, 3000 mV => 1500 mm
//   // Juster disse kalibreringspunktene om nødvendig
//   const int32_t V1_mV = 300;   // nedre referanse (mV)
//   const int32_t MM1   = 100;   // tilsvarende mm ved V1
//   const int32_t V2_mV = 3000;  // øvre referanse (mV)
//   const int32_t MM2   = 1500;  // tilsvarende mm ved V2

//   // Clamp inngangsverdi til kalibrert område
//   int32_t v = adc_mV;
//   if (v < V1_mV) v = V1_mV;
//   if (v > V2_mV) v = V2_mV;

//   // Lineær interpolasjon: mm = MM1 + (v - V1) * (MM2 - MM1) / (V2 - V1)
//   const int32_t dv = V2_mV - V1_mV;      // 2700
//   const int32_t dm = MM2   - MM1;        // 1400
//   int32_t mm = MM1 + ( (v - V1_mV) * dm ) / dv;
//   return (uint16_t)mm;
// }

/* Interrupt Callback --------------------------------------------------- */
void ADC3_EndOfConversion_Callback(void)
{
  tid++;
  uint16_t bit_value = LL_ADC_REG_ReadConversionData12(ADC3);
  adc_mV = konverter_mV(bit_value);
  LL_ADC_ClearFlag_EOC(ADC3);

  /* Oppdater PID med siste måling slik at `pid`-felt og debug-variabel oppdateres
   * (gjør at debuggerens live-watch vil vise oppdatert verdi). */
  

  // Send data Til styreNode
  USART_Transmit_Tid_Avstand(USART3, tid, adc_mV);
}



void USART_RxDMAComplete_Callback_SensorNode(USART_TypeDef *USARTx, uint8_t *buf, uint16_t len)
{
  
  if (len >= 3 && buf[0] == 0xAA && buf[2] == 0x55)
  {
    uint8_t start_stop_byte = buf[1];
    if (start_stop_byte == 0x01)
    {
      // Start ADC conversion TRGO tim7.
      TIM7_Start_TRGO();
      ADC3_StartConversion_TRGO(); 
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