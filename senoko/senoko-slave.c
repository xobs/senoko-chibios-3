#include "ch.h"
#include "hal.h"

#include "senoko.h"
#include "senoko-events.h"
#include "senoko-slave.h"

/* Mask: 0b[s][b]  s = state, b = button */
#define POWER_BUTTON_PRESSED_ID 0
#define POWER_BUTTON_RELEASED_ID 1
#define AC_UNPLUGGED_ID 2
#define AC_CONNETED_ID 3

static void update_irq(void) {
  if (registers.irq_status)
      palWritePad(GPIOA, PA0, 1);
  else
      palWritePad(GPIOA, PA0, 0);
}

static void button_event(eventid_t id) {

  if (id == POWER_BUTTON_PRESSED_ID)
    registers.key_status |= (1 << 0);
  else if (id == POWER_BUTTON_RELEASED_ID)
    registers.key_status &= ~(1 << 0);

  if (registers.ier & (1 << 1))
    registers.irq_status |= (1 << 1);

  update_irq();
}

static void ac_event(eventid_t id) {

  if (id == AC_UNPLUGGED_ID)
    registers.power &= ~(1 << 3);
  else if (id == AC_CONNETED_ID)
    registers.power |= (1 << 3);

  if (registers.ier & (1 << 2))
    registers.irq_status |= (1 << 2);

  update_irq();
}

static evhandler_t evthandler[] = { 
  button_event, /* Power button pressed */
  button_event, /* Power button released */
  ac_event, /* AC connected */
  ac_event, /* AC unplugged */
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

    /* Only certain registers are writeable */
    if ( (offset >= 0x10 && offset < 0x11) || 
         (offset >= 0x14 && offset < 0x19) ||
         (offset == 0x7) ||
         (offset == 0x8) ||
         (offset == 0x9) || (offset == 0xa))
      ((uint8_t *)&registers)[offset] = b[count];

    offset++;
  }

  update_irq();

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
  registers.key_status = ((!palReadPad(GPIOB, PB14)) << 0);
  registers.power = ((!!palReadPad(GPIOA, PA8 )) << 3);

  chThdCreateStatic(waI2cSlaveThread, sizeof(waI2cSlaveThread),
                          70, i2c_slave_thread, NULL);

}
