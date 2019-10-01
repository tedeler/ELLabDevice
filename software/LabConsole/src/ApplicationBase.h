/*
 * Labor.h
 *
 *  Created on: 27.09.2019
 *      Author: torsten
 */

#ifndef APPLICATIONBASE_H_
#define APPLICATIONBASE_H_

#include "MyTouchScreen.h"

class ApplicationBase {
public:
	enum loopResult_t {LR_EXIT, LR_STAY, LR_SWITCH};
	struct userinput_t {int Rotary1Counter;
	                   int Rotary1Switch;
	                   int Rotary2Counter;
	                   int Rotary2Switch;
	                   TSPoint touchPoint;};

	ApplicationBase();
	virtual ~ApplicationBase();

	virtual loopResult_t loop(ApplicationBase::userinput_t userinput) {return LR_STAY;};
	virtual void init() {};
	virtual void done() {};

private:

};

#endif /* APPLICATIONBASE_H_ */
