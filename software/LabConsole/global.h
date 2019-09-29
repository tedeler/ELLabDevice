/*
 * global.h
 *
 *  Created on: 27.09.2019
 *      Author: torsten
 */

#ifndef GLOBAL_H_
#define GLOBAL_H_


// Assign human-readable names to some common 16-bit color values:
#define	BLACK     0x0000
#define	BLUE      0x001F
#define BLUE1     0x041F
#define BLUEHAW1  0x9DDA
#define BLUEHAW2  0x1A32
#define	RED       0xF800
#define	GREEN     0x07E0
#define CYAN      0x07FF
#define MAGENTA   0xF81F
#define YELLOW    0xFFE0
#define ORANGE    0xFC00
#define WHITE     0xFFFF
#define BACKCOLOR 0xFFFF

#define BOXSIZE 40
#define SIDE_H 114
#define SIDE_V 30
#define START_H 30

// TOUCH PANEL PINS
#define YP A2  // must be an analog pin, use "An" notation!
#define XM A3  // must be an analog pin, use "An" notation!
#define YM 8   // can be a digital pin
#define XP 9   // can be a digital pin


#endif /* GLOBAL_H_ */
