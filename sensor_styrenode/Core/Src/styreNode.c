#include "styreNode.h"
#include "main.h"
#include "stm32f303xc.h"


/* Include peripheral drivers -----------------------------------------------*/
#include "adc.h"
#include "stm32f3xx_ll_gpio.h"
#include "stm32f3xx_ll_usart.h"
#include "stm32f3xx_ll_utils.h"
#include "tim.h"
#include "gpio.h"
#include "usart.h"
#include <stdint.h>
#include <sys/stat.h>
#include "dma.h"

/* RX buffer for DMA reception from sensor node (8-byte packets) */
static uint8_t usart3_Rx_buf[8];
// static volatile uint8_t usart2_Rx_buf[8];


void StyreNode_Init(void)
{
  /* Initialisering av perifere enheter for sensorNode ----------------*/
  TIM3_Init();
  USART2_Init();
  USART3_Init();
  GPIO_Init();

  // LED 10 angir oppsart som styreNode
  LL_GPIO_SetOutputPin(LED10_GPIO_PORT, LED10_PIN);

  
  /* Oppstart av perifere enheter for sensorNode ----------------------*/
  (void)USART_StartRx_DMA(USART3, usart3_Rx_buf, sizeof(usart3_Rx_buf));
  TIM3_Start_PWM();

}

void StyreNode_Loop(void)
{
	  for (int i = 0; i < 100; i++)
  {
    (void)TIM3_SetFrequencyHz(10000 + i * 100);
    LL_mDelay(500);
  }
}
/* SensorNode spesifikke funksjoner -------------------------------------*/


/* Interrupt Callback --------------------------------------------------- */




void USART_RxDMAComplete_Callback(USART_TypeDef *USARTx, uint8_t *buf, uint16_t len)
{
  if (USARTx == USART3) {
    if (len >= 8 && buf[0] == 0xAA && buf[7] == 0x55) {
      uint32_t tid = ((uint32_t)buf[1] << 24) | ((uint32_t)buf[2] << 16) | ((uint32_t)buf[3] << 8) | (uint32_t)buf[4];
      uint16_t mm = ((uint16_t)buf[5] << 8) | (uint16_t)buf[6];
      USART_Transmit_Tid_Avstand(USART2, tid, mm);
    }

    /* Restart DMA for next packet */
    (void)USART_StartRx_DMA(USART3, usart3_Rx_buf, sizeof(usart3_Rx_buf));
    
  }
  else if (USARTx == USART2) {
    /* If you enable USART2 DMA RX, handle it here similarly */
  }
}