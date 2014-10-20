#include "ch.h"
#include "hal.h"

#include "senoko.h"
#include "senoko-events.h"
#include "senoko-slave.h"

#define POWER_BUTTON_RELEASED_ID 0
#define AC_CONNETED_ID 1
#define POWER_BUTTON_PRESSED_ID 2
#define AC_UNPLUGGED_ID 3

#include "chprintf.h"

static void button_event(eventid_t id) {
  int button;
  int state;

  button = id & 1;
  state = !!(id & (1 << 1));

  /* Button press */
  if (state) {
    registers.gpio_val_a |= (1 << button);
    if (registers.gpio_irq_rise_a & button)
      registers.gpio_irq_stat_a |= (1 << button);
  }
  /* Button release */
  else {
    registers.gpio_val_a &= ~(1 << button);
    if (registers.gpio_irq_fall_a & button)
      registers.gpio_irq_stat_a |= (1 << button);
  }

  if (registers.gpio_irq_stat_a)
      palWritePad(GPIOA, PA0, 1);
  else
      palWritePad(GPIOA, PA0, 0);

  chprintf(stream, "Got event %d\r\n", id);
}

static evhandler_t evthandler[] = { 
  button_event, /* Power pressed */
  button_event, /* AC connected */
  button_event, /* Power released */
  button_event, /* AC unplugged */
};

static event_listener_t event_listener[5];

void senokoSlaveDispatch(void *bfr, uint32_t size) {
  uint32_t offset;
  uint32_t count;
  uint8_t *b = bfr;

  if (!size)
    return;

  offset = b[0];

  for (count = 1; count < size; count++) {
    offset %= sizeof(registers);

    if ( (offset >= 0x10 && offset < 0x11) || 
         (offset >= 0x14 && offset < 0x19) ||
         (offset == 7) ||
         (offset == 8))
      ((uint8_t *)&registers)[offset] = b[count];

    offset++;
  }

  if (registers.gpio_irq_stat_a)
      palWritePad(GPIOA, PA0, 1);
  else
      palWritePad(GPIOA, PA0, 0);

}

static THD_WORKING_AREA(waI2cSlaveThread, 256);
static msg_t i2c_slave_thread(void *arg) {
  (void)arg;

  chRegSetThreadName("i2c slave thread");
  chThdSleepMilliseconds(300);

  chEvtRegister(&power_button_pressed, &event_listener[0], POWER_BUTTON_PRESSED_ID);
  chEvtRegister(&power_button_released, &event_listener[1], POWER_BUTTON_RELEASED_ID);
  chEvtRegister(&ac_unplugged, &event_listener[2], AC_UNPLUGGED_ID);
  chEvtRegister(&ac_plugged, &event_listener[3], AC_CONNETED_ID);

  while (TRUE)
    chEvtDispatch(evthandler, chEvtWaitOne(ALL_EVENTS));

  return MSG_OK;
}

void senokoSlaveInit(void) {

  registers.signature = 'S';
  registers.version_major = SENOKO_OS_VERSION_MAJOR;
  registers.version_minor = SENOKO_OS_VERSION_MINOR;
  registers.gpio_val_a = ((!palReadPad(GPIOB, PB14)) << 0)
                       | ((!palReadPad(GPIOA, PA8 )) << 1);

  chThdCreateStatic(waI2cSlaveThread, sizeof(waI2cSlaveThread),
                          70, i2c_slave_thread, NULL);

}
