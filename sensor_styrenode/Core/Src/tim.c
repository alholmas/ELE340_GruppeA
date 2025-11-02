#include "tim.h"



void TIM4_Init(void)
{  

    // RCC->APB1ENR |= RCC_APB1ENR_TIM4EN; 
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
    GPIO_InitStruct.Pin = F_CLK_Pin;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
    GPIO_InitStruct.Alternate = LL_GPIO_AF_2;
    LL_GPIO_Init(F_CLK_GPIO_Port, &GPIO_InitStruct);
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
