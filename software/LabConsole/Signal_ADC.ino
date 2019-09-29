//Interfacing Variables


#include <Arduino.h>

unsigned long ADCSampleCount = 0;
uint16_t ADCValue_BNC1 = 0;
bool ADCReady_BNC1 = false;

TSPoint touch(uint16_t xm, uint16_t yp);


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
  ADC_xm = ADC->ADC_CDR[4];   // A3  = xm (CH4)
  ADC_yp = ADC->ADC_CDR[5];   // A2 = yp (CH5)



  if (ADCSampleCount == 20) {
	  ADCSampleCount = 0;
	  ADCValue_BNC1 = ADC->ADC_CDR[10];
	  ADCReady_BNC1 = true;
  }

  TSPoint touchPoint;
  touchPoint = touch(ADC_xm, ADC_yp);

  ADCSampleCount++;
}

TSPoint touch(uint16_t xm, uint16_t yp) {
	TSPoint result(0,0,0);

	const uint8_t _yp=A2, _ym=8, _xm=A3, _xp=9;
	const uint16_t _rxplate=300;

	static int x, y, z;
	int samples[2];

	static int state = 0;

	switch(state) {
	case 0:
		pinMode(_ym, INPUT);
		break;
	case 55:
		pinMode(_yp, INPUT);
		pinMode(_ym, INPUT);
		digitalWrite(_yp, LOW);
		digitalWrite(_ym, LOW);
		pinMode(_xp, OUTPUT);
		pinMode(_xm, OUTPUT);
		digitalWrite(_xp, HIGH);
		digitalWrite(_xm, LOW);
		state++;
		state = 0;
		break;
	case 1:
		samples[0] = yp;
		state++;
		break;
	case 2:
		samples[1] = yp;
		x = (1023-samples[0]);
		if (samples[0]>>2 != samples[1]>>2)
			state = 0;
		else
			state++;
		break;
	case 3:
		pinMode(_xp, INPUT);
		pinMode(_xm, INPUT);
		digitalWrite(_xp, LOW);
		pinMode(_yp, OUTPUT);
		digitalWrite(_yp, HIGH);
		pinMode(_ym, OUTPUT);
		digitalWrite(_ym, LOW);
		state++;
		break;
	case 4:
		samples[0] = xm;
		state++;
		break;
	case 5:
		samples[1] = xm;
		y = (1023-samples[0]);
		if (samples[0]>>2 != samples[1]>>2)
			state = 0;
		else
			state++;
		break;
	case 6:
		pinMode(_xp, OUTPUT);
		digitalWrite(_xp, LOW);
		digitalWrite(_ym, HIGH);
		digitalWrite(_yp, LOW);
		pinMode(_yp, INPUT);
		state++;
		break;
	case 7:
	{
		int z1=xm; int z2=yp;
		float rtouch;
		rtouch = z2;
		rtouch /= z1;
		rtouch -= 1;
		rtouch *= x;
		rtouch *= _rxplate;
		rtouch /= 1024;
		z = rtouch;
		result.x = x;
		result.y = y;
		result.z = z;
		state=0;
		break;
	}
	default:
		state = 0;
	}

	return result;
}

int MyAnalogRead(uint8_t pin){
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

