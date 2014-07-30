#include "ch.h"
#include "hal.h"
#include "iwdg.h"
#include "senoko-wdt.h"

static const IWDGConfig watchdogConfig = {
  MS2ST(SENOKO_WATCHDOG_MS), /* counter */
  IWDG_DIV_64, /* div */
};

static THD_WORKING_AREA(waWdtThread, 32);
static msg_t wdt_thread(void *arg) {
  (void)arg;

  chRegSetThreadName("senoko watchdog");

  while (1) {
    iwdgReset(&IWDGD);
    chThdSleepMilliseconds(SENOKO_WATCHDOG_THREAD_MS);
  }
  return MSG_OK;
}

void senokoWatchdogInit(void) {
  iwdgInit();
  iwdgStart(&IWDGD, &watchdogConfig);

  chThdCreateStatic(waWdtThread, sizeof(waWdtThread),
                    HIGHPRIO - 5, wdt_thread, NULL);
}
