/*
 * LaborStarter.h
 *
 *  Created on: 27.09.2019
 *      Author: torsten
 */

#ifndef LABORSTARTER_H_
#define LABORSTARTER_H_

#include "ApplicationBase.h"

class LaborStarter: public ApplicationBase {
public:
	enum laborchoice_t {LC_None, LC_Labor1, LC_Labor2, LC_Labor3};

	LaborStarter();
	virtual ~LaborStarter();

	virtual loopResult_t loop(LaborStarter::userinput_t userinput);
	virtual void init();
	virtual void done();

	laborchoice_t getChoice() const {
		return choice;
	}

private:
	laborchoice_t choice;
};

#endif /* LABORSTARTER_H_ */
