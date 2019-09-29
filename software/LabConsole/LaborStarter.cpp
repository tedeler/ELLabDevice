/*
 * LaborStarter.cpp
 *
 *  Created on: 27.09.2019
 *      Author: torsten
 */

#include "LaborStarter.h"

#include "global.h"
#include <Arduino.h>
#include <MCUFRIEND_kbv.h>

extern MCUFRIEND_kbv tft;


LaborStarter::LaborStarter()
	:choice(LC_None)
{
}

LaborStarter::~LaborStarter() {
}

LaborStarter::loopResult_t LaborStarter::loop(LaborStarter::userinput_t userinput) {
	TSPoint p = userinput.touchPoint;
	int16_t x, y;


	tft.fillScreen(WHITE);
	if(p.z)
		tft.fillScreen(BLUE);
	else
		tft.fillScreen(GREEN);
	return LR_STAY;



	if (!p.z)
		return LR_STAY;

	x = p.x;
	y = p.y;

	if (x > 114 && x < 364) {
		if (y > 120 && y < 160)
			choice = LC_Labor1;
		if (y > 190 && y < 230)
			choice = LC_Labor2;
		if (y > 260 && y < 300)
			choice = LC_Labor3;
	}

	return choice==LC_None ? LR_STAY:LR_SWITCH;
}

void LaborStarter::init() {
	choice = LC_None;


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

void LaborStarter::done() {
}
