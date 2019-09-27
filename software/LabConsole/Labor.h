/*
 * Labor.h
 *
 *  Created on: 27.09.2019
 *      Author: torsten
 */

#ifndef LABOR_H_
#define LABOR_H_

#include "MyTouchScreen.h"

enum laborchoice_t {LC_None, LC_Labor1, LC_Labor2, LC_Labor3};

class Labor {
public:
	enum loopResult_t {LR_EXIT, LR_STAY, LR_SWITCH};

	Labor();
	virtual ~Labor();
	virtual loopResult_t loop(int Rotary1Counter, int Rotary1Switch, int Rotary2Counter, int Rotary2Switch, TSPoint touchPoint) = 0;

private:

};

#endif /* LABOR_H_ */
