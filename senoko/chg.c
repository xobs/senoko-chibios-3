#include "ch.h"
#include "hal.h"
#include "i2c.h"

#include "chg.h"
#include "gg.h"
#include "bionic.h"
#include "senoko.h"
#include "senoko-i2c.h"
#include "power.h"

#define CHG_ADDR 0x9

/* SMBus times out after 25ms in hardware */
static const systime_t tmo = TIME_INFINITE;

#define THREAD_SLEEP_MS 1100
#define MAX_ERRORS 10

/* If we're under 95%, charge the battery */
#define CAPACITY_TO_CHARGE 95

/* If the current is less than this, enable the charger */
#define CURRENT_TO_CHARGE 1

/* Rate (in mA) at which the battery can charge */
#define CHARGE_CURRENT 3000

/* Target voltage (in mV) of the battery */
#define CHARGE_VOLTAGE 12600

/* Rate (in mA) at which the wall can supply current */
#define CHARGE_WALL_CURRENT 3420

/* This is the ABSOLUTE MAXIMUM voltage allowed for each cell */
#define MV_MAX 4200
#define MV_MIN 3000
static uint16_t g_current;
static uint16_t g_voltage;
static uint16_t g_input;

static int chg_getblock(uint8_t reg, void *data, int size) {
  return senokoI2cMasterTransmitTimeout(CHG_ADDR,
                                        &reg, sizeof(reg),
                                        data, size,
                                        tmo);
}

static int chg_setblock(void *data, int size) {
  return senokoI2cMasterTransmitTimeout(CHG_ADDR,
                                        data, size,
                                        NULL, 0,
                                        tmo);
}

int chgSet(uint16_t current, uint16_t voltage, uint16_t input) {
  int ret;
  uint8_t bfr[3];


  if (current > 8064)
    return -1;

  if (voltage > 19200)
    return -1;

  if (input > 11004)
    return -1;

  current &= 0x1f80;

  voltage &= 0x7ff0;

  input >>= 1;
  input &= 0x1f80;

  senokoI2cAcquireBus();

  g_input = input;
  bfr[0] = 0x3f;
  bfr[1] = g_input;
  bfr[2] = g_input >> 8;
  ret = chg_setblock(bfr, sizeof(bfr));
  if (ret)
    goto out;

  g_current = current;
  bfr[0] = 0x14;
  bfr[1] = g_current;
  bfr[2] = g_current >> 8;
  ret = chg_setblock(bfr, sizeof(bfr));
  if (ret)
    goto out;

  g_voltage = voltage;
  bfr[0] = 0x15;
  bfr[1] = g_voltage;
  bfr[2] = g_voltage >> 8;
  ret = chg_setblock(bfr, sizeof(bfr));
  if (ret)
    goto out;

out:
  senokoI2cReleaseBus();
  return ret;
}

int chgRefresh(uint16_t *current, uint16_t *voltage, uint16_t *input) {
  if (input)
    chg_getblock(0x3f, input, 2);
  if (current)
    chg_getblock(0x14, current, 2);
  if (voltage)
    chg_getblock(0x15, voltage, 2);
  if (input)
    *input <<= 1;
  return 0;
}

int chgGetManuf(uint16_t *word) {
  return chg_getblock(0xfe, word, 2);
}

int chgGetDevice(uint16_t *word) {
  return chg_getblock(0xff, word, 2);
}

static THD_WORKING_AREA(waChgThread, 256);
static msg_t chg_thread(void *arg) {
  int error_count = 0;
  (void)arg;

  chRegSetThreadName("charge controller");
  chThdSleepMilliseconds(200);

  while (1) {
    int16_t cell_mv;
    int cell;
    int ret;
    uint8_t cell_count;
    uint8_t capacity;
    int16_t current;

    chThdSleepMilliseconds(THREAD_SLEEP_MS);

    /*
     * Determine the battery's capacity and charge current,
     * so we can decide if we need to charge or not.
     */
    ret = ggPercent(&capacity);
    if (ret < 0) {
      error_count++;
      capacity = CAPACITY_TO_CHARGE;
    }

    ret = ggAverageCurrent(&current);
    if (ret < 0) {
      error_count++;
      current = CURRENT_TO_CHARGE;
    }

    /* Charge the battery if necessary */
    if (capacity < CAPACITY_TO_CHARGE
     && current < CURRENT_TO_CHARGE
     && (g_current == 0 || g_voltage == 0)) {
      chgSet(CHARGE_CURRENT, CHARGE_VOLTAGE, CHARGE_WALL_CURRENT);
      continue;
    }

    if (error_count > MAX_ERRORS)
      chgSet(0, 0, g_input << 1);


    /* Feed the charger watchdog timer */
    chgSet(g_current, g_voltage, g_input << 1);
    ret = ggCellCount(&cell_count);
    if (ret) {
      error_count++;
      continue;
    }

    /*
     * Examine each cell to determine if it's undervoltage or
     * overvoltage.  If it's undervoltage, cut power to the
     * mainboard.  For overvoltage, stop charging.
     */
    for (cell = 1; cell <= cell_count; cell++) {
      ret = ggCellVoltage(cell, &cell_mv);
      if (ret) {
        error_count++;
        continue;
      }

      /*
       * If any one cell goes below the minimum, shut down
       * everything.  These cells are very prone to expanding,
       * and we don't want to stress them.
       */
      if (cell_mv > 0 && cell_mv < MV_MIN
          && (!g_current || !g_voltage))
        powerOff();

      /* If we exceed the max voltage, shut down charging */
      if (cell_mv > MV_MAX && (g_current || g_voltage))
        chgSet(0, 0, g_input << 1);
    }

    error_count = 0;
  }
  return 0;
}

void chgInit(void) {
  chThdCreateStatic(waChgThread, sizeof(waChgThread),
                    HIGHPRIO, chg_thread, NULL);
}
