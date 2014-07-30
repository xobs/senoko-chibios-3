/*
    ChibiOS/RT - Copyright (C) 2006-2013 Giovanni Di Sirio

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#include "ch.h"
#include "uart.h"
#include "hal.h"
#include "shell.h"
#include "chprintf.h"
#include "i2c.h"
#include "iwdg.h"

#include "senoko.h"
#include "senoko-i2c.h"
#include "senoko-shell.h"
#include "senoko-wdt.h"

#include "chg.h"
#include "power.h"
#include "gg.h"

static void shell_termination_handler(eventid_t id) {
  static int i = 1;
  (void)id;

  chprintf(stream, "\r\nRespawning shell (shell #%d)\r\n", ++i);
  senokoShellRestart();
}

static evhandler_t event_handlers[] = {
  shell_termination_handler,
};

/*
 * Application entry point.
 */
int main(void) {
  event_listener_t event_listener;

  /*
   * System initializations.
   * - HAL initialization, this also initializes the configured device drivers
   *   and performs the board-specific initializations.
   * - Kernel initialization, the main() function becomes a thread and the
   *   RTOS is active.
   */
  halInit();
  chSysInit();

  /* Set up I2C early, to prevent conflicting with the RAM DDC. */
  senokoI2cInit();

  /* Start serial, so we can get status output */
  senokoShellInit();

  chEvtRegister(&shell_terminated, &event_listener, 0);

  chprintf(stream, "\r\nStarting Senoko (Ver %d.%d, git version %s)\r\n", 
      SENOKO_OS_VERSION_MAJOR,
      SENOKO_OS_VERSION_MINOR,
      gitversion);

  /* Turn on the charger (and start charging, if necessary).*/
  chgInit();

  /* Turn on mainboard and synchronize power state.*/
  powerInit();

  /* Power up gas gauge.*/
  ggInit();

  /* Start the Senoko watchdog timer thread.*/
  senokoWatchdogInit();

  senokoShellRestart();
  while (TRUE)
    chEvtDispatch(event_handlers, chEvtWaitOne(ALL_EVENTS));

  return 0;
}
