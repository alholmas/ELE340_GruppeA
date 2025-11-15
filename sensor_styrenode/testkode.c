      static volatile uint8_t test_flag = 0;
    /* one-shot trigger for the tid==10 event to avoid repeated firing */
    static volatile uint8_t trig_10 = 0;
      
      // test-funksjon: kjør handling ved tid==10 annenhver gang
      // bruk en one-shot trigger slik at handlingen ikke repeteres ved
      // flere innkommende pakker med samme tid. Når trig_10 er 0 og
      // tid == 10 utføres handling og trig_10 settes til 1. Når testen
      // avsluttes (tid ruller tilbake) resettes trig_10.
      if (tid < 5) {
        trig_10 = 0; // ny test/start
      }

      if (tid == 10 && !trig_10) {
        if (!test_flag) {
          // første type kjøring
          LL_GPIO_ResetOutputPin(DIR_GPIO_Port, DIR_Pin);
          TIM3_SetFreq(100000);
        } else {
          // annen type kjøring
          LL_GPIO_SetOutputPin(DIR_GPIO_Port, DIR_Pin);
          TIM3_SetFreq(100000);
        }
        // toggle så neste gang gjør den andre handling
        test_flag = !test_flag;
        trig_10 = 1;
      }

      if(tid == 110) 
      {
        TIM3_SetFreq(0);
      }
      if(tid == 150) 
      {
        USART_Tx_Start_Stop(USART3, 0); //stopper sensorNode
      }