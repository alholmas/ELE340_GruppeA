#include "adc.h"



void ADC3_Init(void)
{  
  LL_ADC_InitTypeDef ADC_InitStruct = {0};
  LL_ADC_REG_InitTypeDef ADC_REG_InitStruct = {0};
  LL_ADC_CommonInitTypeDef ADC_CommonInitStruct = {0};

  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* Peripheral clock enable */
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_ADC34);

  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);
  /**ADC3 GPIO Configuration
  PB1   ------> ADC3_IN1
  */
  GPIO_InitStruct.Pin = LL_GPIO_PIN_1;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  NVIC_SetPriority(ADC3_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),0, 0));
  NVIC_EnableIRQ(ADC3_IRQn);

  ADC_InitStruct.Resolution = LL_ADC_RESOLUTION_12B;
  ADC_InitStruct.DataAlignment = LL_ADC_DATA_ALIGN_RIGHT;
  ADC_InitStruct.LowPowerMode = LL_ADC_LP_MODE_NONE;
  LL_ADC_Init(ADC3, &ADC_InitStruct);
  ADC_REG_InitStruct.TriggerSource = LL_ADC_REG_TRIG_EXT_TIM7_TRGO_ADC34;
  ADC_REG_InitStruct.SequencerLength = LL_ADC_REG_SEQ_SCAN_DISABLE;
  ADC_REG_InitStruct.SequencerDiscont = LL_ADC_REG_SEQ_DISCONT_DISABLE;
  ADC_REG_InitStruct.ContinuousMode = LL_ADC_REG_CONV_SINGLE;
  ADC_REG_InitStruct.DMATransfer = LL_ADC_REG_DMA_TRANSFER_LIMITED;
  ADC_REG_InitStruct.Overrun = LL_ADC_REG_OVR_DATA_OVERWRITTEN;
  LL_ADC_REG_Init(ADC3, &ADC_REG_InitStruct);
  ADC_CommonInitStruct.CommonClock = LL_ADC_CLOCK_ASYNC_DIV1;
  ADC_CommonInitStruct.Multimode = LL_ADC_MULTI_INDEPENDENT;
  LL_ADC_CommonInit(__LL_ADC_COMMON_INSTANCE(ADC3), &ADC_CommonInitStruct);
  LL_ADC_REG_SetTriggerEdge(ADC3, LL_ADC_REG_TRIG_EXT_RISING);

  /* Enable ADC internal voltage regulator */
  LL_ADC_EnableInternalRegulator(ADC3);
  /* Delay for ADC internal voltage regulator stabilization. */
  /* Compute number of CPU cycles to wait for, from delay in us. */
  /* Note: Variable divided by 2 to compensate partially */
  /* CPU processing cycles (depends on compilation optimization). */
  /* Note: If system core clock frequency is below 200kHz, wait time */
  /* is only a few CPU processing cycles. */
  uint32_t wait_loop_index;
  wait_loop_index = ((LL_ADC_DELAY_INTERNAL_REGUL_STAB_US * (SystemCoreClock / (100000 * 2))) / 10);
  while(wait_loop_index != 0)
  {
    wait_loop_index--;
  }

  /** Configure Regular Channel
  */
  LL_ADC_REG_SetSequencerRanks(ADC3, LL_ADC_REG_RANK_1, LL_ADC_CHANNEL_1);
  LL_ADC_SetChannelSamplingTime(ADC3, LL_ADC_CHANNEL_1, LL_ADC_SAMPLINGTIME_1CYCLE_5);
  LL_ADC_SetChannelSingleDiff(ADC3, LL_ADC_CHANNEL_1, LL_ADC_SINGLE_ENDED);  
}

void ADC3_StartConversion(void)
{
  LL_ADC_StartCalibration(ADC3, LL_ADC_SINGLE_ENDED);
  while (LL_ADC_IsCalibrationOnGoing(ADC3)) {
    /* wait calibration */
  }
  /* Enable End Of Conversion interrupt */
  LL_ADC_EnableIT_EOC(ADC3);
  /* Enable ADC */
  LL_ADC_Enable(ADC3);
  /* Wait for ADC ready */
  while (!LL_ADC_IsActiveFlag_ADRDY(ADC3)) {
  }
  LL_ADC_REG_StartConversion(ADC3);
}


void ADC3_StopConversion(void)
{
  LL_ADC_REG_StopConversion(ADC3);
  LL_ADC_DisableIT_EOC(ADC3);
  LL_ADC_Disable(ADC3);
} 

// Callback function for end of conversion for bruk i andre moduler
void __attribute__((weak)) ADC3_EndOfConversion_Callback(void)
{
  // Tom default-implementasjon; kan overstyres ved å definere samme funksjon uten __weak i en annen .c-fil
}
