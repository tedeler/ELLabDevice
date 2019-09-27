#include "global.h"

#include "Arduino.h"
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_TFTLCD.h> // Hardware-specific library
#include <math.h>
#include "MyTouchScreen.h"
#include <MCUFRIEND_kbv.h>
MCUFRIEND_kbv tft;

#if defined(__SAM3X8E__)
#undef __FlashStringHelper::F(string_literal) /*??*/
#define F(string_literal) string_literal
#endif

#include "ApplicationBase.h"
#include "LaborLogAmplifier.h"
#include "LaborLogAmplifierTimeChoice.h"


// TOUCH PANEL PINS
#define YP A2  // must be an analog pin, use "An" notation!
#define XM A3  // must be an analog pin, use "An" notation!
#define YM 8   // can be a digital pin
#define XP 9   // can be a digital pin

//Min.-Koordinaten
#define TS_MINX 100
#define TS_MINY 220

//Max.-Koordinaten
#define TS_MAXX 907
#define TS_MAXY 880


#define MINPRESSURE 10    //in TouchScreen.cpp pressureThreshold
#define MAXPRESSURE 1000

TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);
volatile uint16_t ADC_xm, ADC_yp;

const int interrupt0 = 22;


const int Rotary1ClkPin = 22; //Rotary1ClkPin->
const int Rotary1DTPin = 23; //DT->
const int Rotary1SWPin = 24; //SW->
int Rotary1Counter = 0; //Define the count



int MyAnalogRead(uint8_t pin);


void DisplayStartScreen() {
	//tft.fillRect(x1, y1, x-Breite, y-Höhe, Farbe)
	tft.fillScreen(WHITE);
	for (int z = 0; z < 4; z++) {
		tft.fillRect(20, 20 + 20 * z, 60, 4, BLUEHAW1);  //Farbe: HAW hellblau
	}
	for (int z = 0; z < 4; z++) {
		tft.fillRect(36, 30 + 20 * z, 60, 4, BLUEHAW2);  //Farbe: HAW dunkelblau
	}
	tft.setCursor(114, 30);
	tft.setTextColor(BLUEHAW2);
	tft.setTextSize(3);
	tft.println("HAW");

	tft.setCursor(390, 30);
	tft.setTextColor(BLUEHAW2);
	tft.setTextSize(3);
	tft.println("V1.2");

	tft.setCursor(114, 63);
	tft.setTextColor(BLUEHAW2);
	tft.setTextSize(3);
	tft.println("HAMBURG");

	tft.fillRect(SIDE_H, 120, 250, BOXSIZE, BLUEHAW2); //X1, Y1, X-Breite, Y-Höhe, Farbe
	tft.fillRect(SIDE_H, 120 + SIDE_V + BOXSIZE, 250, BOXSIZE, BLUEHAW2);
	tft.fillRect(SIDE_H, 120 + 2 * SIDE_V + 2 * BOXSIZE, 250, BOXSIZE,
	BLUEHAW2);

	tft.setCursor(175, 130);
	tft.setTextColor(WHITE);
	tft.setTextSize(3);
	tft.println("Labor 1");

	tft.setCursor(175, 200);
	tft.setTextColor(WHITE);
	tft.setTextSize(3);
	tft.println("Labor 2");

	tft.setCursor(175, 270);
	tft.setTextColor(WHITE);
	tft.setTextSize(3);
	tft.println("Labor 3");
}

TSPoint ReadDisplayTouch() {
	digitalWrite(13, HIGH);
	TSPoint p = ts.getPoint();
	digitalWrite(13, LOW);

	pinMode(XM, OUTPUT);
	pinMode(YP, OUTPUT);

	p.z = (p.z > MINPRESSURE && p.z < MAXPRESSURE);
	p.x = map(p.x, TS_MINX, TS_MAXX, 0, tft.width());  //tft.width for 480 pixel
	p.y = map(p.y, TS_MINY, TS_MAXY, 0, tft.height()); //tft.hight for 320 pixel

	return p;
}

laborchoice_t getLaborChoiceFromTouchInput() {
	TSPoint p;
	int16_t x, y;

	p = ReadDisplayTouch();

	if (!p.z)
		return LC_None;

	x = p.x;
	y = p.y;

	if (x > 114 && x < 364) {
		if (y > 120 && y < 160) {
			return LC_Labor1;
		}
		if (y > 190 && y < 230) {
			return LC_Labor2;
		}
		if (y > 260 && y < 300) {
			return LC_Labor3;
		}
	}

	x = 0;
	y = 0;
	return LC_None;
}

void Rotary1_InterruptServiceRoutine() {
	static int lastCLK = 0; //Rotary1ClkPin initial value

	delayMicroseconds(40);
	int clkValue = digitalRead(Rotary1ClkPin); //Read the CLK pin level
	int dtValue = digitalRead(Rotary1DTPin); //Read the DT pin level
	if (lastCLK != clkValue) {
		lastCLK = clkValue;
		Rotary1Counter += (clkValue != dtValue ? 1 : -1); //Rotary1ClkPin and inconsistent DT + 1, otherwise - 1
	}
}

void setup(void) {
	Serial.begin(115200);

	tft.reset();
	uint16_t identifier = tft.readID();
	tft.begin(identifier);
	tft.setRotation(1);
	tft.fillScreen(WHITE);
	pinMode(13, OUTPUT);

	pinMode(Rotary1SWPin, INPUT_PULLUP);
	pinMode(Rotary1ClkPin, INPUT);
	pinMode(Rotary1DTPin, INPUT);
	attachInterrupt(interrupt0, Rotary1_InterruptServiceRoutine, CHANGE); //Set the interrupt 0 handler, trigger level change
	tc_setup();
	adc_setup();
}

void loop() {
	enum state_t {
		S_WRITEDISPLAY,
		S_WAIT_TOUCHINPUT,
		S_EXECLAB,
		S_EXECLAB1_TIMECHOICE,
		S_EXECLAB1
	};
	static state_t state = S_WRITEDISPLAY;
	static ApplicationBase *currentLabor;
	laborchoice_t choice = LC_None;
	ApplicationBase::loopResult_t loopResult;
	TSPoint touchPoint;
	int Rotary1Switch = (digitalRead(Rotary1SWPin) == LOW);

	if (currentLabor)
		loopResult = currentLabor->loop(Rotary1Counter, Rotary1Switch, 0, 0,
				touchPoint);

	switch (state) {
	case S_WRITEDISPLAY:
		DisplayStartScreen();

//Wenn man die drei Kommentare l�scht, dann geht er automatisch in die Anwendung mit 10ms
//		state = S_EXECLAB1;
//		currentLabor = new LaborLogAmplifier(10);
//		break;

		state = S_WAIT_TOUCHINPUT;
		break;
	case S_WAIT_TOUCHINPUT:
		choice = getLaborChoiceFromTouchInput();
		switch (choice) {
		case LC_Labor1:
			currentLabor = new LaborLogAmplifier_TimeChoice();
			state = S_EXECLAB1_TIMECHOICE;
			break;
		case LC_Labor2:
			break;
		case LC_Labor3:
			break;
		default:
			break;
		}
		break;
	case S_EXECLAB1_TIMECHOICE:
		if (loopResult == ApplicationBase::LR_EXIT) {
			delete currentLabor;
			currentLabor = 0;
			state = S_WRITEDISPLAY;
		}

		if (loopResult == ApplicationBase::LR_SWITCH) {
			LaborLogAmplifier_TimeChoice *lab =
					reinterpret_cast<LaborLogAmplifier_TimeChoice *>(currentLabor);
			int16_t timems = lab->get_timems();
			delete currentLabor;
			currentLabor = new LaborLogAmplifier(timems);
			state = S_EXECLAB1;
		}
		break;

	case S_EXECLAB1:
		if (loopResult == ApplicationBase::LR_EXIT) {
			delete currentLabor;
			currentLabor = 0;
			state = S_WRITEDISPLAY;
		}
		break;

	default:
		break;
	}
}

