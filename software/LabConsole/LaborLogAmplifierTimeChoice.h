/*
 * LaborLogAmplifierTimeChoice.h
 *
 *  Created on: 27.09.2019
 *      Author: torsten
 */

#ifndef LABORLOGAMPLIFIERTIMECHOICE_H_
#define LABORLOGAMPLIFIERTIMECHOICE_H_

#include <Arduino.h>

#include "Labor.h"

class LaborLogAmplifier_TimeChoice: public Labor {
public:
	LaborLogAmplifier_TimeChoice();
	virtual ~LaborLogAmplifier_TimeChoice();

	virtual loopResult_t loop(int Rotary1Counter, int Rotary1Switch, int Rotary2Counter, int Rotary2Switch, TSPoint touchPoint);
	int16_t get_timems(){return timems;}

private:
	int16_t timems;
};

#endif /* LABORLOGAMPLIFIERTIMECHOICE_H_ */
