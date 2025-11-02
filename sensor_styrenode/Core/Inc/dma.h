#ifndef __DMA_H__
#define __DMA_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"
/* Init GPIO -----------------------------------------------------------------*/
void DMA_Init(void);


/* Interrupt handlers --------------------------------------------------------*/
void USART_DMA_Channel3_Handler(void);
void USART_DMA_Channel6_Handler(void);

#ifdef __cplusplus
}
#endif

#endif /* __DMA_H__ */
