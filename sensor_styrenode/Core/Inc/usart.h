#ifndef __USART_H__
#define __USART_H__

#include "stm32f3xx_ll_usart.h"
#ifdef __cplusplus
extern "C" {
#endif
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "pid.h"

/* Init USART ----------------------------------------------------------------*/
void USART2_Init(void);
void USART3_Init(void);


/* Metoder for USART ------------------------------------------------------------*/
void USART_Tx(USART_TypeDef *USARTx, uint8_t Value);
int USART_StartRx_DMA(USART_TypeDef *USARTx, uint8_t *buffer, uint16_t length);
int USART_Tx_Buffer_IT(USART_TypeDef *USARTx, uint8_t *buffer, uint16_t length);


// Transmit funkjsoner for sensorNode and styreNode
void USART_Tx_Start_Stop(USART_TypeDef *USARTx, uint8_t start_stop_byte);
void USART_Tx_Tid_Avstand(USART_TypeDef *USARTx, uint32_t tid, uint16_t mmAvstand);
void USART_Tx_Tid_Avstand_PidPaadrag(USART_TypeDef *USARTx, uint32_t tid, uint16_t mmAvstand, pid_t *pid);

void USART_Tx_Tid_Avstand_Paadrag(USART_TypeDef *USARTx, uint32_t tid, uint16_t mmAvstand, uint16_t error, uint64_t U);


/* Interrupt handlers ------------------------------------------------------------*/
void USART_TXE_Handler(USART_TypeDef *USARTx);             // Transmit data register empty handler
void USART_TC_Handler(USART_TypeDef *USARTx);              // Transmission Complete handler
void USART_HandleDMA_RxComplete(USART_TypeDef *USARTx);    // DMA Receive buffer complete handler
void USART_RxDMAComplete_Callback_StyreNode(USART_TypeDef *USARTx, uint8_t *buf, uint16_t len);  // RX DMA complete callback
void USART_RxDMAComplete_Callback_SensorNode(USART_TypeDef *USARTx, uint8_t *buf, uint16_t len); 

#ifdef __cplusplus
}
#endif

#endif /* __USART_H__ */

