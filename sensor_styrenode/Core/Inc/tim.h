#ifndef __TIM_H__
#define __TIM_H__

#ifdef __cplusplus
extern "C" {
#endif
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Init timer ----------------------------------------------------------------*/
void TIM4_Init(void);
void TIM7_Init(void);

/* Metoder for TIM ------------------------------------------------------------*/
void TIM4_Start(void);
void TIM4_Stopp(void);
void TIM7_Start(void);
void TIM7_Stopp(void);


#ifdef __cplusplus
 }
 #endif

 #endif /* __TIM_H__ */
