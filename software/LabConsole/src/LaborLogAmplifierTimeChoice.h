/*
 * LaborLogAmplifierTimeChoice.h
 *
 *  Created on: 27.09.2019
 *      Author: torsten
 */

#ifndef LABORLOGAMPLIFIERTIMECHOICE_H_
#define LABORLOGAMPLIFIERTIMECHOICE_H_

#include <Arduino.h>

#include "ApplicationBase.h"

class LaborLogAmplifier_TimeChoice: public ApplicationBase {
public:
	LaborLogAmplifier_TimeChoice();
	virtual ~LaborLogAmplifier_TimeChoice();

	virtual loopResult_t loop(LaborLogAmplifier_TimeChoice::userinput_t userinput);
	void init();
	int16_t get_timems(){return timems;}

private:
	int16_t timems;
};

#endif /* LABORLOGAMPLIFIERTIMECHOICE_H_ */
