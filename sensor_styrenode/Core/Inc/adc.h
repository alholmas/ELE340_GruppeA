/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __ADC_H__
#define __ADC_H__
#ifdef __cplusplus
extern "C" {
#endif
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Init ADC ------------------------------------------------------------------*/
void ADC3_Init(void);

/* Metoder for ADC ------------------------------------------------------------*/
void ADC3_Calibrate(void);
void ADC3_StartConversion_TRGO(void);
void ADC3_StopConversion_TRGO(void);


/* Callbacks ------------------------------------------------------------------*/
void ADC3_EndOfConversion_Callback();

#ifdef __cplusplus
}
#endif
#endif /*__ ADC_H__ */

