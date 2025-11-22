/* Includes ------------------------------------------------------------------*/
#include "usart.h"
#include "pid.h"
#include "stm32f303xc.h"
#include "stm32f3xx_ll_usart.h"
#include <stdint.h>
#include <string.h>


extern volatile uint16_t sensorNode;

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

  LL_DMA_SetDataTransferDirection(DMA1, LL_DMA_CHANNEL_6, LL_DMA_DIRECTION_PERIPH_TO_MEMORY);

  LL_DMA_SetChannelPriorityLevel(DMA1, LL_DMA_CHANNEL_6, LL_DMA_PRIORITY_LOW);

  LL_DMA_SetMode(DMA1, LL_DMA_CHANNEL_6, LL_DMA_MODE_CIRCULAR);

  LL_DMA_SetPeriphIncMode(DMA1, LL_DMA_CHANNEL_6, LL_DMA_PERIPH_NOINCREMENT);

  LL_DMA_SetMemoryIncMode(DMA1, LL_DMA_CHANNEL_6, LL_DMA_MEMORY_INCREMENT);

  LL_DMA_SetPeriphSize(DMA1, LL_DMA_CHANNEL_6, LL_DMA_PDATAALIGN_BYTE);

  LL_DMA_SetMemorySize(DMA1, LL_DMA_CHANNEL_6, LL_DMA_MDATAALIGN_BYTE);

  /* USART2 interrupt Init */
  NVIC_SetPriority(USART2_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),0, 0));
  NVIC_EnableIRQ(USART2_IRQn);


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

  /* USART3 DMA Init */

  /* USART3_RX Init */
  LL_DMA_SetDataTransferDirection(DMA1, LL_DMA_CHANNEL_3, LL_DMA_DIRECTION_PERIPH_TO_MEMORY);

  LL_DMA_SetChannelPriorityLevel(DMA1, LL_DMA_CHANNEL_3, LL_DMA_PRIORITY_LOW);

  LL_DMA_SetMode(DMA1, LL_DMA_CHANNEL_3, LL_DMA_MODE_NORMAL);

  LL_DMA_SetPeriphIncMode(DMA1, LL_DMA_CHANNEL_3, LL_DMA_PERIPH_NOINCREMENT);

  LL_DMA_SetMemoryIncMode(DMA1, LL_DMA_CHANNEL_3, LL_DMA_MEMORY_INCREMENT);

  LL_DMA_SetPeriphSize(DMA1, LL_DMA_CHANNEL_3, LL_DMA_PDATAALIGN_BYTE);

  LL_DMA_SetMemorySize(DMA1, LL_DMA_CHANNEL_3, LL_DMA_MDATAALIGN_BYTE);

  /* USART3 interrupt Init */
  NVIC_SetPriority(USART3_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),0, 0));
  NVIC_EnableIRQ(USART3_IRQn);

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
}


/* ------------------------------------------------------------------
   Interrupt-driven, buffered transmit implementation
   Uses TXE interrupt to feed bytes and TC interrupt to detect end
   of transmission so caller (e.g. ADC EOC ISR) won't block.
   Supports USART2 and USART3 instances used in this project.
   ------------------------------------------------------------------ */

#define USART_TX_BUF_SIZE 256

typedef struct {
  volatile uint8_t busy;
  uint8_t buf[USART_TX_BUF_SIZE];
  uint16_t len;
  volatile uint16_t idx;
} usart_tx_t;

static usart_tx_t tx_usart2 = {0};
static usart_tx_t tx_usart3 = {0};

static usart_tx_t* usart_get_tx(USART_TypeDef *USARTx)
{
  if (USARTx == USART2) return &tx_usart2;
  if (USARTx == USART3) return &tx_usart3;
  return NULL;
}

/* RX via DMA state */
typedef struct {
  uint8_t *buf;
  uint16_t len;
  volatile uint8_t active;
} usart_rx_dma_t;

static usart_rx_dma_t rx_usart2 = {0};
static usart_rx_dma_t rx_usart3 = {0};

/* Helper to get RX state by USART instance */
static usart_rx_dma_t* usart_get_rx(USART_TypeDef *USARTx)
{
  if (USARTx == USART2) return &rx_usart2;
  if (USARTx == USART3) return &rx_usart3;
  return NULL;
}

/* Start DMA-based RX. Uses DMA1 channels configured elsewhere (MX_DMA_Init).
 * For this project mapping is:
 *  - USART3 RX -> DMA1 Channel 3
 *  - USART2 RX -> DMA1 Channel 6
 * Caller provides a buffer that DMA will write into.
 */
int USART_StartRx_DMA(USART_TypeDef *USARTx, uint8_t *buffer, uint16_t length)
{
  if (buffer == NULL || length == 0) return -1;

  DMA_TypeDef *DMAx = DMA1;
  uint32_t channel = 0;
  usart_rx_dma_t *rx = usart_get_rx(USARTx);
  if (rx == NULL) return -1;

  if (USARTx == USART3) {
    channel = LL_DMA_CHANNEL_3;
  } else if (USARTx == USART2) {
    channel = LL_DMA_CHANNEL_6;
  } else {
    return -1;
  }

  /* Check channel not active */
  if (LL_DMA_IsEnabledChannel(DMAx, channel)) {
    return -2; /* busy */
  }

  /* Program peripheral and memory addresses and length */
  /* Peripheral address: USART RDR (data register for RX). */
  /* Clear any pending flags for this channel (safe to call) and enable TC/TE interrupts */
  if (channel == LL_DMA_CHANNEL_3) {
    LL_DMA_ClearFlag_TC3(DMA1);   
    LL_DMA_ClearFlag_TE3(DMA1);
    LL_DMA_EnableIT_TC(DMA1, LL_DMA_CHANNEL_3);
    LL_DMA_EnableIT_TE(DMA1, LL_DMA_CHANNEL_3);
  } else if (channel == LL_DMA_CHANNEL_6) {
    LL_DMA_ClearFlag_TC6(DMA1);
    LL_DMA_ClearFlag_TE6(DMA1);
    LL_DMA_EnableIT_TC(DMA1, LL_DMA_CHANNEL_6);
    LL_DMA_EnableIT_TE(DMA1, LL_DMA_CHANNEL_6);
  }

  LL_DMA_SetPeriphAddress(DMAx, channel, (uint32_t)&(USARTx->RDR));
  LL_DMA_SetMemoryAddress(DMAx, channel, (uint32_t)buffer);
  LL_DMA_SetDataLength(DMAx, channel, length);

  /* Enable USART DMA request for RX and enable DMA channel */
  LL_USART_EnableDMAReq_RX(USARTx);
  LL_DMA_EnableChannel(DMAx, channel);

  /* store state so IRQ can reference buffer if needed */
  rx->buf = buffer;
  rx->len = length;
  rx->active = 1;

  return 0;
}


/* Called by DMA IRQ handler in dma.c when a transfer completes for a USART.
 * This function retrieves the internal buffer pointer/length and calls the
 * user-overridable callback `USART_RxDMAComplete_Callback` with that info.
 */
void USART_HandleDMA_RxComplete(USART_TypeDef *USARTx)
{
  usart_rx_dma_t *rx = usart_get_rx(USARTx);
  if (rx == NULL) return;
  uint8_t *buf = rx->buf;
  uint16_t len = rx->len;
  rx->active = 0;
  if (buf != NULL && len != 0) {
    if(sensorNode)
    {
      USART_RxDMAComplete_Callback_SensorNode(USARTx, buf, len);
    }
    else
    {
      USART_RxDMAComplete_Callback_StyreNode(USARTx, buf, len);
    }
  }
}

int USART_Tx_Buffer_IT(USART_TypeDef *USARTx, uint8_t *buffer, uint16_t length)
{
  if (buffer == NULL || length == 0) return -1;
  usart_tx_t *tx = usart_get_tx(USARTx);
  if (tx == NULL) return -1;
  if (length > USART_TX_BUF_SIZE) return -3;
  if (tx->busy) return -2;

  /* Copy into internal buffer so caller can reuse/stack can pop */
  memcpy(tx->buf, buffer, length);
  tx->len = length;
  tx->idx = 0;
  tx->busy = 1;

  /* Enable TXE interrupt to start transmission (will fire when TXE=1) */
  LL_USART_EnableIT_TXE(USARTx);
  return 0;
}

// USART transmit register empty handler
void USART_TXE_Handler(USART_TypeDef *USARTx)
{
  usart_tx_t *tx = usart_get_tx(USARTx);
  if (tx == NULL) return;

  /* If there are bytes to send, write next byte. */
  if (tx->idx < tx->len) {
    LL_USART_TransmitData8(USARTx, tx->buf[tx->idx++]);
    /* If we've just written the last byte, stop TXE interrupts and enable TC */
    if (tx->idx >= tx->len) {
      LL_USART_DisableIT_TXE(USARTx);
      LL_USART_EnableIT_TC(USARTx);
    }
  } else {
    /* Nothing left: disable TXE and enable TC just in case */
    LL_USART_DisableIT_TXE(USARTx);
    LL_USART_EnableIT_TC(USARTx);
  }
}

// USART Transmission Complete handler
void USART_TC_Handler(USART_TypeDef *USARTx)
{
  usart_tx_t *tx = usart_get_tx(USARTx);
  if (tx == NULL) return;

  /* Clear TC flag and mark not busy */
  if (LL_USART_IsActiveFlag_TC(USARTx)) {
    LL_USART_ClearFlag_TC(USARTx);
  }
  tx->busy = 0;
  tx->len = 0;
  tx->idx = 0;
}

// // USART transmit 1byte
// void USART_Tx(USART_TypeDef *USARTx, uint8_t Value)
// {
//   while(!LL_USART_IsActiveFlag_TXE(USARTx))
//   {
//     LL_USART_TransmitData8(USARTx, Value);
//   }
// }

void USART_Tx_Start_Stop(USART_TypeDef *USARTx, uint8_t start_stop_byte)
{
  uint8_t dataBuffer[3];
  dataBuffer[0] = 0xAA;                       // Header startbyte
  dataBuffer[1] = start_stop_byte;            // Start/Stop byte
  dataBuffer[2] = 0x55;                       // Footer endbyte

  (void)USART_Tx_Buffer_IT(USARTx, dataBuffer, sizeof(dataBuffer));
}

void USART_Tx_Tid_Avstand(USART_TypeDef *USARTx, uint32_t tid, uint16_t mmAvstand)
{
  uint8_t dataBuffer[8];
  dataBuffer[0] = 0xAA;                     // Header startbyte
  dataBuffer[1] = tid & 0xFF;               // Least significant byte av tid (little-endian)
  dataBuffer[2] = (tid >> 8) & 0xFF;
  dataBuffer[3] = (tid >> 16) & 0xFF;
  dataBuffer[4] = (tid >> 24) & 0xFF;       // Most significant byte av tid
  dataBuffer[5] = mmAvstand & 0xFF;         // Least significant byte av mmAvstand (little-endian)
  dataBuffer[6] = (mmAvstand >> 8) & 0xFF;  // Most significant byte av mmAvstand
  dataBuffer[7] = 0x55;                     // Footer endbyte


  (void)USART_Tx_Buffer_IT(USARTx, dataBuffer, sizeof(dataBuffer));
}

void USART_Tx_Tid_Avstand_PidPaadrag(USART_TypeDef *USARTx, uint32_t tid, uint16_t mmAvstand, pid_t *pid)
{
  uint8_t dataBuffer[28];
  dataBuffer[0] = 0xAA;                       // Header startbyte
  dataBuffer[1] = tid & 0xFF;                // Least significant byte av tid (little-endian)
  dataBuffer[2] = (tid >> 8) & 0xFF;
  dataBuffer[3] = (tid >> 16) & 0xFF;
  dataBuffer[4] = (tid >> 24) & 0xFF;         
  dataBuffer[5] = mmAvstand & 0xFF;           
  dataBuffer[6] = (mmAvstand >> 8) & 0xFF;    
  dataBuffer[7] = pid->err & 0xFF;                  
  dataBuffer[8] = (pid->err >> 8) & 0xFF;
  dataBuffer[9] = (pid->err >> 16) & 0xFF;
  dataBuffer[10] = (pid->err >> 24) & 0xFF;
  dataBuffer[11] = (pid->output) & 0xFF;    
  dataBuffer[12] = (pid->output >> 8) & 0xFF;
  dataBuffer[13] = (pid->output >> 16) & 0xFF;
  dataBuffer[14] = (pid->output >> 24) & 0xFF;
  dataBuffer[15] = pid->proportional & 0xFF;
  dataBuffer[16] = (pid->proportional >> 8) & 0xFF;
  dataBuffer[17] = (pid->proportional >> 16) & 0xFF;
  dataBuffer[18] = (pid->proportional >> 24) & 0xFF;
  dataBuffer[19] = pid->integral & 0xFF;
  dataBuffer[20] = (pid->integral >> 8) & 0xFF;
  dataBuffer[21] = (pid->integral >> 16) & 0xFF;
  dataBuffer[22] = (pid->integral >> 24) & 0xFF;
  dataBuffer[23] = pid->derivative & 0xFF;
  dataBuffer[24] = (pid->derivative >> 8) & 0xFF;
  dataBuffer[25] = (pid->derivative >> 16) & 0xFF;
  dataBuffer[26] = (pid->derivative >> 24) & 0xFF;
  dataBuffer[27] = 0x55;                       // Footer endbyte

  (void)USART_Tx_Buffer_IT(USARTx, dataBuffer, sizeof(dataBuffer));
}



void USART_Tx_Tid_Avstand_Paadrag(USART_TypeDef *USARTx, uint32_t tid, uint16_t mmAvstand, uint16_t error, uint64_t U)
{
  uint8_t dataBuffer[18];
  dataBuffer[0] = 0xAA;                       // Header startbyte
  dataBuffer[1] = tid & 0xFF;                 // Least significant byte av tid (little-endian)
  dataBuffer[2] = (tid >> 8) & 0xFF;
  dataBuffer[3] = (tid >> 16) & 0xFF;
  dataBuffer[4] = (tid >> 24) & 0xFF;         // Most significant byte av tid
  dataBuffer[5] = mmAvstand & 0xFF;           // Least significant byte av mmAvstand (little-endian)
  dataBuffer[6] = (mmAvstand >> 8) & 0xFF;    // Most significant byte av mmAvstand
  dataBuffer[7] = error & 0xFF;               // Least significant byte av error (little-endian)
  dataBuffer[8] = (error >> 8) & 0xFF;        // Most significant byte av error
  dataBuffer[9] = U & 0xFF;                   // Least significant byte av U (little-endian)
  dataBuffer[10] = (U >> 8) & 0xFF;
  dataBuffer[11] = (U >> 16) & 0xFF;
  dataBuffer[12] = (U >> 24) & 0xFF;
  dataBuffer[13] = (U >> 32) & 0xFF;
  dataBuffer[14] = (U >> 40) & 0xFF;
  dataBuffer[15] = (U >> 48) & 0xFF;
  dataBuffer[16] = (U >> 56) & 0xFF;          // Most significant byte av U
  dataBuffer[17] = 0x55;                       // Footer endbyte

  (void)USART_Tx_Buffer_IT(USARTx, dataBuffer, sizeof(dataBuffer));
}


void __attribute__((weak)) USART_RxDMAComplete_Callback_StyreNode(USART_TypeDef *USARTx, uint8_t *buf, uint16_t len)
{
  /* Default empty implementation; override in application to process received buffer */
  (void)USARTx; (void)buf; (void)len;
}

void __attribute__((weak)) USART_RxDMAComplete_Callback_SensorNode(USART_TypeDef *USARTx, uint8_t *buf, uint16_t len)
{
  /* Default empty implementation; override in application to process received buffer */
  (void)USARTx; (void)buf; (void)len;
}