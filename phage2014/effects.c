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

uint8_t brightness = 255;

uint32_t bump_amount = 0;

unsigned int rstate = 0xfade1337;
unsigned int rand(void);
unsigned int shift_lfsr(unsigned int v);

unsigned int shift_lfsr(unsigned int v)
{
  /*
    config          : galois
    length          : 16
    taps            : (16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 3, 2)
    shift-amount    : 1
    shift-direction : right
  */
  enum {
    length = 16,
    tap_00 = 16,
    tap_01 = 15,
    tap_02 = 14,
    tap_03 = 13,
    tap_04 = 12,
    tap_05 = 11,
    tap_06 = 10,
    tap_07 =  9,
    tap_08 =  8,
    tap_09 =  7,
    tap_10 =  6,
    tap_11 =  5,
    tap_12 =  3,
    tap_13 =  2
  };
  typedef unsigned int T;
  const T zero = (T)(0);
  const T lsb = zero + (T)(1);
  const T feedback = (
		      (lsb << (tap_00 - 1)) ^
		      (lsb << (tap_01 - 1)) ^
		      (lsb << (tap_02 - 1)) ^
		      (lsb << (tap_03 - 1)) ^
		      (lsb << (tap_04 - 1)) ^
		      (lsb << (tap_05 - 1)) ^
		      (lsb << (tap_06 - 1)) ^
		      (lsb << (tap_07 - 1)) ^
		      (lsb << (tap_08 - 1)) ^
		      (lsb << (tap_09 - 1)) ^
		      (lsb << (tap_10 - 1)) ^
		      (lsb << (tap_11 - 1)) ^
		      (lsb << (tap_12 - 1)) ^
		      (lsb << (tap_13 - 1))
		      );
  v = (v >> 1) ^ ((zero - (v & lsb)) & feedback);
  return v;
}

unsigned int rand(void) {
  rstate = shift_lfsr(rstate);
  return rstate;
}

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

static void strobePatternFB(void *fb, int count, int loop) {
  uint16_t i;
  
  brightness = 255;

  for( i = 0; i < count; i++ ) {
    if( (rand() % (unsigned int) count) < ((unsigned int) count / 3) )
      ledSetRGB(fb, i, 255, 255, 255);
    else
      ledSetRGB(fb, i, 0, 0, 0);
  }

  chThdSleepMilliseconds(30 + (rand() % 25));

  for( i = 0; i < count; i++ ) {
    ledSetRGB(fb, i, 0, 0, 0);
  }

  chThdSleepMilliseconds(30 + (rand() % 25));
}

static void calmPatternFB(void *fb, int count, int loop) {
  int i;
  int count_mask;
  Color c;

  count_mask = count & 0xff;
  loop = loop % (256 * 5);
  for (i = 0; i < count; i++) {
    c = Wheel( (i * (256 / count_mask) + loop) & 0xFF );
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

static void shootPatternFB(void *fb, int count, int loop) {
  int i;

  //loop = (loop >> 3) % count;
  loop = loop % count;
  for (i = 0; i < count; i++) {
    if (loop == i)
      ledSetRGB(fb, i, 255, 255, 255);
    else
      ledSetRGB(fb, i, 0, 0, 0);
  }
}

static uint32_t asb_l(int i) {
  if (i > 0)
      return i;
  return -i;
}

static void larsonScannerFB(void *fb, int count, int loop) {
  int i;
  int dir;

  loop %= (count * 2);

  if (loop >= count)
    dir = 1;
  else
    dir = 0;

  loop %= count;

  for (i = 0; i < count; i++) {
    uint32_t x = i;

    if (dir)
      x = count - i - 1;

    /* LED going out */
    if (asb_l(i - loop) == 2)
      ledSetRGBClipped(fb, x, 1, 0, 0);
    else if (asb_l(i - loop) == 1)
      ledSetRGBClipped(fb, x, 20, 0, 0);
    else if (asb_l(i - loop) == 0)
      ledSetRGBClipped(fb, x, 255, 0, 0);
    else
      ledSetRGBClipped(fb, x, 0, 0, 0);
  }

}

void bump(uint32_t amount) {
  bump_amount = amount;
}

static int draw_pattern(struct effects_config *config) {
    config->loop++;

    if( bump_amount != 0 ) {
      config->loop += bump_amount;
      bump_amount = 0;
    }

    if (config->pattern == patternShoot)
      shootPatternFB(config->fb, config->count, config->loop);
    else if (config->pattern == patternCalm) {
      calmPatternFB(config->fb, config->count, config->loop);
      config->loop += 2; // make this one go faster
    } else if (config->pattern == patternTest)
      testPatternFB(config->fb, config->count, config->loop);
    else if (config->pattern == patternStrobe)
      strobePatternFB(config->fb, config->count, config->loop);
    else {
      larsonScannerFB(config->fb, config->count, config->loop);
    }

    return 0;
}

static struct effects_config g_config;
void effectsSetPattern(enum pattern pattern) {
  g_config.pattern = pattern;
}

enum pattern effectsGetPattern(void) {
  return g_config.pattern;
}


static THD_WORKING_AREA(waEffectsThread, 256);
static msg_t effects_thread(void *arg) {

  chRegSetThreadName("effects");

  while (1) {
    draw_pattern(arg);
    ledUpdate();
    //chThdSleepMilliseconds(EFFECTS_REDRAW_MS);
  }
  return MSG_OK;
}

void effectsStart(void *_fb, int _count) {

  g_config.fb = _fb;
  g_config.count = _count;
  g_config.loop = 0;
  g_config.pattern = patternCalm;
  g_config.pattern = patternTest;

  draw_pattern(&g_config);
  chThdCreateStatic(waEffectsThread, sizeof(waEffectsThread),
      NORMALPRIO - 6, effects_thread, &g_config);
}
