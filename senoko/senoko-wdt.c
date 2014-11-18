#include "ch.h"
#include "hal.h"
#include "iwdg.h"
#include "power.h"
#include "senoko-wdt.h"

static const IWDGConfig watchdogConfig = {
  MS2ST(SENOKO_WATCHDOG_MS), /* counter */
  IWDG_DIV_64, /* div */
};

static int enabled = 0;
static int seconds;

int senokoWatchdogEnabled(void) {
  return enabled;
}

void senokoWatchdogSet(int new_seconds) {
  seconds = new_seconds * 2; /* Our timer runs twice per second */
}

int senokoWatchdogTimeToReset(void) {
  return seconds / 2;
}

void senokoWatchdogEnable(void) {
  enabled = 1;
}

void senokoWatchdogDisable(void) {
  enabled = 0;
}

static THD_WORKING_AREA(waWdtThread, 32);
static msg_t wdt_thread(void *arg) {
  (void)arg;

  chRegSetThreadName("senoko watchdog");

  while (1) {
    iwdgReset(&IWDGD);
    chThdSleepMilliseconds(SENOKO_WATCHDOG_THREAD_MS);

    if (enabled && seconds)
      seconds--;
    if (enabled && !seconds) {
      enabled = 0;
      powerReboot();
    }
  }
  return MSG_OK;
}

void senokoWatchdogInit(void) {
  iwdgInit();
  iwdgStart(&IWDGD, &watchdogConfig);

  chThdCreateStatic(waWdtThread, sizeof(waWdtThread),
                    LOWPRIO, wdt_thread, NULL);
}
