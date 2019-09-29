// Touch screen library with X Y and Z (pressure) readings as well
// as oversampling to avoid 'bouncing'
// (c) ladyada / adafruit
// Code under MIT License

#include "pins_arduino.h"
#include "wiring_private.h"
//#if !defined(__SAM3X8E__)
//	#include <include/pio.h>
//	typedef Pio* Port_t;
//#else
//	#include <avr/pgmspace.h>
//	typedef uint8_t Port_t;
//#endif
#include "MyTouchScreen.h"
#include "global.h"

// increase or decrease the touchscreen oversampling. This is a little different than you make think:
// 1 is no oversampling, whatever data we get is immediately returned
// 2 is double-sampling and we only return valid data if both points are the same
// 3+ uses insert sort to get the median value.
// We found 2 is precise yet not too slow so we suggest sticking with it!

#define NUMSAMPLES 2
int MyAnalogRead(uint8_t pin);
TSPoint::TSPoint(void) {
  x = y = 0;
}

TSPoint::TSPoint(int16_t x0, int16_t y0, int16_t z0) {
  x = x0;
  y = y0;
  z = z0;
}

bool TSPoint::operator==(TSPoint p1) {
  return  ((p1.x == x) && (p1.y == y) && (p1.z == z));
}

bool TSPoint::operator!=(TSPoint p1) {
  return  ((p1.x != x) || (p1.y != y) || (p1.z != z));
}

#if (NUMSAMPLES > 2)
static void insert_sort(int array[], uint8_t size) {
  uint8_t j;
  int save;
  
  for (int i = 1; i < size; i++) {
    save = array[i];
    for (j = i; j >= 1 && save < array[j - 1]; j--)
      array[j] = array[j - 1];
    array[j] = save; 
  }
}
#endif

#define OUTLOW(p) p.pPort->PIO_OER = p.ulPin; p.pPort->PIO_CODR = p.ulPin;
#define OUTHIGH(p) p.pPort->PIO_OER = p.ulPin; p.pPort->PIO_SODR = p.ulPin;
#define PININPUT(p) p.pPort->PIO_ODR = p.ulPin

TSPoint TouchScreen::getPoint(void) {
	unsigned long start = micros();
	const PinDescription p_YP = g_APinDescription[YP];
	const PinDescription p_YM = g_APinDescription[YM];
	const PinDescription p_XP = g_APinDescription[XP];
	const PinDescription p_XM = g_APinDescription[XM];
	static int state = 0;
	static int x, y, z;
	static int samples[2];
	static uint8_t valid;
	static int z1;
	static int z2;


	Serial.print(state);
	Serial.print(": ");

	switch(state) {
	case 0:
		state++;
		break;
	case 1:
		PININPUT(p_YP);
		PININPUT(p_YM);
		OUTHIGH(p_XP);
		OUTLOW(p_XM);
		samples[0] = MyAnalogRead(_yp);
		samples[1] = MyAnalogRead(_yp);
		x = (1023-samples[1]);
		if (samples[0]>>2 != samples[1]>>2)
			state = 0;
		else
			state++;
		break;
	case 2:
		PININPUT(p_XP);
		PININPUT(p_XM);
		OUTHIGH(p_YP);
		OUTLOW(p_YM);

		samples[0] = MyAnalogRead(_xm);
		samples[1] = MyAnalogRead(_xm);
		y = (1023-samples[1]);

		if (samples[0]>>2 != samples[1]>>2)
			state = 0;
		else
			state++;
		break;
	case 3:
		OUTLOW(p_XP);
		OUTHIGH(p_YM);
		PININPUT(p_YP);
		PININPUT(p_XM);

		z1 = MyAnalogRead(_xm);
		z2 = MyAnalogRead(_yp);

		if (_rxplate != 0) {
			// now read the x
			float rtouch;
			rtouch = z2;
			rtouch /= z1;
			rtouch -= 1;
			rtouch *= x;
			rtouch *= _rxplate;
			rtouch /= 1024;

			z = rtouch;
		} else {
			z = (1023-(z2-z1));
		}

		state = 0;
		break;
	default:
		state = 0;
		break;
	}



	OUTLOW(p_XP);
	OUTHIGH(p_YM);
	PININPUT(p_YP);
	PININPUT(p_XM);



	unsigned long stop = micros();

	Serial.println(stop - start);
	if(z>0)
	{
		Serial.print(" -- ");
		Serial.print(x); Serial.print(", ");
		Serial.print(y); Serial.print(", ");
		Serial.print(z); Serial.println("");
	}
	return TSPoint(x, y, z);
}

TouchScreen::TouchScreen(uint8_t xp, uint8_t yp, uint8_t xm, uint8_t ym) {
  _yp = yp;
  _xm = xm;
  _ym = ym;
  _xp = xp;
  _rxplate = 0;
  pressureThreshhold = 10;
}


TouchScreen::TouchScreen(uint8_t xp, uint8_t yp, uint8_t xm, uint8_t ym,
			 uint16_t rxplate) {
  _yp = yp;
  _xm = xm;
  _ym = ym;
  _xp = xp;
  _rxplate = rxplate;

  pressureThreshhold = 10;
}

int TouchScreen::readTouchX(void) {
   pinMode(_yp, INPUT);
   pinMode(_ym, INPUT);
   digitalWrite(_yp, LOW);
   digitalWrite(_ym, LOW);
   
   pinMode(_xp, OUTPUT);
   digitalWrite(_xp, HIGH);
   pinMode(_xm, OUTPUT);
   digitalWrite(_xm, LOW);
   
   return (1023-MyAnalogRead(_yp));
}


int TouchScreen::readTouchY(void) {
   pinMode(_xp, INPUT);
   pinMode(_xm, INPUT);
   digitalWrite(_xp, LOW);
   digitalWrite(_xm, LOW);
   
   pinMode(_yp, OUTPUT);
   digitalWrite(_yp, HIGH);
   pinMode(_ym, OUTPUT);
   digitalWrite(_ym, LOW);
   
   return (1023-MyAnalogRead(_xm));
}


uint16_t TouchScreen::pressure(void) {
  // Set X+ to ground
  pinMode(_xp, OUTPUT);
  digitalWrite(_xp, LOW);
  
  // Set Y- to VCC
  pinMode(_ym, OUTPUT);
  digitalWrite(_ym, HIGH); 
  
  // Hi-Z X- and Y+
  digitalWrite(_xm, LOW);
  pinMode(_xm, INPUT);
  digitalWrite(_yp, LOW);
  pinMode(_yp, INPUT);
  
  int z1 = MyAnalogRead(_xm); 
  int z2 = MyAnalogRead(_yp);

  if (_rxplate != 0) {
    // now read the x 
    float rtouch;
    rtouch = z2;
    rtouch /= z1;
    rtouch -= 1;
    rtouch *= readTouchX();
    rtouch *= _rxplate;
    rtouch /= 1024;
    
    return rtouch;
  } else {
    return (1023-(z2-z1));
  }
}
