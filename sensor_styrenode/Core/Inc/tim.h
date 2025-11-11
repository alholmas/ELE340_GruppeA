#ifndef __TIM_H__
#define __TIM_H__

#ifdef __cplusplus
extern "C" {
#endif
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Init timer ----------------------------------------------------------------*/
void TIM3_Init(void);
void TIM4_Init(void);
void TIM7_Init(void);

/* Metoder for TIM ------------------------------------------------------------*/
void TIM3_SetFreq(uint32_t freq_hz);
void TIM3_Stop_PWM(void);

void TIM4_Start_PWM(void);
void TIM4_Stopp_PWM(void);

void TIM7_Start_TRGO(void);
void TIM7_Stopp_TRGO(void);




#ifdef __cplusplus
 }
 #endif

 #endif /* __TIM_H__ */
