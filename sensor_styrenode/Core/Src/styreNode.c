#include "styreNode.h"


/* Include peripheral drivers -----------------------------------------------*/
#include "adc.h"
#include "main.h"
#include "stm32f3xx_ll_gpio.h"
#include "tim.h"
#include "gpio.h"
#include "usart.h"
#include "dma.h"
#include "pid.h"


/* Variabler ----------------------------------------------------------------*/
static uint8_t usart3_Rx_buf[8];
static uint8_t usart2_Rx_buf[13];
static volatile uint8_t system_aktivt = 0;
static pid_t pid = {0};
static volatile uint16_t mm_avstand = 0;


void StyreNode_Init(void)
{
  /* Initialisering av perifere enheter for sensorNode ----------------*/
  TIM3_Init();
  USART2_Init();
  USART3_Init();
  GPIO_Init();

  // LED 10 angir oppsart som styreNode
  LL_GPIO_SetOutputPin(LED10_GPIO_PORT, LED10_PIN);
  
  /* Oppstart av perifer enheter for sensorNode ----------------------*/
  USART_StartRx_DMA(USART3, usart3_Rx_buf, sizeof(usart3_Rx_buf));
  USART_StartRx_DMA(USART2, usart2_Rx_buf, sizeof(usart2_Rx_buf));

}

void StyreNode_Loop(void)
{

}
/* SensorNode spesifikke funksjoner -------------------------------------*/
void set_linmot_paadrag(pid_t *pid)
{
  if (!pid) return;
  if (pid->err > (int32_t)pid->err_deadzone || pid->err < -(int32_t)pid->err_deadzone)
  {
    if (pid->output > 0)
    {
      LL_GPIO_SetOutputPin(DIR_GPIO_Port, DIR_Pin);
      TIM3_SetFreq(pid->output);
    }
    else if (pid->output < 0)
    {
      pid->output = -pid->output;
      LL_GPIO_ResetOutputPin(DIR_GPIO_Port, DIR_Pin);
      TIM3_SetFreq(pid->output);
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
      mm_avstand = (uint16_t)buf[5] | ((uint16_t)buf[6] << 8);
      
      compute_PID_Output(&pid, mm_avstand);
      set_linmot_paadrag(&pid);
      /* Sender data videre til PC */
      USART_Tx_Tid_Avstand_PidPaadrag(USART2, tid, mm_avstand, &pid);
    }

    /* Restart DMA */
    USART_StartRx_DMA(USART3, usart3_Rx_buf, sizeof(usart3_Rx_buf));
    
  }
  // Mottak fra GUI
  else if (USARTx == USART2) {
    if (len >= 13 && buf[0] == 0xAA && buf[12] == 0x55) {
      uint8_t start_stop_byte = buf[1];
      /* Little-endian: LSB first for 16-bit fields */
      uint16_t Kp = (uint16_t)buf[2] | ((uint16_t)buf[3] << 8);
      uint16_t Ki = (uint16_t)buf[4] | ((uint16_t)buf[5] << 8);
      uint16_t Kd = (uint16_t)buf[6] | ((uint16_t)buf[7] << 8);
      uint32_t kb = (uint16_t)buf[8] | ((uint16_t)buf[9] << 8);
      uint16_t setpoint = (uint16_t)buf[10] | ((uint16_t)buf[11] << 8);
    
      
      switch (start_stop_byte) {
        case 0: // Stopp signal mottat fra GUI
          USART_Tx_Start_Stop(USART3, start_stop_byte);
          reset_pid(&pid);
          TIM3_Stop_PWM();
          system_aktivt = 0;
        break;
        case 1: // Start signal mottat fra GUI, settter PID verdier
          USART_Tx_Start_Stop(USART3, start_stop_byte);
          pid_init(&pid, Kp, Ki, Kd, kb, setpoint);
          if (!system_aktivt) {
            system_aktivt = 1;
            LL_GPIO_TogglePin(LED10_GPIO_PORT, LED10_PIN);
          }
        break;
        case 2: // Oppdater signal mottat fra GUI,setter KP, TI, TD, Setpoint og integral begrensing
          update_pid_parameters(&pid, Kp, Ki, Kd, kb, setpoint);
        break;
      }
      /* Restart DMA */
      USART_StartRx_DMA(USART2, usart2_Rx_buf, sizeof(usart2_Rx_buf));
     }
  }
}

// Manuell start/stopp via brukerknapp, med default PID verdier
void USER_BUTTON_Callback()
{
  if (!system_aktivt) {
    pid_default_init(&pid);
    USART_Tx_Start_Stop(USART3, 0x01);
    LL_GPIO_SetOutputPin(LED7_GPIO_PORT, LED7_PIN);
    system_aktivt = 1;
  } else {
    USART_Tx_Start_Stop(USART3, 0x00);
    reset_pid(&pid);
    TIM3_Stop_PWM();
    LL_GPIO_ResetOutputPin(LED7_GPIO_PORT, LED7_PIN);
    system_aktivt = 0;
  }
}

void SW1_Callback(void)
{
  if (pid.err_deadzone != 0) {
    pid_deactivate_deadzone(&pid);
  } else {
    pid_activate_deadzone(&pid);
  }
}