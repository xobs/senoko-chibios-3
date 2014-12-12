#include "ch.h"
#include "hal.h"

#include "chg.h"
#include "senoko-wdt.h"
#include "senoko-events.h"

#define REBOOT_QUIESCE_MS 300

enum power_state {
  power_off = 0,
  power_on = 1,
};

//#define TESTING_POWER

/* Save power state across boots.  Shared with senoko-slave module. */
static uint32_t *power_state = ((uint32_t *)(0x40006c00 + 0x18));

static void power_set_state_x(enum power_state state) {
#ifdef TESTING_POWER
#warning "Testing power: Won't turn off"
  palWritePad(GPIOB, PB15, power_on);
#else
  palWritePad(GPIOB, PB15, state);
#endif

  /* Save the value in a persistent register, in case we crash */
  *power_state = ((*power_state) & ~1) | (!state);
  return;
}

static enum power_state power_state_x(void) {
  return (!((*power_state) & 1));
}

static void power_set_state_i(enum power_state state) {
  if (state != power_state_x()) {
    power_set_state_x(state);

    chSysLockFromISR();
    if (state == power_on)
      chEvtBroadcastI(&powered_on);
    else if (state == power_off)
      chEvtBroadcastI(&powered_off);
    chSysUnlockFromISR();
  }
}

static void power_set_state(enum power_state state) {
  if (state != power_state_x()) {
    power_set_state_x(state);

    if (state == power_on)
      chEvtBroadcast(&powered_on);
    else if (state == power_off)
      chEvtBroadcast(&powered_off);
  }
}

static virtual_timer_t power_on_vt;
static void call_power_on(void *arg) {
  (void)arg;
  power_set_state_i(power_on);
}

void powerOff(void) {
  power_set_state(power_off);
  senokoWatchdogDisable();
  return;
}

void powerOffI(void) {
  power_set_state_i(power_off);
  senokoWatchdogDisable();
  return;
}

void powerOn(void) {
  power_set_state(power_on);
  return;
}

void powerOnI(void) {
  power_set_state_i(power_on);
  return;
}

void powerRebootI(void) {
  powerOffI();
  chSysLockFromISR();
  chVTSetI(&power_on_vt, MS2ST(REBOOT_QUIESCE_MS), call_power_on, NULL);
  chSysUnlockFromISR();
}

void powerReboot(void) {
  powerOff();
  chVTSet(&power_on_vt, MS2ST(REBOOT_QUIESCE_MS), call_power_on, NULL);
}

int powerIsOn(void) {
  return power_state_x() == power_on;
}

int powerIsOff(void) {
  return power_state_x() == power_off;
}

void powerToggleI(void) {
  if (powerIsOn())
    powerOffI();
  else
    powerOnI();
  return;
}

void powerInit(void) {
  if (powerIsOn())
    powerOn();
  else
    powerOff();
  return;
}
