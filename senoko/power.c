#include "ch.h"
#include "hal.h"

#include "chg.h"

enum power_state {
  power_off = 0,
  power_on = 1,
};

#define THREAD_SLEEP_MS 1000
//#define TESTING_POWER
static uint32_t *power_state = ((uint32_t *)(0x40006c00 + 0x18));

#include "senoko.h"
#include "chprintf.h"
static void power_set_state(enum power_state state) {
#ifdef TESTING_POWER
#warning "Testing power: Won't turn off"
  palWritePad(GPIOB, PB15, power_on);
#else
  palWritePad(GPIOB, PB15, state);
#endif
  *power_state = ((*power_state) & ~1) | (!state);
  return;
}

void powerOff(void) {
  power_set_state(power_off);
  return;
}

int powerIsOn(void) {
  return (!((*power_state) & 1)) == power_on;
}

int powerIsOff(void) {
  return (!((*power_state) & 1)) == power_off;
}

void powerOn(void) {
  power_set_state(power_on);
  return;
}

void powerToggle(void) {
  if (powerIsOn())
    powerOff();
  else
    powerOn();
  return;
}

void powerInit(void) {
  if (powerIsOn())
    powerOn();
  else
    powerOff();
}
