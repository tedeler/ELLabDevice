#include "global.h"

#include "Arduino.h"
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_TFTLCD.h> // Hardware-specific library
#include <math.h>
#include "MyTouchScreen.h"
#include <MCUFRIEND_kbv.h>
MCUFRIEND_kbv tft;

#if defined(__SAM3X8E__)
#undef __FlashStringHelper::F(string_literal)
#define F(string_literal) string_literal
#endif

#include "ApplicationBase.h"
#include "LaborLogAmplifier.h"
#include "LaborLogAmplifierTimeChoice.h"
#include "LaborStarter.h"


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
		S_EXECSTART,
		S_EXECLAB1_TIMECHOICE,
		S_EXECLAB,
	};
	ApplicationBase::loopResult_t loopResult;
	TSPoint touchPoint;
	int Rotary1Switch;

	ApplicationBase *currentApp = 0, *oldApp;

	//Das hier sind die zur Verf�gung stehenden Apps
	LaborStarter app_start;
	LaborLogAmplifier_TimeChoice app_lab1_prelude;
	LaborLogAmplifier app_lab1;

	//Hier ist der Userinput (Rotary + TouchDisplay) drin.
	ApplicationBase::userinput_t userinput;


	//Auswahl der ersten app (Startbildschirm)
	state_t state = S_EXECSTART;
	currentApp = oldApp = &app_start;
	currentApp->init();


	while(1){
		static int r1sw_oldstate = HIGH;
		int r1sw_currentstate = digitalRead(Rotary1SWPin);
		userinput.Rotary1Switch = (r1sw_currentstate != r1sw_oldstate && r1sw_currentstate==LOW);
		r1sw_oldstate = r1sw_currentstate;

		userinput.Rotary1Counter = Rotary1Counter;

		if(state != S_EXECLAB)
			userinput.touchPoint = ReadDisplayTouch();

		loopResult = currentApp->loop(userinput);

		switch (state) {
			case S_EXECSTART:
				switch(loopResult){
				case ApplicationBase::LR_SWITCH:
					if(app_start.getChoice() == LaborStarter::LC_Labor1) {
						currentApp = &app_lab1_prelude;
						state = S_EXECLAB1_TIMECHOICE;
					}
					break;
				default:
					break;
				}
				break;
			case S_EXECLAB1_TIMECHOICE:
				switch(loopResult){
				case ApplicationBase::LR_STAY:
					break;
				case ApplicationBase::LR_EXIT:
					currentApp = &app_start;
					state = S_EXECSTART;
					break;
				case ApplicationBase::LR_SWITCH:
					currentApp = &app_lab1;
					app_lab1.setTimems( app_lab1_prelude.get_timems() );
					state = S_EXECLAB;
					break;
				}
				break;
			case S_EXECLAB:
				switch(loopResult){
				case ApplicationBase::LR_STAY:
					break;
				case ApplicationBase::LR_EXIT:
					currentApp = &app_start;
					state = S_EXECSTART;
					break;
				case ApplicationBase::LR_SWITCH:
					break;
				}
				break;
		}

		if (currentApp != oldApp) {
			currentApp->init();
			oldApp->done();
			oldApp = currentApp;
		}

	}
}
