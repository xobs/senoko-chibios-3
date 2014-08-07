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
<<<<<<< HEAD
#include "iwdg.h"

#include "senoko.h"
#include "senoko-events.h"
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

static void power_button_pressed_handler(eventid_t id) {
  (void)id;
  chprintf(stream, " [Button pressed] ");
}

static void power_button_released_handler(eventid_t id) {
  (void)id;
  chprintf(stream, " [Button released] ");
}

static void ac_unplugged_handler(eventid_t id) {
  (void)id;
  chprintf(stream, " [AC unplugged] ");
}

static void ac_plugged_handler(eventid_t id) {
  (void)id;
  chprintf(stream, " [AC plugged] ");
}

static evhandler_t event_handlers[] = {
  shell_termination_handler,
  power_button_pressed_handler,
  power_button_released_handler,
  ac_unplugged_handler,
  ac_plugged_handler,
};

static event_listener_t event_listeners[ARRAY_SIZE(event_handlers)];

=======

#include "senoko.h"
#include "senoko-i2c.h"
#include "senoko-shell.h"

static const SerialConfig serialConfig = {
  115200,
  0,
  0,
  0,
};

static THD_WORKING_AREA(waThread1, 128);
static msg_t Thread1(void *arg) {

  int i = 0;
  BaseSequentialStream *chp = arg;
  chRegSetThreadName("text");
  while (TRUE) {
    chprintf(chp, "Thread 1 (loop %d)\r\n", i++);
    chThdSleepMilliseconds(25000);
  }
  return 0;
}

>>>>>>> 4177a65a07b748bb28ca7f5533e1ca3dadba5e2c
/*
 * Application entry point.
 */
int main(void) {
<<<<<<< HEAD
=======
  int i = 0;
>>>>>>> 4177a65a07b748bb28ca7f5533e1ca3dadba5e2c

  /*
   * System initializations.
   * - HAL initialization, this also initializes the configured device drivers
   *   and performs the board-specific initializations.
   * - Kernel initialization, the main() function becomes a thread and the
   *   RTOS is active.
   */
  halInit();
  chSysInit();

<<<<<<< HEAD
  /* Set up I2C early, to prevent conflicting with the RAM DDC.*/
  senokoI2cInit();

  /* Start serial, so we can get status output.*/
  senokoShellInit();
  chEvtRegister(&shell_terminated, &event_listeners[0], 0);

  /* Listen to GPIO events (e.g. button presses, status changes).*/
  senokoEventsInit();
  chEvtRegister(&power_button_pressed, &event_listeners[1], 1);
  chEvtRegister(&power_button_released, &event_listeners[2], 2);
  chEvtRegister(&ac_unplugged, &event_listeners[3], 3);
  chEvtRegister(&ac_plugged, &event_listeners[4], 4);

  chprintf(stream, "\r\nStarting Senoko (Ver %d.%d, git version %s)\r\n", 
=======
  /* Set up I2C early, to prevent conflicting with the RAM DDC. */
  senokoI2cInit();

  /* Pull GG_SYSPRES low to bring it out of reset, and enable RAM DDC. */
  palWritePad(GPIOA, PA11, 0);

  /* Start serial, so we can get status output */
  sdStart(serialDriver, &serialConfig);

  shellInit();

  chprintf(stream, "\r\nResetting Senoko (Ver %d.%d, git version %s)\r\n", 
>>>>>>> 4177a65a07b748bb28ca7f5533e1ca3dadba5e2c
      SENOKO_OS_VERSION_MAJOR,
      SENOKO_OS_VERSION_MINOR,
      gitversion);

<<<<<<< HEAD
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
=======
  chprintf(stream, "Launching Thread1...\r\n");
  chThdCreateStatic(waThread1, sizeof(waThread1),
                    NORMALPRIO + 10, Thread1, stream);

  while (TRUE) {
    if (shellTerminated()) {
      chprintf(stream, "Spawning new shell (shell #%d)\r\n", i++);
      shellRestart();
    }
    chThdSleepMilliseconds(500);
  }
>>>>>>> 4177a65a07b748bb28ca7f5533e1ca3dadba5e2c

  return 0;
}
