#include "ch.h"
#include "hal.h"
#include "ext.h"
#include "keypress.h"
#include "phage2014.h"

static virtual_timer_t vt;
static uint8_t keyStates[4];

#include "ledDriver.h"
extern enum pattern currentPattern;

extern void keyPressHook(enum keychar, int state);

static void keyDown(void *arg)
{
  int channel = (int)arg;
  enum keychar key = KEY_NONE;
  int state;

  if (channel == 4) {
    state = !palReadPad(GPIOB, PB4);
    key = KEY_0;
  }
  else if (channel == 5) {
    state = palReadPad(GPIOB, PB5);
    key = KEY_3;
  }
  else if (channel == 13) {
    state = !palReadPad(GPIOA, PA13);
    key = KEY_2;
  }
  else if (channel == 15) {
    state = !palReadPad(GPIOA, PA15);
    key = KEY_1;
  }
  else
    return;

  if (state == keyStates[key])
    return;

  keyStates[key] = state;

  keyPressHook(key, state);
}

void keyISR(EXTDriver *extp, expchannel_t channel)
{
  (void)extp;

  chSysLockFromISR();
  chVTSetI(&vt, MS2ST(KEY_DEBOUNCE_MS), keyDown, (void *)channel);
  chSysUnlockFromISR();
}

void keyInit(void)
{
//  chEvtObjectInit(&keyStateSource);
  return;
}

enum keychar keyIsPressed(void)
{
  if (!palReadPad(GPIOB, PB4))
    return KEY_0;

  if (palReadPad(GPIOB, PB5))
    return KEY_3;

  if (!palReadPad(GPIOA, PA13))
    return KEY_2;

  if (!palReadPad(GPIOA, PA15))
    return KEY_1;

  return KEY_NONE;
}
