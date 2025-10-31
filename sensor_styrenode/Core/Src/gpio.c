/* Includes ------------------------------------------------------------------*/
#include "gpio.h"


void GPIO_Init(void)
{

  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* GPIO Ports Clock Enable */
  SW1_GPIO_CLK_ENABLE();
  LED3_GPIO_CLK_ENABLE();
  /*Reset LED3_PIN*/
  LL_GPIO_ResetOutputPin(LED3_GPIO_PORT, LED3_PIN);

  /*SW1_Pin PF4*/
  GPIO_InitStruct.Pin = SW1_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(SW1_GPIO_Port, &GPIO_InitStruct);

  /*LED 3*/
  GPIO_InitStruct.Pin = LED3_PIN;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(LED3_GPIO_PORT, &GPIO_InitStruct);

}


