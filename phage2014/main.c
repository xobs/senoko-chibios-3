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
#include "LEDDriver.h"
#include "radio.h"

#include "phage2014.h"
#include "phage2014-shell.h"

#define NUMPIX 240
uint32_t fb[NUMPIX];  // pixels stored in xGRB format
extern int ws2812_send(uint32_t *fb, uint32_t len);

static const SerialConfig serialConfig = {
  115200,
  0,
  0,
  0,
};

static THD_WORKING_AREA(waThread1, 128);
static msg_t Thread1(void *arg)
{

  int i = 0;

  (void)arg;

  chRegSetThreadName("text");
  while (TRUE) {
    i++;

    /* Blink the LED */
    //    palWritePad(GPIOA, PA2, (i & 1) ? PAL_LOW : PAL_HIGH);
    palWritePad(GPIOA, PA2, PAL_LOW);

    chThdSleepMilliseconds(250);
  }
  return 0;
}

static THD_WORKING_AREA(waThread3, 128);
static msg_t Thread3(void *arg)
{

  int i = 0;

  (void)arg;

  chRegSetThreadName("text3");
  while (TRUE) {
    i++;

    if( PAL_LOW == palReadPad(GPIOA, PA13) ) {
      radio_TX(0xC3);
    }

    chThdSleepMilliseconds(100);
  }
  return 0;
}

static THD_WORKING_AREA(waThread4, 128);
static msg_t Thread4(void *arg)
{

  uint8_t c;

  (void)arg;

  chRegSetThreadName("text4");
  while (TRUE) {
    if( PAL_HIGH == palReadPad(GPIOA, PA11) ) {  // DR is asserted
      c = radio_RX();
      if( c == 0xC3 ) {
	palWritePad(GPIOA, PA2, PAL_HIGH);
      }
    }

    chThdSleepMilliseconds(10);
  }
  return 0;
}

#define COLOR(r,g,b)  ((((g) & 0xFF) << 16) | (((r) & 0xFF) << 8) | ((b) & 0xFF))
// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(uint8_t WheelPos) {
  if(WheelPos < 85) {
    return COLOR(WheelPos * 3, 255 - WheelPos * 3, 0);
  }
  else if(WheelPos < 170) {
    WheelPos -= 85;
    return COLOR(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  else {
    WheelPos -= 170;
    return COLOR(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}

static THD_WORKING_AREA(waThread2, 128);
static msg_t Thread2(void *arg)
{
  int i, j = 0;
  uint32_t color;
  (void)arg;

  chRegSetThreadName("text2");
  while (TRUE) {

#if 1    
    j = j % (256 * 5);
    for( i = 0; i < NUMPIX; i++ ) {
      color = Wheel( (i * (256 / NUMPIX) + j) & 0xFF );
      fb[i] = color;
    }
    j += 2;
#else
    for( i = 0; i < NUMPIX; i++ ) {
      if( j == i ) {
	fb[i] = 0xFFFFFF;
      } else
	fb[i] = 0;
    }
    j++;
    j %= NUMPIX;
#endif
    
    /* Blink the LED */
    chSysLock(); // locks out interrupts
    ws2812_send(fb, NUMPIX);
    chSysUnlock(); // restore interrupts

    chThdSleepMilliseconds(20);
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

  /* Start serial, so we can get status output */
  sdStart(serialDriver, &serialConfig);

  shellInit();

  chprintf(stream, "\r\nResetting Phage2014 (Ver %d.%d, git version %s)\r\n", 
      PHAGE2014_OS_VERSION_MAJOR,
      PHAGE2014_OS_VERSION_MINOR,
      gitversion);

  radioStart();

  chprintf(stream, "Launching Thread1...\r\n");
  chThdCreateStatic(waThread1, sizeof(waThread1),
                    NORMALPRIO, Thread1, stream);


  chprintf(stream, "Launching Thread2...\r\n");
  chThdCreateStatic(waThread2, sizeof(waThread2),
                    HIGHPRIO, Thread2, stream);

  chprintf(stream, "Launching Thread3...\r\n");
  chThdCreateStatic(waThread3, sizeof(waThread3),
                    NORMALPRIO, Thread3, stream);

  chprintf(stream, "Launching Thread4...\r\n");
  chThdCreateStatic(waThread4, sizeof(waThread4),
                    NORMALPRIO, Thread4, stream);

#if 1
  for( i = 0; i < 0x28; i += 4 ) {
    chprintf(stream, "RCC + %x: %x\r\n", i, *((unsigned int *) (0x40021000 + i)) );
  }
#endif

  while (TRUE) {
    if (shellTerminated()) {
      chprintf(stream, "Spawning new shell (shell #%d)\r\n", i++);
      shellRestart();
    }
    chThdSleepMilliseconds(500);

  }

  return 0;
}
