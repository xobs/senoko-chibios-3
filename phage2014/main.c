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
#include "ext.h"
#include "shell.h"
#include "chprintf.h"

#include "ledDriver.h"
#include "effects.h"
#include "phage.h"
#include "phage-shell.h"
#include "phage-events.h"
#include "phage-wdt.h"

#define LED_COUNT 22

static void shell_termination_handler(eventid_t id) {
  static int i = 1;
  (void)id;

  chprintf(stream, "\r\nRespawning shell (shell #%d)\r\n", ++i);
  phageShellRestart();
}

static void power_button_pressed_handler(eventid_t id) {
    (void)id;
      chprintf(stream, " [Button pressed] ");
}

static void power_button_released_handler(eventid_t id) {
    (void)id;
      chprintf(stream, " [Button released] ");
}

static evhandler_t event_handlers[] = {
  shell_termination_handler,
  power_button_pressed_handler,
  power_button_released_handler,
};

static event_listener_t event_listeners[ARRAY_SIZE(event_handlers)];

/*
 * Application entry point.
 */
int main(void) {
  uint8_t *framebuffer;

  /*
   * System initializations.
   * - HAL initialization, this also initializes the configured device drivers
   *   and performs the board-specific initializations.
   * - Kernel initialization, the main() function becomes a thread and the
   *   RTOS is active.
   */
  halInit();
  chSysInit();

  /* Start up serial console.*/
  phageShellInit();
  chEvtRegister(&shell_terminated, &event_listeners[0], 0);

  /* Listen to GPIO events (e.g. button presses, status changes).*/
  phageEventsInit();
  chEvtRegister(&power_button_pressed, &event_listeners[1], 1);
  chEvtRegister(&power_button_released, &event_listeners[2], 2);

  chprintf(stream, "\r\nStarting Phage (Ver %d.%d, git version %s)\r\n", 
      PHAGE_OS_VERSION_MAJOR,
      PHAGE_OS_VERSION_MINOR,
      gitversion);

  framebuffer = ledDriverInit(LED_COUNT, GPIOB, 0b11);
  chprintf(stream, "\tFramebuffer address: 0x%08x\r\n", framebuffer);
  ledDriverStart(framebuffer);

  /* Start the Phage watchdog timer thread.*/
  phageWatchdogInit();

  /* Start LED effects.*/
  effectsStart(framebuffer, LED_COUNT);

  /* Enter main event loop.*/
  phageShellRestart();
  while (TRUE)
    chEvtDispatch(event_handlers, chEvtWaitOne(ALL_EVENTS));

  return 0;
}
