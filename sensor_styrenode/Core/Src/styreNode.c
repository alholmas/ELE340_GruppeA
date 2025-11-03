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
static uint8_t usart2_Rx_buf[13];
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
  (void)USART_StartRx_DMA(USART2, usart2_Rx_buf, sizeof(usart2_Rx_buf));
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
      /* Little-endian: LSB first */
      uint32_t tid = (uint32_t)buf[1] | ((uint32_t)buf[2] << 8) | ((uint32_t)buf[3] << 16) | ((uint32_t)buf[4] << 24);
      uint16_t mm = (uint16_t)buf[5] | ((uint16_t)buf[6] << 8);
      /* Sender data videre til PC */
      USART_Transmit_Tid_Avstand(USART2, tid, mm);
    }

    /* Restart DMA */
    (void)USART_StartRx_DMA(USART3, usart3_Rx_buf, sizeof(usart3_Rx_buf));
    
  }
  else if (USARTx == USART2) {
    if (len >= 13 && buf[0] == 0xAA && buf[12] == 0x55) {
      uint8_t start_stop_byte = buf[1];
      /* Little-endian: LSB first for 16-bit fields */
      uint16_t Kp = (uint16_t)buf[2] | ((uint16_t)buf[3] << 8);
      uint16_t Ti = (uint16_t)buf[4] | ((uint16_t)buf[5] << 8);
      uint16_t Td = (uint16_t)buf[6] | ((uint16_t)buf[7] << 8);
      uint16_t integrator_limit = (uint16_t)buf[8] | ((uint16_t)buf[9] << 8);
      uint16_t setpoint = (uint16_t)buf[10] | ((uint16_t)buf[11] << 8);

      if (start_stop_byte == 0x01) {
        USART_Transmit_Start_Stop(USART2, 0x01);
        LL_GPIO_TogglePin(LED3_GPIO_PORT, LED3_PIN);
      } else if (start_stop_byte == 0x00) {
        USART_Transmit_Start_Stop(USART2, 0x00);
      } else if (start_stop_byte == 0x02 || start_stop_byte == 0x01) {
        /* Oppdater parametere (bruk variablene som trengs) */
        (void)Kp; (void)Ti; (void)Td; (void)integrator_limit; (void)setpoint;
      }
      /* Restart DMA */
      (void)USART_StartRx_DMA(USART2, usart2_Rx_buf, sizeof(usart2_Rx_buf));
     }
  }
}