/*
 * LaborLogAmplifier.cpp
 *
 *  Created on: 27.09.2019
 *      Author: torsten
 */

#include "global.h"
#include "LaborLogAmplifier.h"
#include <Arduino.h>
#include <MCUFRIEND_kbv.h>

extern MCUFRIEND_kbv tft;

extern uint16_t ADCValue_BNC1;
extern bool ADCReady_BNC1;


LaborLogAmplifier::LaborLogAmplifier(int16_t timems) :
		timems(timems) {
	//Darstellung PPM:
	tft.fillScreen(BLACK);    //Bildschirm schwarz

	//Anzeigelinien der dB-Werte:
	for (int z = 0; z < 47; z++) {
		if (z != 4 && z != 14 && z != 24 && z != 34 && z != 44) {
			tft.fillRect(13 + 10 * z, 155, 3, 10, WHITE);
		} else if (z == 4 || z == 14 || z == 24) {
			tft.fillRect(13 + 10 * z, 155, 3, 10, BLUE1);
		} else if (z == 34) {
			tft.fillRect(13 + 10 * z, 155, 3, 10, YELLOW);
		} else if (z == 44) {
			tft.fillRect(13 + 10 * z, 155, 3, 10, RED);
		}
	}
	tft.fillRect(10, 120, 7, 30, BLUE1);   //ersten Balken blau anzeigen
	tft.setCursor(32, 175);
	tft.setTextColor(BLUE1);
	tft.setTextSize(2);
	tft.println("-30");

	tft.setCursor(132, 175);
	tft.setTextColor(BLUE1);
	tft.setTextSize(2);
	tft.println("-20");

	tft.setCursor(232, 175);
	tft.setTextColor(BLUE1);
	tft.setTextSize(2);
	tft.println("-10");

	tft.setCursor(349, 175);
	tft.setTextColor(YELLOW);
	tft.setTextSize(2);
	tft.println("0");

	tft.setCursor(432, 175);
	tft.setTextColor(RED);
	tft.setTextSize(2);
	tft.println("+10");

	tft.setCursor(13, 94);
	tft.setTextColor(WHITE);
	tft.setTextSize(2);
	tft.println("dB");

}

LaborLogAmplifier::~LaborLogAmplifier() {
}



void LaborLogAmplifier::DisplayDBBar(double ValueDB){
	static double oldValue = 0;
	int16_t currentcolor;

	if (ValueDB == oldValue)
		return;

	for(int z= (int) (ValueDB+33); z<47; z++){
		tft.fillRect(10+10*z, 120, 7, 30, BLACK);
	}
    tft.fillRect(10, 120, 7, 30, BLUE1);   //ersten Balken blau anzeigen
	for(int z=1; z<(ValueDB+35); z++){
		currentcolor = BLUE1;
		if(z > 28 && z < 35)currentcolor = YELLOW;
		else if(z > 34 && z < 41)currentcolor = ORANGE;
		else if(z > 40 && z < 47)currentcolor = RED;
		if(z < 47){
			tft.fillRect(10+10*z, 120, 7, 30, currentcolor);
		}
	}
	oldValue = ValueDB;
}

LaborLogAmplifier::loopResult_t LaborLogAmplifier::loop(int Rotary1Counter,
		int Rotary1Switch, int Rotary2Counter, int Rotary2Switch,
		TSPoint touchPoint) {

	static int sampleCount = 0;
	static double meanSampleDBValue = 0;

	if(ADCReady_BNC1) {
		ADCReady_BNC1 = false;
		meanSampleDBValue = ((ADCValue_BNC1-744)*20/185.69+2.21);
		sampleCount += 1;

		DisplayDBBar(meanSampleDBValue);
	}

	return Rotary1Switch ? LR_EXIT:LR_STAY;
}

