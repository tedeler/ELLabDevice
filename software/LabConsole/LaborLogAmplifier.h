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
	LaborLogAmplifier();
	virtual ~LaborLogAmplifier();

	virtual loopResult_t loop(LaborLogAmplifier::userinput_t userinput);
	virtual void init();

	void setTimems(int16_t timems) {
		this->timems = timems;
	}

private:
	int16_t timems;

	void DisplayDBBar(double ValueDB);
};

#endif /* LABORLOGAMPLIFIER_H_ */
