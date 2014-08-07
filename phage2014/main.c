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
<<<<<<< HEAD
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
  chprintf(stream, " [Accel IRQ 1] ");
}

static void accel_int2_handler(eventid_t id) {
  (void)id;
  chprintf(stream, " [Accel IRQ 2] ");
}

static void key_up_handler(eventid_t id) {
  (void)id;
  chprintf(stream, " [Key Up] ");
  effectsSetPattern(patternShoot);
}

static void key_down_handler(eventid_t id) {
  (void)id;
  chprintf(stream, " [Key Down] ");
  effectsSetPattern(patternCalm);
}

static void key_left_handler(eventid_t id) {
  (void)id;
  chprintf(stream, " [Key Left] ");
  effectsSetPattern(patternLarson);
  radioSend('x');
}

static void key_right_handler(eventid_t id) {
  (void)id;
  chprintf(stream, " [Key Right] ");
  effectsSetPattern(patternTest);
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
=======
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
>>>>>>> 4177a65a07b748bb28ca7f5533e1ca3dadba5e2c
}

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
  /* Start up serial console.*/
  phageShellInit();
  chEvtRegister(&shell_terminated, &event_listeners[0], 0);

  /* Listen to GPIO events (e.g. button presses, status changes).*/
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

  chprintf(stream, "\r\nStarting Phage (Ver %d.%d, git version %s)\r\n", 
      PHAGE_OS_VERSION_MAJOR,
      PHAGE_OS_VERSION_MINOR,
      gitversion);

  /* Start I2C, which is necessary for accelerometer.*/
  phageI2cInit();

  /* Now that I2C is running, start the accelerometer.*/
  phageAccelInit();

  radioStart();

  ledDriverInit(LED_COUNT, GPIOB, 0b11, framebuffer);
  chprintf(stream, "\tFramebuffer address: 0x%08x\r\n", framebuffer);
  ledDriverStart(framebuffer);

  /* Start the Phage watchdog timer thread.*/
  phageWatchdogInit();

  //phageAdcInit();

  /* Start LED effects.*/
  effectsStart(framebuffer, LED_COUNT);

  /* Enter main event loop.*/
  phageShellRestart();

  debugme();
  while (TRUE)
    chEvtDispatch(event_handlers, chEvtWaitOne(ALL_EVENTS));
=======
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
>>>>>>> 4177a65a07b748bb28ca7f5533e1ca3dadba5e2c

  return 0;
}
