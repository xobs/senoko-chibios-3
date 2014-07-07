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

/*
 * Application entry point.
 */
int main(void) {
  int i = 0;

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

  /* Pull GG_SYSPRES low to bring it out of reset, and enable RAM DDC. */
  palWritePad(GPIOA, PA11, 0);

  /* Start serial, so we can get status output */
  sdStart(serialDriver, &serialConfig);

  shellInit();

  chprintf(stream, "\r\nResetting Senoko (Ver %d.%d, git version %s)\r\n", 
      SENOKO_OS_VERSION_MAJOR,
      SENOKO_OS_VERSION_MINOR,
      gitversion);

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

  return 0;
}
