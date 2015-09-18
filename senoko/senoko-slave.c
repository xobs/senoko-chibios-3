#include "ch.h"
#include "hal.h"

#include "ac.h"
#include "bionic.h"
#include "board-type.h"
#include "power.h"
#include "uart.h"
#include "senoko.h"
#include "senoko-events.h"
#include "senoko-slave.h"
#include "senoko-wdt.h"

/* Save ISR-enable values across boot.  Shared with power.c. */
static uint32_t *power_state = ((uint32_t *)(0x40006c00 + 0x18));

/* Mask: 0b[s][b]  s = state, b = button */
#define POWER_BUTTON_PRESSED_ID 0
#define POWER_BUTTON_RELEASED_ID 1
#define AC_UNPLUGGED_ID 2
#define AC_CONNETED_ID 3
#define POWERED_OFF_ID 4
#define POWERED_ON_ID 5

static void update_irq(void) {
  if (registers.irq_status)
      palWritePad(GPIOA, PA0, 1);
  else
      palWritePad(GPIOA, PA0, 0);
}

static void button_event(eventid_t id) {

  if (id == POWER_BUTTON_PRESSED_ID)
    registers.power |= REG_POWER_PB_STATUS_MASK;
  else if (id == POWER_BUTTON_RELEASED_ID)
    registers.power &= ~REG_POWER_PB_STATUS_MASK;

  if (registers.irq_enable & REG_IRQ_KEYPAD_MASK)
    registers.irq_status |= REG_IRQ_KEYPAD_MASK;

  update_irq();
}

static void ac_event(eventid_t id) {

  if (id == AC_UNPLUGGED_ID)
    registers.power &= ~REG_POWER_AC_STATUS_MASK;
  else if (id == AC_CONNETED_ID)
    registers.power |= REG_POWER_AC_STATUS_MASK;

  if (registers.irq_enable & REG_IRQ_POWER_MASK)
    registers.irq_status |= REG_IRQ_POWER_MASK;

  update_irq();
}

static void power_event(eventid_t id) {

  registers.power &= ~REG_POWER_STATE_MASK;

  if (id == POWERED_OFF_ID)
    registers.power |= REG_POWER_STATE_OFF;
  else if (id == POWERED_ON_ID)
    registers.power |= REG_POWER_STATE_ON;

  update_irq();
}

static evhandler_t evthandler[] = { 
  button_event, /* Power button pressed */
  button_event, /* Power button released */
  ac_event, /* AC connected */
  ac_event, /* AC unplugged */
  power_event,  /* Powered off */
  power_event,  /* Powered on */
};

static event_listener_t event_listener[6];

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
    if (offset == REG_POWER) {
      /* Power control */
      if ((b[count] & REG_POWER_KEY_MASK) == REG_POWER_KEY_WRITE) {

        if (b[count] & REG_POWER_WDT_ENABLE) {
          senokoWatchdogEnable();
          registers.power |= REG_POWER_WDT_MASK;
        }
        else {
          senokoWatchdogDisable();
          registers.power &= ~REG_POWER_WDT_MASK;
        }

        switch (b[count] & REG_POWER_STATE_MASK) {
        case REG_POWER_STATE_OFF:
          powerOffI();
          break;

        case REG_POWER_STATE_ON:
          powerOnI();
          break;

        case REG_POWER_STATE_REBOOT:
          powerRebootI();
          break;

        default:
          break;
        }
      }
    }
    else if (offset == REG_IRQ_ENABLE) {
      /* Save IRQ enable status across boots. */
      *power_state = ((*power_state) & 0x00ff) | ((b[count] << 8) & 0xff00);
      ((uint8_t *)&registers)[offset] = b[count];
    }
    else if (offset == REG_IRQ_STATUS) {
      /* IRQ status values (allow user to clear status) */
      ((uint8_t *)&registers)[offset] = b[count];
    }
    else if (offset == REG_WATCHDOG_SECONDS) {
      /* Watchdog */
      senokoWatchdogSet(b[count]);
      ((uint8_t *)&registers)[offset] = b[count];
    }
    else if (offset == REG_UART) {
      /* UART */
      switch (b[count] & REG_UART_STATE_MASK) {
      case REG_UART_STATE_ON:
        uartOn();
        ((uint8_t *)&registers)[offset] = REG_UART_STATE_ON;
        break;
      
      case REG_UART_STATE_OFF:
        uartOff();
        ((uint8_t *)&registers)[offset] = REG_UART_STATE_OFF;
        break;
      
      default:
        break;
      }
    }
    else if ( (offset >= 0x10 && offset < 0x11) || 
         (offset >= 0x14 && offset < 0x1b)) {
      /* GPIO registers */
      ((uint8_t *)&registers)[offset] = b[count];
    }

    offset++;
  }

  update_irq();
}

void senokoSlavePrepTransaction(void) {
  memcpy(registers.uptime, &senoko_uptime, sizeof(registers.uptime));
  registers.wdt_seconds = senokoWatchdogTimeToReset();
  if (senokoWatchdogEnabled())
    registers.power |= REG_POWER_WDT_ENABLE;
  else
    registers.power &= ~REG_POWER_WDT_ENABLE;
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
  chEvtRegister(&powered_off, &event_listener[4], POWERED_OFF_ID);
  chEvtRegister(&powered_on, &event_listener[5], POWERED_ON_ID);

  while (TRUE)
    chEvtDispatch(evthandler, chEvtWaitOne(ALL_EVENTS));

  return MSG_OK;
}

void senokoSlaveInit(void) {

  registers.signature = 'S';
  registers.version_major = SENOKO_OS_VERSION_MAJOR;
  registers.version_minor = SENOKO_OS_VERSION_MINOR;

  if (boardType() == senoko_full)
    registers.features = REG_FEATURES_BATTERY;
  else
    registers.features = REG_FEATURES_GPIO;

  registers.power = (acPlugged() << REG_POWER_AC_STATUS_SHIFT)
                  | ((!palReadPad(GPIOB, PB14)) << REG_POWER_PB_STATUS_SHIFT)
                  | REG_POWER_KEY_READ;

  registers.irq_enable = (*power_state) >> 8;

  chThdCreateStatic(waI2cSlaveThread, sizeof(waI2cSlaveThread),
                          70, i2c_slave_thread, NULL);

}
