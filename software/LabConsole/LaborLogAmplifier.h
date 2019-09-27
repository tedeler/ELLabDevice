/*
 * LaborLogAmplifier.h
 *
 *  Created on: 27.09.2019
 *      Author: torsten
 */

#ifndef LABORLOGAMPLIFIER_H_
#define LABORLOGAMPLIFIER_H_
#include <arduino.h>

#include "ApplicationBase.h"

class LaborLogAmplifier: public ApplicationBase {
public:
	LaborLogAmplifier(int16_t timems);
	virtual ~LaborLogAmplifier();

	virtual loopResult_t loop(int Rotary1Counter, int Rotary1Switch, int Rotary2Counter, int Rotary2Switch, TSPoint touchPoint);

private:
	int16_t timems;

	void DisplayDBBar(double ValueDB);
};

#endif /* LABORLOGAMPLIFIER_H_ */
