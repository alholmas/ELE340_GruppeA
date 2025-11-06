#include "tim.h"
#include "stm32f303xc.h"
#include "stm32f3xx_ll_gpio.h"
#include "stm32f3xx_ll_rcc.h"
#include "stm32f3xx_ll_tim.h"
#include <stdint.h>

void TIM3_Init(void)
{
  LL_TIM_InitTypeDef TIM_InitStruct = {0};
  LL_TIM_OC_InitTypeDef TIM_OC_InitStruct = {0};

  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* Peripheral clock enable */
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM3);


  TIM_InitStruct.Prescaler = 143; /* 72 MHz / 720 = 100 kHz timer clock */
  TIM_InitStruct.CounterMode = LL_TIM_COUNTERMODE_UP;
  TIM_InitStruct.Autoreload = 1;
  TIM_InitStruct.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1;
  LL_TIM_Init(TIM3, &TIM_InitStruct);
  LL_TIM_DisableARRPreload(TIM3);
  LL_TIM_SetClockSource(TIM3, LL_TIM_CLOCKSOURCE_INTERNAL);
  LL_TIM_OC_EnablePreload(TIM3, LL_TIM_CHANNEL_CH1);
  TIM_OC_InitStruct.OCMode = LL_TIM_OCMODE_PWM1;
  TIM_OC_InitStruct.OCState = LL_TIM_OCSTATE_ENABLE;
  TIM_OC_InitStruct.OCNState = LL_TIM_OCSTATE_DISABLE;
  
  TIM_OC_InitStruct.CompareValue = 1; /* 50% duty cycle */
  TIM_OC_InitStruct.OCPolarity = LL_TIM_OCPOLARITY_HIGH;
  
  // Buffer for ARR og CCR1 registers
  /* Enable ARR/CCR preload for TIM3 */
  LL_TIM_EnableARRPreload(TIM3);
  LL_TIM_OC_EnablePreload(TIM3, LL_TIM_CHANNEL_CH1);


  LL_TIM_OC_Init(TIM3, LL_TIM_CHANNEL_CH1, &TIM_OC_InitStruct);
  LL_TIM_OC_DisableFast(TIM3, LL_TIM_CHANNEL_CH1);
  LL_TIM_SetTriggerOutput(TIM3, LL_TIM_TRGO_RESET);
  LL_TIM_DisableMasterSlaveMode(TIM3);


  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);
    /**TIM3 GPIO Configuration
    PB4     ------> TIM3_CH1
    */
  GPIO_InitStruct.Pin = LL_GPIO_PIN_4;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_2;
  LL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

void TIM3_Start_PWM(void)
{
  LL_TIM_EnableCounter(TIM3);
}

void TIM3_Stop_PWM(void)
{
  LL_TIM_DisableCounter(TIM3);
}

int TIM3_SetFrequencyHz(uint32_t freq_hz)
{
    /*
     * Sett TIM3 slik at PWM-frekvensen blir så nær som mulig den etterspurte
     * `freq_hz` med 50% duty cycle. Maks frekvens begrenses til 250 kHz.
     * Denne funksjonen autokalibrerer prescaler (PSC) og auto-reload (ARR)
     * for best mulig oppløsning (velger lavest mulig PSC som gir gyldig ARR).
     *
     * Returnerer 0 ved suksess, -1 ved ugyldig input eller hvis det ikke
     * er mulig å oppnå frekvensen med 16-bit PSC/ARR.
     */

    if (freq_hz == 0) {
        return -1;
    }

    const uint32_t MAX_FREQ_HZ = 250000UL;
    if (freq_hz > MAX_FREQ_HZ) {
        freq_hz = MAX_FREQ_HZ; // clamp til maks
    }

    /* Beregn timerklokke for TIM3. Timerclock = PCLK1 * (APB1 prescale == 1 ? 1 : 2)
     * Vi bruker SystemCoreClock og RCC->CFGR for å finne APB1-prescaler.
     */
    uint32_t sysclk = SystemCoreClock;
    uint32_t ppre1 = (RCC->CFGR & RCC_CFGR_PPRE1) >> RCC_CFGR_PPRE1_Pos;
    uint32_t pclk1;
    if (ppre1 < 4) {
        /* 0xx => HCLK not divided */
        pclk1 = sysclk;
    } else {
        /* 100 => divide by 2, 101 => /4, 110 => /8, 111 => /16 */
        uint32_t div = 2U << (ppre1 - 4);
        pclk1 = sysclk / div;
    }
    /* Når APB prescaler != 1, timer klokker multipliseres med 2 på STM32 */
    uint32_t tim_clk = pclk1;
    if (ppre1 >= 4) {
        tim_clk = pclk1 * 2U;
    }

    /* Vi søker etter minste PSC (dvs. størst oppløsning) som gir ARR innen 16-bit */
    const uint32_t MAX_ARR = 0xFFFFU;
    uint32_t found_psc = 0xFFFFFFFFU;
    uint32_t found_arr = 0xFFFFFFFFU;

    for (uint32_t psc = 0; psc <= 0xFFFFU; ++psc) {
        /* arr+1 = tim_clk / (freq*(psc+1))  -> arr = (tim_clk/(freq*(psc+1))) - 1 */
        uint64_t denom = (uint64_t)freq_hz * (uint64_t)(psc + 1U);
        if (denom == 0) continue;
        uint64_t arr_plus1 = (uint64_t)tim_clk / denom;
        if (arr_plus1 == 0) continue; /* for høy frekvens / stor psc */
        uint64_t arr = arr_plus1 - 1ULL;
        if (arr <= MAX_ARR) {
            found_psc = psc;
            found_arr = (uint32_t)arr;
            break; /* første løsning gir lavest PSC */
        }
    }

    if (found_psc == 0xFFFFFFFFU) {
        /* Fant ingen gyldig kombinasjon */
        return -1;
    }

    /* Sett PSC/ARR/CCR for TIM3 kanal 1 (50% duty) */
    LL_TIM_DisableCounter(TIM3);
    LL_TIM_SetPrescaler(TIM3, found_psc);
    LL_TIM_SetAutoReload(TIM3, found_arr);
    uint32_t ccr = (found_arr + 1U) / 2U; /* 50% duty */
    LL_TIM_OC_SetCompareCH1(TIM3, ccr);

    /* Oppdater preload ved å generere update event */
    LL_TIM_GenerateEvent_UPDATE(TIM3);
    LL_TIM_EnableCounter(TIM3);

    return 0;
}


/* Variant som lar brukeren spesifisere prescaler direkte. prescaler er PSC
 * register-verdi (0..65535). Funksjonen setter ARR for å forsøke å oppnå
 * ønsket frekvens og returnerer 0 ved suksess, -1 ved feil.
 */
int TIM3_SetFrequencyHzWithPrescaler(uint32_t freq_hz, uint32_t prescaler)
{
    if (freq_hz == 0) return -1;
    if (freq_hz > 250000U) freq_hz = 250000U;
    if (prescaler > 0xFFFFU) return -1;

    uint32_t sysclk = SystemCoreClock;
    uint32_t ppre1 = (RCC->CFGR & RCC_CFGR_PPRE1) >> RCC_CFGR_PPRE1_Pos;
    uint32_t pclk1;
    if (ppre1 < 4) {
        pclk1 = sysclk;
    } else {
        uint32_t div = 2U << (ppre1 - 4);
        pclk1 = sysclk / div;
    }
    uint32_t tim_clk = pclk1;
    if (ppre1 >= 4) tim_clk = pclk1 * 2U;

    const uint32_t MAX_ARR = 0xFFFFU;
    uint64_t denom = (uint64_t)freq_hz * (uint64_t)(prescaler + 1U);
    if (denom == 0) return -1;
    uint64_t arr_plus1 = (uint64_t)tim_clk / denom;
    if (arr_plus1 == 0) return -1;
    uint64_t arr = arr_plus1 - 1ULL;
    if (arr > MAX_ARR) return -1;

    LL_TIM_DisableCounter(TIM3);
    LL_TIM_SetPrescaler(TIM3, prescaler);
    LL_TIM_SetAutoReload(TIM3, (uint32_t)arr);
    uint32_t ccr = ((uint32_t)arr + 1U) / 2U;
    LL_TIM_OC_SetCompareCH1(TIM3, ccr);
    LL_TIM_GenerateEvent_UPDATE(TIM3);
    LL_TIM_EnableCounter(TIM3);

    return 0;
}




void TIM4_Init(void)
{  

  // Enable clock for TIM4
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM4);

  TIM4->CCMR1 = 0x68;            // = 0b01101000
  TIM4->CCER = 0x01;
  TIM4->PSC = 71;                // = 71, altså 72. Det gir f_TIM4 = 1 MHz og T_TIM4 = 1 us.
  TIM4->ARR = 249;               // = 249            (altså til 250 us, f = 4 kHz)
  TIM4->CCR1 = 125;              // = 125            (altså til 125, halvparten D = 50%)

  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOD);
    /**TIM4 GPIO Configuration
    PD12     ------> TIM4_CH1
    */
  GPIO_InitStruct.Pin = LL_GPIO_PIN_12;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_2;
  LL_GPIO_Init(GPIOD, &GPIO_InitStruct);
}

void TIM4_Start_PWM(void)
{
  LL_TIM_EnableCounter(TIM4);
}

void TIM4_Stopp_PWM(void)
{
  LL_TIM_DisableCounter(TIM4);
}


void TIM7_Init(void)
{
  LL_TIM_InitTypeDef TIM_InitStruct = {0};

  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM7);

  TIM_InitStruct.Prescaler = 7199; /* 72 000 000 / 7200 = 10000 Hz */
  TIM_InitStruct.CounterMode = LL_TIM_COUNTERMODE_UP;
  TIM_InitStruct.Autoreload = 99; /* 10000 / 100 = 100 Hz -> 10 ms period */
  LL_TIM_Init(TIM7, &TIM_InitStruct);
  LL_TIM_DisableARRPreload(TIM7);
  LL_TIM_SetTriggerOutput(TIM7, LL_TIM_TRGO_UPDATE);
  LL_TIM_DisableMasterSlaveMode(TIM7);

}

void TIM7_Start_TRGO(void)
{
  LL_TIM_EnableCounter(TIM7);
}

void TIM7_Stopp_TRGO(void)
{
  LL_TIM_DisableCounter(TIM7);
}
