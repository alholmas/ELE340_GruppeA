/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __GPIO_H__
#define __GPIO_H__
#ifdef __cplusplus
extern "C" {
#endif
/* Includes ------------------------------------------------------------------*/
#include "main.h"
/* Init GPIO -----------------------------------------------------------------*/
void GPIO_Init(void);

/* Metoder for GPIO -----------------------------------------------------------*/

/* Callbacks ------------------------------------------------------------------*/
void USER_BUTTON_Callback(void);
void SW1_Callback(void);

#ifdef __cplusplus
}
#endif
#endif /*__ GPIO_H__ */

