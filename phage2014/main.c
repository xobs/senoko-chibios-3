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
#include "LEDDriver.h"
#include "radio.h"

#include "phage2014.h"
#include "phage2014-shell.h"

static const SerialConfig serialConfig = {
  115200,
  0,
  0,
  0,
};

void key0Pressed(EXTDriver *extp, expchannel_t channel)
{
  chprintf(stream, "Pressed Key 0\r\n");
}

void key1Pressed(EXTDriver *extp, expchannel_t channel)
{
  chprintf(stream, "Pressed Key 1\r\n");
}

void key2Pressed(EXTDriver *extp, expchannel_t channel)
{
  chprintf(stream, "Pressed Key 2\r\n");
}

void key3Pressed(EXTDriver *extp, expchannel_t channel)
{
  chprintf(stream, "Pressed Key 3\r\n");
}

static const EXTConfig extConfig = {
  {
    {EXT_CH_MODE_FALLING_EDGE     /* PA0 */
      | EXT_CH_MODE_AUTOSTART
      | EXT_MODE_GPIOA, radioAddressMatch},
    {EXT_CH_MODE_DISABLED, NULL}, /* PA1 */
    {EXT_CH_MODE_DISABLED, NULL}, /* PA2 */
    {EXT_CH_MODE_DISABLED, NULL}, /* PA3 */
    {EXT_CH_MODE_FALLING_EDGE     /* PB4 */
      | EXT_CH_MODE_AUTOSTART
      | EXT_MODE_GPIOB, key0Pressed},
    {EXT_CH_MODE_FALLING_EDGE     /* PB5 */
      | EXT_CH_MODE_AUTOSTART
      | EXT_MODE_GPIOB, key3Pressed},
    {EXT_CH_MODE_DISABLED, NULL}, /* PA6 */
    {EXT_CH_MODE_DISABLED, NULL}, /* PA7 */
    {EXT_CH_MODE_DISABLED, NULL}, /* PA8 */
    {EXT_CH_MODE_DISABLED, NULL}, /* PA9 */
    {EXT_CH_MODE_DISABLED, NULL}, /* PA10 */
    {EXT_CH_MODE_FALLING_EDGE     /* PA11 */
      | EXT_CH_MODE_AUTOSTART
      | EXT_MODE_GPIOA, radioDataReceived},
    {EXT_CH_MODE_FALLING_EDGE     /* PA12 */
      | EXT_CH_MODE_AUTOSTART
      | EXT_MODE_GPIOA, radioCarrier},
    {EXT_CH_MODE_FALLING_EDGE     /* PA13 */
      | EXT_CH_MODE_AUTOSTART
      | EXT_MODE_GPIOA, key2Pressed},
    {EXT_CH_MODE_DISABLED, NULL}, /* PA14 */
    {EXT_CH_MODE_FALLING_EDGE     /* PA15 */
      | EXT_CH_MODE_AUTOSTART
      | EXT_MODE_GPIOA, key2Pressed},
  }
};

#if ANNOYING_BLINK
static THD_WORKING_AREA(waThread1, 128);
static msg_t Thread1(void *arg)
{

  int i = 0;

  (void)arg;

  chRegSetThreadName("text");
  while (TRUE) {
    i++;

    /* Blink the LED */
    palWritePad(GPIOA, PA2, (i & 1) ? PAL_LOW : PAL_HIGH);

    chThdSleepMilliseconds(250);
  }
  return 0;
}
#endif

/*
 * Application entry point.
 */
int main(void) {
  int i = 0;
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

  /* Start serial, so we can get status output */
  sdStart(serialDriver, &serialConfig);

  shellInit();

  chprintf(stream, "\r\nResetting Phage2014 (Ver %d.%d, git version %s)\r\n", 
      PHAGE2014_OS_VERSION_MAJOR,
      PHAGE2014_OS_VERSION_MINOR,
      gitversion);

  /* Begin listening to GPIOs (e.g. the button) */
  extStart(&EXTD1, &extConfig);

  radioStart();

//  ledDriverInit(1, GPIOB, 0b11, &framebuffer);
  chprintf(stream, "Framebuffer address: 0x%08x\n", framebuffer);

#if ANNOYING_BLINK
  chprintf(stream, "Launching Thread1...\r\n");
  chThdCreateStatic(waThread1, sizeof(waThread1),
                    NORMALPRIO + 10, Thread1, stream);
#endif

  while (TRUE) {
    testPatternFB(framebuffer);
    if (shellTerminated()) {
      chprintf(stream, "Spawning new shell (shell #%d)\r\n", i++);
      shellRestart();
    }
    chThdSleepMilliseconds(500);
  }

  return 0;
}
