/*
 * LaborLogAmplifierTimeChoice.cpp
 *
 *  Created on: 27.09.2019
 *      Author: torsten
 */

#include "global.h"
#include "LaborLogAmplifierTimeChoice.h"
#include <Arduino.h>
#include <MCUFRIEND_kbv.h>

extern MCUFRIEND_kbv tft;

LaborLogAmplifier_TimeChoice::LaborLogAmplifier_TimeChoice()
	:timems(0)
{
    //Darstellung PPM:
    tft.fillScreen(BLACK);    //Bildschirm schwarz
    tft.setCursor(25, 38);
    tft.setTextColor(WHITE);
    tft.setTextSize(3);
    tft.println("PPM");

    tft.setCursor(25, 85);
    tft.setTextColor(WHITE);
    tft.setTextSize(3);
    tft.println("Averaging time: ");


    tft.setCursor(35, 260);
    tft.setTextColor(WHITE);
    tft.setTextSize(3);
    tft.println("Home");
    tft.drawRect(25, 250, 88,43,WHITE);
}

LaborLogAmplifier_TimeChoice::~LaborLogAmplifier_TimeChoice() {
	// TODO Auto-generated destructor stub
}

LaborLogAmplifier_TimeChoice::loopResult_t LaborLogAmplifier_TimeChoice::loop(int Rotary1Counter, int Rotary1Switch, int Rotary2Counter, int Rotary2Switch, TSPoint touchPoint) {

    int selection = (3+Rotary1Counter % 3)%3;
    char *text[] = {"10ms", "100ms", "1000ms"};
    int value[] = {10, 100, 1000};

    for(int i=0; i<3; i++){
    	tft.setTextColor(i == selection?YELLOW:WHITE);
        tft.setCursor(25, 121+36*i);
        tft.println(text[i]);
    }

    if(Rotary1Switch) {
    	timems = value[selection];
    	return LR_SWITCH;
    }

    return LR_STAY;
}
