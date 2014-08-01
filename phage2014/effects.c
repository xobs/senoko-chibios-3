/*
 * LEDDriver.c
 *
 *  Created on: Aug 26, 2013
 *      Author: Omri Iluz
 */

#include "ch.h"
#include "hal.h"
#include "pwm.h"
#include "ledDriver.h"
#include "effects.h"

#include "chprintf.h"
#include "phage.h"

struct effects_config {
  void *fb;
  uint32_t count;
  uint32_t loop;
  enum pattern pattern;
};

static Color Wheel(uint8_t wheelPos) {
  Color c;

  if (wheelPos < 85) {
    c.r = wheelPos * 3;
    c.g = 255 - wheelPos * 3;
    c.b = 0;
  }
  else if (wheelPos < 170) {
    wheelPos -= 85;
    c.r = 255 - wheelPos * 3;
    c.g = 0;
    c.b = wheelPos * 3;
  }
  else {
    wheelPos -= 170;
    c.r = 0;
    c.g = wheelPos * 3;
    c.b = 255 - wheelPos * 3;
  }
  return c;
}

static void calmPatternFB(void *fb, int count, int loop) {
  int i;

  count &= 0xff;
  
  for (i = 0; i < count; i++) {
    Color c;
    c = Wheel( (i * (256 / count) + loop) & 0xFF );
    ledSetRGB(fb, i, c.r, c.g, c.b);
  }
}

static void testPatternFB(void *fb, int count, int loop) {
  int i = 0;

#if 0
  while (i < count) {
    /* Black */
    ledSetRGB(fb, (i + loop) % count, 0, 0, 0);
    if (++i >= count) break;

    /* Red */
    ledSetRGB(fb, (i + loop) % count, 255, 0, 0);
    if (++i >= count) break;

    /* Yellow */
    ledSetRGB(fb, (i + loop) % count, 255, 255, 0);
    if (++i >= count) break;

    /* Green */
    ledSetRGB(fb, (i + loop) % count, 0, 255, 0);
    if (++i >= count) break;

    /* Cyan */
    ledSetRGB(fb, (i + loop) % count, 0, 255, 255);
    if (++i >= count) break;

    /* Blue */
    ledSetRGB(fb, (i + loop) % count, 0, 0, 255);
    if (++i >= count) break;

    /* Purple */
    ledSetRGB(fb, (i + loop) % count, 255, 0, 255);
    if (++i >= count) break;

    /* White */
    ledSetRGB(fb, (i + loop) % count, 255, 255, 255);
    if (++i >= count) break;
  }
#endif
  while (i < count) {
    if (loop & 1) {
      /* Black */
      ledSetRGB(fb, (i++ + loop) % count, 0, 0, 0);

      /* Black */
      ledSetRGB(fb, (i++ + loop) % count, 0, 0, 0);

      /* White */
      ledSetRGB(fb, (i++ + loop) % count, 32, 32, 32);
    }
    else {
      /* White */
      ledSetRGB(fb, (i++ + loop) % count, 32, 32, 32);

      /* Black */
      ledSetRGB(fb, (i++ + loop) % count, 0, 0, 0);

      /* Black */
      ledSetRGB(fb, (i++ + loop) % count, 0, 0, 0);
    }
  }
}

static void shootPatternFB(void *fb, int count) {
  int i;

  count %= count;
  for (i = 0; i < count; i++) {
    if (count == i)
      ledSetRGB(fb, i, 255, 255, 255);
    else
      ledSetRGB(fb, i, 0, 0, 0);
  }
}

static int cycle_pattern(struct effects_config *config) {
    config->loop++;

    if (config->pattern == patternShoot)
      shootPatternFB(config->fb, config->count);
    else if (config->pattern == patternCalm)
      calmPatternFB(config->fb, config->count, config->loop);
    else if (config->pattern == patternTest)
      testPatternFB(config->fb, config->count, config->loop);
    else
      calmPatternFB(config->fb, config->count, config->loop);

    return 0;
}

static THD_WORKING_AREA(waEffectsThread, 32);
static msg_t effects_thread(void *arg) {

  chRegSetThreadName("effects");

  while (1) {
    cycle_pattern(arg);
    chThdSleepMilliseconds(EFFECTS_REDRAW_MS);
  }
  return MSG_OK;
}


void effectsStart(void *_fb, int _count) {
  static struct effects_config config;

  config.fb = _fb;
  config.count = _count;
  config.loop = 0;
  config.pattern = patternCalm;
  config.pattern = patternTest;

  cycle_pattern(&config);
  chThdCreateStatic(waEffectsThread, sizeof(waEffectsThread),
      NORMALPRIO, effects_thread, &config);
}
