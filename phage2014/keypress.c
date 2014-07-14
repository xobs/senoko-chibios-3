#include "ch.h"
#include "hal.h"
#include "ext.h"
#include "keypress.h"
#include "phage2014.h"

static virtual_timer_t vt;
static uint8_t keyStates[4];
static EVENTSOURCE_DECL(keyStateSource);

static void keyDown(void *arg)
{
  int channel = (int)arg;
  uint8_t key = -1;
  uint8_t state;

  if (channel == 4) {
    state = palReadPad(GPIOB, PB4);
    key = KEY_0;
  }
  else if (channel == 5) {
    state = palReadPad(GPIOB, PB5);
    key = KEY_3;
  }
  else if (channel == 13) {
    state = palReadPad(GPIOA, PA13);
    key = KEY_2;
  }
  else if (channel == 15) {
    state = palReadPad(GPIOA, PA15);
    key = KEY_1;
  }
  else
    return;

  if (state != keyStates[key])
    return;

  keyStates[key] = state;

  if (state)
    chEvtBroadcastFlags(&keyStateSource, key);
  else
    chEvtBroadcastFlags(&keyStateSource, key << 4);
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
  chEvtObjectInit(&keyStateSource);
  return;
}

void keyRegisterListener(event_listener_t *el)
{
  chEvtRegisterMask(&keyStateSource, el, 0xff);
}
