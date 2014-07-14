/*
 * LEDDriver.h
 *
 *  Created on: Aug 26, 2013
 *      Author: Omri Iluz
 */

#ifndef LEDDRIVER_H_
#define LEDDRIVER_H_

#include "hal.h"

#define sign(x) (( x > 0 ) - ( x < 0 ))

typedef struct Color Color;
struct Color {
  uint8_t r;
  uint8_t g;
  uint8_t b;
};

void ledDriverInit(int leds, GPIO_TypeDef *port, uint32_t mask, uint8_t **o_fb);
void ledDriverStart(uint8_t *fb);
void ledDriverStop(void);

enum pattern {
  patternCalm,
  patternTest,
  patternShoot,
};

void runPatternFB(uint8_t *fb, enum pattern, int count, uint32_t param1);

#endif /* LEDDRIVER_H_ */
