#include "styreNode.h"


/* Include peripheral drivers -----------------------------------------------*/
#include "adc.h"
#include "tim.h"
#include "gpio.h"
#include "usart.h"
#include "dma.h"
#include "pid.h"


/* RX buffer for DMA reception from sensor node (8-byte packets) */
static uint8_t usart3_Rx_buf[8];
static uint8_t usart2_Rx_buf[15];
static pid_t pid = {0};



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

}

void StyreNode_Loop(void)
{

}
/* SensorNode spesifikke funksjoner -------------------------------------*/

void set_linmot_paadrag(pid_t *pid)
{
  if (!pid) return;
  int32_t paadrag_hz = pid->output;
  int32_t error = pid->error;
  if (error > 2 || error < -2)
  {
    if (paadrag_hz > 0)
    {
      LL_GPIO_ResetOutputPin(DIR_GPIO_Port, DIR_Pin);
      TIM3_SetFreq(paadrag_hz);
    }
    else if (paadrag_hz < 0)
    {
      paadrag_hz = -paadrag_hz;
      LL_GPIO_SetOutputPin(DIR_GPIO_Port, DIR_Pin);
      TIM3_SetFreq(paadrag_hz);
    }
  }
  else
  {
     TIM3_Stop_PWM();
  }
}


/* Interrupt Callback --------------------------------------------------- */
void USART_RxDMAComplete_Callback_StyreNode(USART_TypeDef *USARTx, uint8_t *buf, uint16_t len)
{
  // Mottak fra sensorNode
  if (USARTx == USART3) {
    if (len >= 8 && buf[0] == 0xAA && buf[7] == 0x55) {
      /* Little-endian: LSB first */
      uint32_t tid = (uint32_t)buf[1] | ((uint32_t)buf[2] << 8) | ((uint32_t)buf[3] << 16) | ((uint32_t)buf[4] << 24);
      uint16_t mm = (uint16_t)buf[5] | ((uint16_t)buf[6] << 8);
      
      (void)compute_PID_Output(&pid, mm);
      set_linmot_paadrag(&pid);
      /* Sender data videre til PC */
      USART_Tx_Tid_Avstand_PidPaadrag(USART2, tid, mm, &pid);
    }

    /* Restart DMA */
    (void)USART_StartRx_DMA(USART3, usart3_Rx_buf, sizeof(usart3_Rx_buf));
    
  }
  // Mottak fra GUI
  else if (USARTx == USART2) {
    if (len >= 15 && buf[0] == 0xAA && buf[14] == 0x55) {
      uint8_t start_stop_byte = buf[1];
      /* Little-endian: LSB first for 16-bit fields */
      uint16_t Kp = (uint16_t)buf[2] | ((uint16_t)buf[3] << 8);
      uint16_t Ti = (uint16_t)buf[4] | ((uint16_t)buf[5] << 8);
      uint16_t Td = (uint16_t)buf[6] | ((uint16_t)buf[7] << 8);
      uint32_t integral_limit = (uint16_t)buf[8] | ((uint16_t)buf[9] << 8) | ((uint16_t)buf[10] << 16) | ((uint16_t)buf[11] << 24);
      uint16_t setpoint = (uint16_t)buf[12] | ((uint16_t)buf[13] << 8);
    
      
      switch (start_stop_byte) {
        case 0: // Stopp signal mottat fra GUI
          USART_Tx_Start_Stop(USART3, start_stop_byte);
          reset_pid(&pid);
          // TIM3_SetFrequencyHz(0);
        break;
        case 1: // Start signal mottat fra GUI, settter PID verdier
          LL_GPIO_TogglePin(LED10_GPIO_PORT, LED10_PIN);
          USART_Tx_Start_Stop(USART3, start_stop_byte);
          pid_init(&pid, Kp, Ti, Td, setpoint, integral_limit);
          
        break;
        case 2: // Oppdater signal mottat fra GUI,setter KP, TI, TD, Setpoint og integral begrensing
          update_pid_parameters(&pid, Kp, Ti, Td, setpoint, integral_limit);
          LL_GPIO_SetOutputPin(LED3_GPIO_PORT, LED3_PIN);
        break;

      }
      /* Restart DMA */
      (void)USART_StartRx_DMA(USART2, usart2_Rx_buf, sizeof(usart2_Rx_buf));
     }
  }
}