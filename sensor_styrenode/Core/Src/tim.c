#include "tim.h"
#include "stm32f303xc.h"
#include "stm32f3xx_ll_gpio.h"
#include "stm32f3xx_ll_rcc.h"
#include "stm32f3xx_ll_tim.h"

void TIM3_Init(void)
{
  LL_TIM_InitTypeDef TIM_InitStruct = {0};
  LL_TIM_OC_InitTypeDef TIM_OC_InitStruct = {0};

  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* Peripheral clock enable */
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM3);


  TIM_InitStruct.Prescaler = 71;
  TIM_InitStruct.CounterMode = LL_TIM_COUNTERMODE_UP;
  TIM_InitStruct.Autoreload = 999;
  TIM_InitStruct.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1;
  LL_TIM_Init(TIM3, &TIM_InitStruct);
  LL_TIM_DisableARRPreload(TIM3);
  LL_TIM_SetClockSource(TIM3, LL_TIM_CLOCKSOURCE_INTERNAL);
  LL_TIM_OC_EnablePreload(TIM3, LL_TIM_CHANNEL_CH1);
  TIM_OC_InitStruct.OCMode = LL_TIM_OCMODE_PWM1;
  TIM_OC_InitStruct.OCState = LL_TIM_OCSTATE_ENABLE;
  TIM_OC_InitStruct.OCNState = LL_TIM_OCSTATE_DISABLE;
  
  TIM_OC_InitStruct.CompareValue = 500;
  TIM_OC_InitStruct.OCPolarity = LL_TIM_OCPOLARITY_HIGH;
  
  // Buffer for ARR og CCR1 registers
  /* Enable ARR/CCR preload for TIM3 */
  LL_TIM_EnableARRPreload(TIM3);
  LL_TIM_OC_EnablePreload(TIM3, LL_TIM_CHANNEL_CH1);


  LL_TIM_OC_Init(TIM3, LL_TIM_CHANNEL_CH1, &TIM_OC_InitStruct);
  LL_TIM_OC_DisableFast(TIM3, LL_TIM_CHANNEL_CH1);
  LL_TIM_SetTriggerOutput(TIM3, LL_TIM_TRGO_RESET);
  LL_TIM_DisableMasterSlaveMode(TIM3);


  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);
    /**TIM3 GPIO Configuration
    PB4     ------> TIM3_CH1
    */
  GPIO_InitStruct.Pin = LL_GPIO_PIN_4;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_2;
  LL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}
void TIM3_Start_PWM(void)
{
  LL_TIM_EnableCounter(TIM3);
}

void TIM3_Stop_PWM(void)
{
  LL_TIM_DisableCounter(TIM3);
}

int TIM3_SetFrequencyHz(uint32_t freq_hz)
{
    if (freq_hz == 0) return -1;

    /* Use current PSC value */
    uint32_t psc = TIM3->PSC;
    uint32_t timer_clk = 72000000;

    uint64_t denom = (uint64_t)freq_hz * (uint64_t)(psc + 1U);
    if (denom == 0) return -1;
    uint64_t arr64 = (uint64_t)timer_clk / denom;
    if (arr64 == 0) return -1;
    arr64 = arr64 - 1ULL;
    if (arr64 > 0xFFFFULL) return -1; /* cannot represent in 16-bit ARR */
    uint32_t arr = (uint32_t)arr64;

    /* Set ARR and keep 50% duty by setting CCR1 = (ARR+1)/2 - 1? For 50% duty, CCR = (ARR+1)/2 */
    TIM3->ARR = arr;
    uint32_t ccr = (uint32_t)(((uint64_t)(arr + 1U) * 50U) / 100U);
    if (ccr > arr) ccr = arr;
    TIM3->CCR1 = ccr;
    return 0;
}




void TIM4_Init(void)
{  

  // Enable clock for TIM4
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM4);

  TIM4->CCMR1 = 0x68;            // = 0b01101000
  TIM4->CCER = 0x01;
  TIM4->PSC = 71;                // = 71, altså 72. Det gir f_TIM4 = 1 MHz og T_TIM4 = 1 us.
  TIM4->ARR = 249;               // = 249            (altså til 250 us, f = 4 kHz)
  TIM4->CCR1 = 125;              // = 125            (altså til 125, halvparten D = 50%)

  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOD);
    /**TIM4 GPIO Configuration
    PD12     ------> TIM4_CH1
    */
  GPIO_InitStruct.Pin = LL_GPIO_PIN_12;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_2;
  LL_GPIO_Init(GPIOD, &GPIO_InitStruct);
}

void TIM4_Start_PWM(void)
{
  LL_TIM_EnableCounter(TIM4);
}

void TIM4_Stopp_PWM(void)
{
  LL_TIM_DisableCounter(TIM4);
}


void TIM7_Init(void)
{
  LL_TIM_InitTypeDef TIM_InitStruct = {0};

  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM7);

  TIM_InitStruct.Prescaler = 7199; /* 72 000 000 / 7200 = 10000 Hz */
  TIM_InitStruct.CounterMode = LL_TIM_COUNTERMODE_UP;
  TIM_InitStruct.Autoreload = 99; /* 10000 / 100 = 100 Hz -> 10 ms period */
  LL_TIM_Init(TIM7, &TIM_InitStruct);
  LL_TIM_DisableARRPreload(TIM7);
  LL_TIM_SetTriggerOutput(TIM7, LL_TIM_TRGO_UPDATE);
  LL_TIM_DisableMasterSlaveMode(TIM7);

}

void TIM7_Start_TRGO(void)
{
  LL_TIM_EnableCounter(TIM7);
}

void TIM7_Stopp_TRGO(void)
{
  LL_TIM_DisableCounter(TIM7);
}
