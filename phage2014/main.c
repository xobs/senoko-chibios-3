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
#include "phage-accel.h"
#include "phage-adc.h"
#include "phage-events.h"
#include "phage-i2c.h"
#include "phage-radio.h"
#include "phage-shell.h"
#include "phage-wdt.h"
#include "phage-ui.h"

#define LED_COUNT 60

static uint8_t framebuffer[LED_COUNT * 3];

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

static void accel_int1_handler(eventid_t id) {
  (void)id;
  chprintf(stream, "A"); // reduce chatter, improve performance and stability but allow for debug
  bump(100);
}

static void accel_int2_handler(eventid_t id) {
  (void)id;
  chprintf(stream, " [Accel IRQ 2] ");
}


// up transmits beacon
// left/right control pattern
// down controls intensity

static void key_up_handler(eventid_t id) { // right
  (void)id;
  chprintf(stream, "R");
  //  effectsSetPattern(patternShoot);
  effectsNextPattern();
}

static void key_down_handler(eventid_t id) { // left
  (void)id;
  chprintf(stream, "L");
  effectsPrevPattern();
  //  effectsSetPattern(patternCalm);
}

static void key_left_handler(eventid_t id) { // I call this "up"
  (void)id;
  chprintf(stream, "U");
  //  effectsSetPattern(patternLarson);  // don't change effect, just page
}

static void key_right_handler(eventid_t id) {  // I call this "down"
  uint8_t s;
  (void)id;
  chprintf(stream, "D");
  s = getShift();
  if( s > 5 )
    s = 0;
  s++;
  setShift(s);
}

static void radio_carrier_detect_handler(eventid_t id) {
  (void)id;
  chprintf(stream, " [Radio Carrier Detect!!] ");
  effectsSetPattern(patternCalm);
}

static void radio_data_received_handler(eventid_t id) {
  (void)id;
  chprintf(stream, " [Radio Data Received!!] ");
  effectsSetPattern(patternCalm);
}

static void radio_address_matched_handler(eventid_t id) {
  (void)id;
  chprintf(stream, " [Radio Address Matched!!] ");
  effectsSetPattern(patternCalm);
}

static evhandler_t event_handlers[] = {
  shell_termination_handler,
  power_button_pressed_handler,
  power_button_released_handler,
  accel_int1_handler,
  accel_int2_handler,
  key_up_handler,
  key_down_handler,
  key_left_handler,
  key_right_handler,
  //  radio_carrier_detect_handler,
  //  radio_data_received_handler,
  //  radio_address_matched_handler,
};

static event_listener_t event_listeners[ARRAY_SIZE(event_handlers)];

#define REG_AFIO  (0x40010000)
#define REG_PORTA (0x40010800)
#define AFIO_MAPR  (REG_AFIO + 0x4)
#define PORTA_CRH  (REG_PORTA + 0x4)

void debugme(void) {
  int i;
#if 0
  for( i = 0; i < 7; i++ ) {
    chprintf(stream, "GPIOA%d: %08x\r\n", i, *((unsigned int *)(REG_PORTA + i*4)) );
  }

  chprintf(stream, "\r\n" );
  for( i = 0; i < 6; i++ ) {
    chprintf(stream, "AFIO%d: %08x\r\n", i, *((unsigned int *)(REG_AFIO + i*4)) );
  }

  chprintf(stream, "\r\n" );
#endif
  *((unsigned int *) AFIO_MAPR) =  (*((unsigned int *) AFIO_MAPR) & 0xF8FFFFFF) | 0x04000000;
  *((unsigned int *) PORTA_CRH) =  (*((unsigned int *) PORTA_CRH) & 0x0FFFFFFF) | 0x40000000;

#if 0
  for( i = 0; i < 7; i++ ) {
    chprintf(stream, "GPIOA%d: %08x\r\n", i, *((unsigned int *)(REG_PORTA + i*4)) );
  }

  chprintf(stream, "\r\n" );
  for( i = 0; i < 6; i++ ) {
    chprintf(stream, "AFIO%d: %08x\r\n", i, *((unsigned int *)(REG_AFIO + i*4)) );
  }
#endif
}

void debugme2(void) {

  while(TRUE) {
    chprintf(stream, "%d\r\n", palReadPad(GPIOA, PA15) );
    chprintf(stream, "GPIOA%d: %08x\r\n", 8, *((unsigned int *)(REG_PORTA + 8)) );
  }
}

/*
 * Application entry point.
 */
int main(void) {

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
#if 1
  phageEventsInit();
  chEvtRegister(&power_button_pressed, &event_listeners[1], 1);
  chEvtRegister(&power_button_released, &event_listeners[2], 2);
  chEvtRegister(&accel_int1, &event_listeners[3], 3);
  chEvtRegister(&accel_int2, &event_listeners[4], 4);
  chEvtRegister(&key_up_pressed, &event_listeners[5], 5);
  chEvtRegister(&key_down_pressed, &event_listeners[6], 6);
  chEvtRegister(&key_left_pressed, &event_listeners[7], 7);
  chEvtRegister(&key_right_pressed, &event_listeners[8], 8);
  //  chEvtRegister(&radio_carrier_detect, &event_listeners[9], 9);
  //  chEvtRegister(&radio_data_received, &event_listeners[10], 10);
  //  chEvtRegister(&radio_address_matched, &event_listeners[11], 11);
#endif

  chprintf(stream, "\r\nStarting Phage (Ver %d.%d, git version %s)\r\n", 
      PHAGE_OS_VERSION_MAJOR,
      PHAGE_OS_VERSION_MINOR,
      gitversion);

  /* Start I2C, which is necessary for accelerometer.*/
  phageI2cInit();

  /* Now that I2C is running, start the accelerometer.*/
  phageAccelInit();

  ledDriverInit(LED_COUNT, GPIOB, 0b11, framebuffer);
  chprintf(stream, "\tFramebuffer address: 0x%08x\r\n", framebuffer);
  ledDriverStart(framebuffer);

  /* Start the Phage watchdog timer thread.*/
  phageWatchdogInit();

  //phageAdcInit();

  /* Start LED effects.*/
  effectsStart(framebuffer, LED_COUNT);

  radioStart();

  /* Enter main event loop.*/
  phageShellRestart();

  phageUiInit(); // start the UI interaction loop (RF events, buttons)

  debugme();
  while (TRUE) 
    chEvtDispatch(event_handlers, chEvtWaitOne(ALL_EVENTS));


  return 0;
}
