/* Includes ------------------------------------------------------------------*/
#include "dma.h"
#include "usart.h"
#include "stm32f3xx_ll_dma.h"
#include "stm32f3xx_ll_usart.h"

void DMA_Init(void)
{

  /* Init with LL driver */
  /* DMA controller clock enable */
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA1);

  /* DMA interrupt init */
  /* DMA1_Channel3_IRQn interrupt configuration */
  NVIC_SetPriority(DMA1_Channel3_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),0, 0));
  NVIC_EnableIRQ(DMA1_Channel3_IRQn);
  /* DMA1_Channel6_IRQn interrupt configuration */
  NVIC_SetPriority(DMA1_Channel6_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),0, 0));
  NVIC_EnableIRQ(DMA1_Channel6_IRQn);

}

/* DMA IRQ handlers: call into USART module when RX transfer completes */
void USART_DMA_Channel3_Handler(void)
{
  /* Channel 3 -> USART3 RX */
  if (LL_DMA_IsActiveFlag_TC3(DMA1)) {
    LL_DMA_ClearFlag_TC3(DMA1);
    LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_3);
    LL_USART_DisableDMAReq_RX(USART3);
    USART_HandleDMA_RxComplete(USART3);
  }
  if (LL_DMA_IsActiveFlag_TE3(DMA1)) {
    LL_DMA_ClearFlag_TE3(DMA1);
    /* Transfer error handling could be added here */
  }
}

void USART_DMA_Channel6_Handler(void)                           
{
  /* Channel 6 -> USART2 RX */
  if (LL_DMA_IsActiveFlag_TC6(DMA1)) {
    LL_DMA_ClearFlag_TC6(DMA1);
    LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_6);
    LL_USART_DisableDMAReq_RX(USART2);
    USART_HandleDMA_RxComplete(USART2);
  }
  if (LL_DMA_IsActiveFlag_TE6(DMA1)) {
    LL_DMA_ClearFlag_TE6(DMA1);
    /* Transfer error handling could be added here */
  }
}


