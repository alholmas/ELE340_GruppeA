#include "styreNode.h"
#include "stm32f303xc.h"


/* Include peripheral drivers -----------------------------------------------*/
#include "adc.h"
#include "stm32f3xx_ll_usart.h"
#include "tim.h"
#include "gpio.h"
#include "usart.h"


void StyreNode_Init(void)
{
  /* Initialisering av perifere enheter for sensorNode ----------------*/
  TIM4_Init();
  TIM7_Init();
  ADC3_Init();
  USART2_Init();
  USART3_Init();

  GPIO_Init();

  /* Oppstart av perifere enheter for sensorNode ----------------------*/
}

void StyreNode_Loop(void)
{
	// TODO: Kjør periodiske oppgaver for styre-node her hvis nødvendig
}
/* SensorNode spesifikke funksjoner -------------------------------------*/


/* Interrupt Callback --------------------------------------------------- */
void USART2_RxReady_Callback()
{ 
  // uint8_t reciveBuffer[10];
  // for (int i = 0; i < 8; i++)
  // {
  //   LL_USART_ReceiveData8(USART2);
  // }
  
  
}

void USART3_RxReady_Callback()
{
  // Håndter mottatt data på USART3 for styreNode her
}