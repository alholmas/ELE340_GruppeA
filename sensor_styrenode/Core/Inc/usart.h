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
void USART2_Transmit_styreNode(uint32_t tid, uint16_t mmAvstand, uint16_t mmAvik);
void USART3_Transmit_sensorNode(uint32_t tid, uint16_t mmAvstand);
// uint8_t USART3_Recive_StyreNode(void);
void USART2_RxReady_Callback();
void USART3_RxReady_Callback();


// volatile uint8_t usart2_rx_data[8];
// volatile uint8_t usart3_rx_data[8];
// voltile uint8_t usart2_data_ready;
// volatile uint8_t usart3_data_ready;
#ifdef __cplusplus
}
#endif

#endif /* __USART_H__ */

