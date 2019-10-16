/*
 * LaborLogAmplifier.cpp
 *
 *  Created on: 27.09.2019
 *      Author: torsten
 */

#include "LaborLogAmplifier.h"

#include "global.h"
#include <Arduino.h>
#include <MCUFRIEND_kbv.h>
#include <math.h>

extern MCUFRIEND_kbv tft;

extern uint16_t ADCValue_BNC1;
extern bool ADCReady_BNC1;


LaborLogAmplifier::LaborLogAmplifier() :
		timems(0) {
}

LaborLogAmplifier::~LaborLogAmplifier() {
}

void LaborLogAmplifier::init() {
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


	tft.setCursor(350, 250);
	tft.setTextColor(YELLOW);    tft.setTextSize(3);
	tft.print(timems);
	tft.print("ms");
}

uint16_t LaborLogAmplifier::DisplayColorbyDB(double ValueDB) {
	uint16_t color;

	if(ValueDB < -4)
		color = BLUE1;
	else if (ValueDB < 2)
		color = YELLOW;
	else if (ValueDB < 8)
		color = ORANGE;
	else
		color = RED;

	return color;
}

void LaborLogAmplifier::DisplayDBBar(double ValueDB){
	int16_t currentcolor;

	for(int i = 0; i<47; i++) {
		uint16_t color=BLACK;
		double CurrentBar_dbValue = i-33;

		if (i==0)
			color = BLUE1;
		else if (CurrentBar_dbValue <= ValueDB){
			color = this->DisplayColorbyDB(CurrentBar_dbValue);
		}
		tft.fillRect(10+10*i, 120, 7, 30, color);
	}
}

LaborLogAmplifier::loopResult_t LaborLogAmplifier::loop(LaborLogAmplifier::userinput_t userinput) {

	static int sampleCount = 0;
	double dbValue = 0;
	static double Energy = 0;

	if(ADCReady_BNC1) {
		ADCReady_BNC1 = false;
		dbValue = ((ADCValue_BNC1-744)*20/185.69+2.21);
		Energy += pow(10, (dbValue/10));
		sampleCount += 1;


		if(sampleCount*20/44.1 > this->timems)
		{
			double MeanPower = Energy/sampleCount;
			double MeanDb = log10(MeanPower)*10;
			DisplayDBBar(MeanDb);
			sampleCount = 0;
			Energy = 0;
			SerialUSB.print(dbValue);
			SerialUSB.print(", ");
			SerialUSB.println(MeanDb);
		}
	}

	return userinput.Rotary1Switch ? LR_EXIT:LR_STAY;
}

