#include "ch.h"
#include "hal.h"

#include "chg.h"

#define THREAD_SLEEP_MS 1000
#define TESTING_POWER

enum power_state {
  power_off = 0,
  power_on = 1,
};
static enum power_state power_state = power_off;

static void power_set_state(enum power_state state) {
#ifdef TESTING_POWER
#warning "Testing power: Won't turn off"
  palWritePad(GPIOB, PB15, power_on);
#else
  palWritePad(GPIOB, PB15, state);
#endif
  power_state = state;
  return;
}

void powerOff(void) {
  power_set_state(power_off);
  return;
}

int powerIsOn(void) {
  return power_state == power_on;
}

int powerIsOff(void) {
  return power_state == power_off;
}

void powerOn(void) {
  power_set_state(power_on);
  return;
}

void powerToggle(void) {
  if (power_state == power_on)
    powerOff();
  else
    powerOn();
  return;
}

static THD_WORKING_AREA(waPower, 128);
static msg_t power_thread(void *arg) {
  (void)arg;

  chRegSetThreadName("on-fire check");

  while (1) {
    chThdSleepMilliseconds(200);

    /* Monitor "We're On Fire" GPIO.*/
    if (palReadPad(GPIOA, PA0)) {
      powerOff();
      chgSet(0, 0, 0);
    }
    chThdSleepMilliseconds(THREAD_SLEEP_MS);
  }
  return 0;
}

void powerInit(void) {
  powerOn();
  chThdCreateStatic(waPower, sizeof(waPower), HIGHPRIO, power_thread, NULL);
}
