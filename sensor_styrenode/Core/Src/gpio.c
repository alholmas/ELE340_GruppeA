/* Includes ------------------------------------------------------------------*/
#include "gpio.h"
#include "main.h"
#include "stm32f3xx_ll_exti.h"


void GPIO_Init(void)
{
  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* GPIO Ports Clock Enable */
  SW1_GPIO_CLK_ENABLE();
  USER_BUTTON_GPIO_CLK_ENABLE();
  DIR_GPIO_CLK_ENABLE();
  LED3_GPIO_CLK_ENABLE();
  LED7_GPIO_CLK_ENABLE();
  LED10_GPIO_CLK_ENABLE();
  system_velger_GPIO_CLK_ENABLE();
  /*Reset LED_PIN*/
  LL_GPIO_ResetOutputPin(LED3_GPIO_PORT, LED3_PIN);
  LL_GPIO_ResetOutputPin(LED7_GPIO_PORT, LED7_PIN);
  LL_GPIO_ResetOutputPin(LED10_GPIO_PORT, LED10_PIN);

  
  /* DIR_PIN*/
  GPIO_InitStruct.Pin = DIR_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(DIR_GPIO_Port, &GPIO_InitStruct);

  /* sensorNode_Enable_PIN  PB11 */
  GPIO_InitStruct.Pin = system_velger_PIN;
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

  /*LED 7*/
  GPIO_InitStruct.Pin = LED7_PIN;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(LED7_GPIO_PORT, &GPIO_InitStruct);


  /* LED 10*/
  GPIO_InitStruct.Pin = LED10_PIN;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(LED10_GPIO_PORT, &GPIO_InitStruct);

  LL_EXTI_InitTypeDef EXTI_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOF);
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);

  /**/
  LL_SYSCFG_SetEXTISource(LL_SYSCFG_EXTI_PORTA, LL_SYSCFG_EXTI_LINE0);

  /**/
  LL_SYSCFG_SetEXTISource(LL_SYSCFG_EXTI_PORTF, LL_SYSCFG_EXTI_LINE4);

  /**/
  LL_GPIO_SetPinPull(GPIOA, LL_GPIO_PIN_0, LL_GPIO_PULL_NO);

  /**/
  LL_GPIO_SetPinPull(GPIOF, LL_GPIO_PIN_4, LL_GPIO_PULL_NO);

  /**/
  LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_0, LL_GPIO_MODE_INPUT);

  /**/
  LL_GPIO_SetPinMode(GPIOF, LL_GPIO_PIN_4, LL_GPIO_MODE_INPUT);

  /**/
  EXTI_InitStruct.Line_0_31 = LL_EXTI_LINE_0;
  EXTI_InitStruct.Line_32_63 = LL_EXTI_LINE_NONE;
  EXTI_InitStruct.LineCommand = ENABLE;
  EXTI_InitStruct.Mode = LL_EXTI_MODE_IT;
  EXTI_InitStruct.Trigger = LL_EXTI_TRIGGER_RISING;
  LL_EXTI_Init(&EXTI_InitStruct);

  /**/
  EXTI_InitStruct.Line_0_31 = LL_EXTI_LINE_4;
  EXTI_InitStruct.LineCommand = ENABLE;
  EXTI_InitStruct.Mode = LL_EXTI_MODE_IT;
  EXTI_InitStruct.Trigger = LL_EXTI_TRIGGER_FALLING;
  LL_EXTI_Init(&EXTI_InitStruct);

  /* EXTI interrupt init*/
  NVIC_SetPriority(EXTI0_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),0, 0));
  NVIC_EnableIRQ(EXTI0_IRQn);
  NVIC_SetPriority(EXTI4_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),0, 0));
  NVIC_EnableIRQ(EXTI4_IRQn);

}


void __attribute__((weak)) USER_BUTTON_Callback(void)
{
  /* Tom default-implementasjon; kan overstyres ved å definere samme funksjon uten __weak i en annen .c-fil */
}

void __attribute__((weak)) SW1_Callback(void)
{
  /* Tom default-implementasjon; kan overstyres ved å definere samme funksjon uten __weak i en annen .c-fil */
}