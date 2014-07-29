#include "ch.h"
#include "hal.h"

#include "chg.h"

#define THREAD_SLEEP_MS 1000

enum power_state {
  power_off = 0,
  power_on = 1,
};
static enum power_state power_state;

static void power_set_state(enum power_state state) {
  palWritePad(GPIOB, PB15, state);
  power_state = state;
  return;
}

void powerOff(void) {
  //power_set_state(power_off);
#warning "Power off is disabled!"
  power_set_state(power_on);
  return;
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
