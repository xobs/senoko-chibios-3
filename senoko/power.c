#include "ch.h"
#include "hal.h"

#include "chg.h"
#include "senoko-wdt.h"
#include "senoko-events.h"

/* When a "reboot" is issued, stay powered off for this long */
#define REBOOT_QUIESCE_MS 300

/* When powered off, wait this long before allowing a powerup */
#define COOL_OFF_MS 500

static int cooling_off;

enum power_state {
  power_off = 0,
  power_on = 1,
};

#define POWER_STATE_SIGNATURE_MASK 0x00f0
#define POWER_STATE_SIGNATURE 0x0050
#define POWER_STATE_MASK 0x1

//#define TESTING_POWER

/* Save power state across boots.  Shared with senoko-slave module. */
static uint32_t *power_state = ((uint32_t *)(0x40006c00 + 0x18));

static void power_set_state_x(enum power_state state) {
  uint32_t new_power_state;
#ifdef TESTING_POWER
#warning "Testing power: Won't turn off"
  palWritePad(GPIOB, PB15, power_on);
#else
  palWritePad(GPIOB, PB15, state);
#endif

  /* Save the value in a persistent register, in case we crash */
  new_power_state = (*power_state);
  new_power_state &= ~POWER_STATE_MASK;
  new_power_state &= ~POWER_STATE_SIGNATURE_MASK;
  new_power_state |= state;
  new_power_state |= POWER_STATE_SIGNATURE;

  *power_state = new_power_state;
  return;
}

static enum power_state power_state_x(void) {
  return ((*power_state) & POWER_STATE_MASK);
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

/* To prevent flap, make sure the board is off for a certain amount of time.*/
static virtual_timer_t cool_off_vt;
static void stop_cool_off(void *arg) {
  (void)arg;
  cooling_off = 0;
}

void powerOff(void) {
  power_set_state(power_off);
  cooling_off = 1;
  chVTSet(&cool_off_vt, MS2ST(COOL_OFF_MS), stop_cool_off, NULL);
  senokoWatchdogDisable();
  return;
}

void powerOffI(void) {
  power_set_state_i(power_off);
  cooling_off = 1;
  chVTSetI(&cool_off_vt, MS2ST(COOL_OFF_MS), stop_cool_off, NULL);
  senokoWatchdogDisable();
  return;
}

void powerOn(void) {
  if (cooling_off)
    return;
  power_set_state(power_on);
  return;
}

void powerOnI(void) {
  if (cooling_off)
    return;
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

  cooling_off = 0;

  /* If the signature is not valid, assume fresh STM32, and power on.*/
  if (((*power_state) & POWER_STATE_SIGNATURE_MASK) != POWER_STATE_SIGNATURE)
    powerOn();

  if (powerIsOn())
    powerOn();
  else
    powerOff();
  return;
}
