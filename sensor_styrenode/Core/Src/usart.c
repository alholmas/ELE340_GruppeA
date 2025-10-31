/* Includes ------------------------------------------------------------------*/
#include "usart.h"
#include "stm32f3xx_ll_usart.h"
#include <stdint.h>


void USART2_Init(void)
{
  LL_USART_InitTypeDef USART_InitStruct = {0};

  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* Peripheral clock enable */
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_USART2);

  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
  /**USART2 GPIO Configuration
  PA2   ------> USART2_TX
  PA3   ------> USART2_RX
  */
  GPIO_InitStruct.Pin = LL_GPIO_PIN_2|LL_GPIO_PIN_3;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_7;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* USART2 interrupt Init */
  NVIC_SetPriority(USART2_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),0, 0));
  NVIC_EnableIRQ(USART2_IRQn);

  // LL_USART_EnableIT_RXNE(USART2);
  // LL_USART_EnableIT_TXE(USART2);
  // LL_USART_EnableIT_ERROR(USART2);

  USART_InitStruct.BaudRate = 115200;
  USART_InitStruct.DataWidth = LL_USART_DATAWIDTH_8B;
  USART_InitStruct.StopBits = LL_USART_STOPBITS_1;
  USART_InitStruct.Parity = LL_USART_PARITY_NONE;
  USART_InitStruct.TransferDirection = LL_USART_DIRECTION_TX_RX;
  USART_InitStruct.HardwareFlowControl = LL_USART_HWCONTROL_NONE;
  USART_InitStruct.OverSampling = LL_USART_OVERSAMPLING_16;
  LL_USART_Init(USART2, &USART_InitStruct);
  LL_USART_DisableIT_CTS(USART2);
  LL_USART_ConfigAsyncMode(USART2);
  LL_USART_Enable(USART2);
  // LL_USART_EnableDirectionTx(USART2);

}
/* USART3 init function */

void USART3_Init(void)
{
  LL_USART_InitTypeDef USART_InitStruct = {0};

  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* Peripheral clock enable */
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_USART3);

  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOD);
  /**USART3 GPIO Configuration
  PD8   ------> USART3_TX
  PD9   ------> USART3_RX
  */
  GPIO_InitStruct.Pin = LL_GPIO_PIN_8|LL_GPIO_PIN_9;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_7;
  LL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /* USART3 interrupt Init */
  NVIC_SetPriority(USART3_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),0, 0));
  NVIC_EnableIRQ(USART3_IRQn);

  LL_USART_EnableIT_RXNE(USART3);
  // LL_USART_EnableIT_TXE(USART3);
  // LL_USART_EnableIT_ERROR(USART3);


  USART_InitStruct.BaudRate = 115200;
  USART_InitStruct.DataWidth = LL_USART_DATAWIDTH_8B;
  USART_InitStruct.StopBits = LL_USART_STOPBITS_1;
  USART_InitStruct.Parity = LL_USART_PARITY_NONE;
  USART_InitStruct.TransferDirection = LL_USART_DIRECTION_TX_RX;
  USART_InitStruct.HardwareFlowControl = LL_USART_HWCONTROL_NONE;
  USART_InitStruct.OverSampling = LL_USART_OVERSAMPLING_16;
  LL_USART_Init(USART3, &USART_InitStruct);
  LL_USART_DisableIT_CTS(USART3);
  LL_USART_ConfigAsyncMode(USART3);
  LL_USART_Enable(USART3);
  LL_USART_ReceiveData8(USART3);
  // LL_USART_EnableDirectionTx(USART3);
}


void USART3_Transmit_sensorNode(uint32_t tid, uint16_t mmAvstand)
{
  uint8_t dataBuffer[8];
  dataBuffer[0] = 0xAA;                     // Header startbyte
  dataBuffer[1] = (tid >> 24) & 0xFF;       // Mest signifikante byte av tid
  dataBuffer[2] = (tid >> 16) & 0xFF;
  dataBuffer[3] = (tid >> 8) & 0xFF;
  dataBuffer[4] = tid & 0xFF;               // Minst signifikante byte av tid
  dataBuffer[5] = (mmAvstand >> 8) & 0xFF;  // Mest signifikante byte av mmAvstand
  dataBuffer[6] = mmAvstand & 0xFF;         // Minst signifikante byte av mmAvstand
  dataBuffer[7] = 0x55;                     // Footer endbyte

  for (int i = 0; i < 8; i++)
  {
    while (!LL_USART_IsActiveFlag_TXE(USART3)); // Vent til transmit buffer er tom
    LL_USART_TransmitData8(USART3, dataBuffer[i]);
  } 
}



void USART2_Transmit_styreNode(uint32_t tid, uint16_t mmAvstand, uint16_t mmAvik)
{
  uint8_t dataBuffer[10];
  dataBuffer[0] = 0xAA;                       // Header startbyte
  dataBuffer[1] = (tid >> 24) & 0xFF;         // Mest signifikante byte av tid
  dataBuffer[2] = (tid >> 16) & 0xFF;
  dataBuffer[3] = (tid >> 8) & 0xFF;
  dataBuffer[4] = tid & 0xFF;                 // Minst signifikante byte av tid
  dataBuffer[5] = (mmAvstand >> 8) & 0xFF;    // Mest signifikante byte av mmAvstand
  dataBuffer[6] = mmAvstand & 0xFF;           // Minst signifikante byte av mmAvstand
  dataBuffer[7] = (mmAvik >> 8) & 0xFF;       // Mest signifikante byte av mmAvik
  dataBuffer[8] = mmAvik & 0xFF;              // Minst signifikante byte av mmAvik
  dataBuffer[9] = 0x55;                       // Footer endbyte


  for (int i = 0; i < 10; i++)
  {
    while (!LL_USART_IsActiveFlag_TXE(USART2)); // Vent til transmit buffer er tom
    LL_USART_TransmitData8(USART2, dataBuffer[i]);
  } 
}

// uint8_t[] USART3_Recive_StyreNode(void)
//   {
//     uint8_t receivedData[8];
//     for (int i = 0; i < 8; i++)
//     {
//       while (!LL_USART_IsActiveFlag_RXNE(USART3)); // Vent til data er mottatt
//       receivedData[i] = LL_USART_ReceiveData8(USART3);
//     }
//     return receivedData;
//   }



void __attribute__((weak)) USART2_RxReady_Callback(void)
{
  // Tom default-implementasjon; kan overstyres ved å definere samme funksjon uten __weak i en annen .c-fil
}

void __attribute__((weak)) USART3_RxReady_Callback(void)
{
  // Tom default-implementasjon; kan overstyres ved å definere samme funksjon uten __weak i en annen .c-fil
} 

