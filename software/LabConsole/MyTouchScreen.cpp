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

TSPoint TouchScreen::getPoint(void) {
  unsigned long start = micros();


  int x, y, z;
  int samples[NUMSAMPLES];
  uint8_t valid;

  valid = 1;

  pinMode(_yp, INPUT);
  pinMode(_ym, INPUT);
  
  digitalWrite(_yp, LOW);
  digitalWrite(_ym, LOW);
   
  pinMode(_xp, OUTPUT);
  pinMode(_xm, OUTPUT);
  digitalWrite(_xp, HIGH);
  digitalWrite(_xm, LOW);
  samples[0] = MyAnalogRead(_yp);
  samples[1] = MyAnalogRead(_yp);
  if (samples[0]>>2 != samples[1]>>2)
	  valid = 0;
  x = (1023-samples[1]);

   pinMode(_xp, INPUT);
   pinMode(_xm, INPUT);
//   *portOutputRegister(xp_port) &= ~xp_pin;
   digitalWrite(_xp, LOW);
   
   pinMode(_yp, OUTPUT);
//   *portOutputRegister(yp_port) |= yp_pin;
   digitalWrite(_yp, HIGH);
   pinMode(_ym, OUTPUT);
//   *portOutputRegister(ym_port) &= ~yp_pin;
   digitalWrite(_ym, LOW);

   samples[0] = MyAnalogRead(_xm);
   samples[1] = MyAnalogRead(_xm);

   if (samples[0]>>2 != samples[1]>>2)
	   valid = 0;

   y = (1023-samples[1]);

   // Set X+ to ground
   pinMode(_xp, OUTPUT);
//   *portOutputRegister(xp_port) &= ~xp_pin;
   digitalWrite(_xp, LOW);
  
   // Set Y- to VCC
//   *portOutputRegister(ym_port) |= ym_pin;
   digitalWrite(_ym, HIGH); 
  
   // Hi-Z X- and Y+
//   *portOutputRegister(yp_port) &= ~yp_pin;
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
     rtouch *= x;
     rtouch *= _rxplate;
     rtouch /= 1024;
     
     z = rtouch;
   } else {
     z = (1023-(z2-z1));
   }

   if (! valid) {
     z = 0;
   }

   Serial.println(micros() - start);
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
