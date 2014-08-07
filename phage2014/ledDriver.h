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

void ledDriverInit(int leds, GPIO_TypeDef *port, uint32_t mask, void *fb);
void ledDriverStart(void *fb);
void ledDriverPause(void);
void ledDriverResume(void);
void ledUpdate(void);
void ledSetRGB(void *ptr, int x, uint8_t r, uint8_t g, uint8_t b);
void ledSetRGBClipped(void *fb, uint32_t i,
                      uint8_t r, uint8_t g, uint8_t b);

#endif /* LEDDRIVER_H_ */
