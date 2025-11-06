/* Includes ------------------------------------------------------------------*/
#include "gpio.h"
#include "main.h"
#include "stm32f3xx_ll_gpio.h"


void GPIO_Init(void)
{
  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* GPIO Ports Clock Enable */
  SW1_GPIO_CLK_ENABLE();
  DIR_GPIO_CLK_ENABLE();
  LED3_GPIO_CLK_ENABLE();
  LED7_GPIO_CLK_ENABLE();
  sensorNode_Enable_GPIO_CLK_ENABLE();
  /*Reset LED_PIN*/
  LL_GPIO_ResetOutputPin(LED3_GPIO_PORT, LED3_PIN);
  LL_GPIO_ResetOutputPin(LED10_GPIO_PORT, LED10_PIN);

  
  /* DIR_PIN*/
  GPIO_InitStruct.Pin = DIR_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(DIR_GPIO_Port, &GPIO_InitStruct);

  /* SW1_Pin PF4*/
  GPIO_InitStruct.Pin = SW1_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(SW1_GPIO_Port, &GPIO_InitStruct);

  /* sensorNode_Enable_PIN  PB11 */
  GPIO_InitStruct.Pin = sensorNode_Enable_PIN;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_DOWN;
  LL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* LED 3*/
  GPIO_InitStruct.Pin = LED3_PIN;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(LED3_GPIO_PORT, &GPIO_InitStruct);

  // /*LED 7*/
  // GPIO_InitStruct.Pin = LED7_PIN;
  // GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
  // GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  // GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  // GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  // LL_GPIO_Init(LED7_GPIO_PORT, &GPIO_InitStruct);


  /* LED 10*/
  GPIO_InitStruct.Pin = LED10_PIN;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(LED10_GPIO_PORT, &GPIO_InitStruct);
}


