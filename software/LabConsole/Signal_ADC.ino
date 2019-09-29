//Interfacing Variables



unsigned long ADCSampleCount = 0;
uint16_t ADCValue_BNC1 = 0;
bool ADCReady_BNC1 = false;


/*************  Configure ADC function  *******************/
void adc_setup() {
  PMC->PMC_PCER1 |= PMC_PCER1_PID37;                    // ADC power on
  ADC->ADC_CR     = ADC_CR_SWRST;                       // Reset ADC P.1344
  ADC->ADC_MR    |= ADC_MR_TRGEN_EN |                   // Hardware trigger select
                    ADC_MR_TRGSEL_ADC_TRIG3 |           // Trigger by TIOA2 P. 1345
                    ADC_MR_LOWRES_BITS_10;              // 10-bit resolution P. 1345; without this line 12-bit
  ADC->ADC_IER =  ADC_IER_EOC10;                        // Interrupt on End Of Conversion for channel 10 = A8, wer, wie, wo ???        
        
  NVIC_EnableIRQ(ADC_IRQn);                             // Enable ADC interrupts
  ADC->ADC_CHER = ADC_CHER_CH10;                        // Enable ADC CH10 = A8
}

void ADC_Handler() {
	uint16_t ch10value = ADC->ADC_CDR[10];
//	ADC_xm = ADC->ADC_CDR[4];   // A3  = xm (CH4)
//	ADC_yp = ADC->ADC_CDR[5];   // A2 = yp (CH5)



	if (ADCSampleCount == 20) {
	  ADCSampleCount = 0;
	  ADCValue_BNC1 = ch10value;
	  ADCReady_BNC1 = true;
	}

	ADCSampleCount++;
}

int MyAnalogRead(uint8_t pin){
  unsigned long result;
  unsigned int channelnr;

  if(pin == YP)
	  channelnr = 5;
  if(pin == XM)
	  channelnr = 4;

  ADC->ADC_CHER = (1<<channelnr);

  while((ADC->ADC_ISR & (1<<channelnr)) == 0);

  result = ADC->ADC_CDR[channelnr];

  ADC->ADC_CHDR |= ADC_CHDR_CH4 |
                   ADC_CHDR_CH5;

  return result;
}

int MyAnalogRead1(uint8_t pin){
  unsigned long result;
  if(pin == YP){
    ADC->ADC_CHER = ADC_CHER_CH5;
  }
  if(pin == XM){
    ADC->ADC_CHER = ADC_CHER_CH4;
  }
  delayMicroseconds(40);
  #if(debug)  
      Serial.print("* ");
      Serial.print(pin);
  #endif
  if(pin == YP){
      result = ADC_yp;
      #if(debug)
          Serial.print(" ADC_yp: ");
          Serial.println(ADC_yp);
      #endif    
  }

  if(pin == XM){
      result = ADC_xm;
      #if(debug)
          Serial.print(" ADC_xm: ");
          Serial.println(ADC_xm);
      #endif    
 }

  ADC->ADC_CHDR |= ADC_CHDR_CH4 |
                   ADC_CHDR_CH5;
  return result;
}

/*************  Timer Counter 0 Channel 2 to generate PWM pulses thru TIOA2  ************/
void tc_setup() {
  PMC->PMC_PCER0 |= PMC_PCER0_PID29;                       // TC2 power ON : Timer Counter 0 channel 2 IS TC2
  TC0->TC_CHANNEL[2].TC_CMR = TC_CMR_TCCLKS_TIMER_CLOCK1   // MCK/2, clk on rising edge
//    TC0->TC_CHANNEL[2].TC_CMR = TC_CMR_TCCLKS_TIMER_CLOCK4   // MCK/128, clk on rising edge
                              | TC_CMR_WAVE                // Waveform mode
                              | TC_CMR_WAVSEL_UP_RC        // UP mode with automatic trigger on RC Compare
                              | TC_CMR_ACPA_CLEAR          // Clear TIOA2 on RA compare match
                              | TC_CMR_ACPC_SET;           // Set TIOA2 on RC compare match


  TC0->TC_CHANNEL[2].TC_RC = 952;  //<*********************  Frequency = (Mck/2)/TC_RC  Hz = 44,1 KHz
  TC0->TC_CHANNEL[2].TC_RA = 400;  //<********************   Any Duty cycle in between 1 and 951

  TC0->TC_CHANNEL[2].TC_CCR = TC_CCR_SWTRG | TC_CCR_CLKEN; // Software trigger TC2 counter and enable
}

