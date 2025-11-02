#ifndef __USART_H__
#define __USART_H__

#include "stm32f3xx_ll_usart.h"
#ifdef __cplusplus
extern "C" {
#endif
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Init USART ----------------------------------------------------------------*/
void USART2_Init(void);
void USART3_Init(void);


/* Metoder for USART ------------------------------------------------------------*/
void USART_Tx(USART_TypeDef *USARTx, uint8_t Value);
int USART_StartRx_DMA(USART_TypeDef *USARTx, uint8_t *buffer, uint16_t length);
int USART_SendBuffer_IT(USART_TypeDef *USARTx, uint8_t *buffer, uint16_t length);


void USART_Transmit_Tid_Avstand(USART_TypeDef *USARTx, uint32_t tid, uint16_t mmAvstand);
void USART_Transmit_Tid_Avstand_Avvik(USART_TypeDef *USARTx, uint32_t tid, uint16_t mmAvstand, uint16_t mmAvvik);

/* Non-blocking transmit using TXE/TC interrupts
 * Returns 0 on success, -1 on invalid args, -2 if UART is busy, -3 if length too large
 */


/* Handlers called from IRQ when flags are set */
void USART_TXE_Handler(USART_TypeDef *USARTx);
void USART_TC_Handler(USART_TypeDef *USARTx);

/* DMA RX API */
/* Start RX using DMA channel pre-configured in MX_DMA_Init (dma.c).
 * Returns 0 on success, -1 invalid args, -2 if DMA channel busy.
 */


/* DMA channel IRQ handlers implemented in usart.c and called from
	DMA1_ChannelX IRQs in stm32f3xx_it.c */
/* DMA IRQ handlers moved to dma.c */




void USART_TxComplete_Callback(USART_TypeDef *USARTx);
/* Called by dma.c (on DMA transfer complete) to let USART module notify higher layer.
 * Implemented in usart.c and will call the weak callback below with buffer pointer and length.
 */
void USART_HandleDMA_RxComplete(USART_TypeDef *USARTx);

/* Weak callback invoked when a DMA RX transfer completes. Provides buffer pointer and length.
 * Override in application (e.g. styreNode.c) to process data.
 */
void USART_RxDMAComplete_Callback(USART_TypeDef *USARTx, uint8_t *buf, uint16_t len);



// volatile uint8_t usart2_rx_data[8];
// volatile uint8_t usart3_rx_data[8];
// voltile uint8_t usart2_data_ready;
// volatile uint8_t usart3_data_ready;
#ifdef __cplusplus
}
#endif

#endif /* __USART_H__ */

