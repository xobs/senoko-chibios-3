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

#define THREAD_SLEEP_MS 2500

/* Defaults for this board, for an unconfigured gas gauge.*/
#define CELL_COUNT 3
#define CELL_CAPACITY 5000
#define CHARGE_CURRENT 5000
#define WALL_CURRENT 3750

/* Minimum amount of current to move from 'normal discharge' to 'charging'.*/
#define KICKSTART_VOLTAGE 12600
#define KICKSTART_CURRENT 128

/* This is the ABSOLUTE MAXIMUM voltage allowed for each cell.*/
#define MV_MAX 4200
#define MV_MIN 3000

/* Voltages and currents for waking up gas gauge when it's asleep.*/
#define CHARGE_GG_WAKEUP_CURRENT 1024
#define CHARGE_GG_WAKEUP_VOLTAGE 12600

static uint16_t g_current;
static uint16_t g_voltage;
static uint16_t g_input;

static int chg_getblock(uint8_t reg, void *data, int size) {
  if (senokoI2cMasterTransmitTimeout(CHG_ADDR,
                                     &reg, sizeof(reg),
                                     data, size))
    return senokoI2cErrors();
  return 0;
}

static int chg_setblock(void *data, int size) {
  if (senokoI2cMasterTransmitTimeout(CHG_ADDR,
                                     data, size,
                                     NULL, 0))
    return senokoI2cErrors();
  return 0;
}

static int chg_set_voltage(uint16_t voltage) {
  uint8_t bfr[3];

  if (voltage > 19200)
    return -1;

  voltage &= 0x7ff0;

  g_voltage = voltage;
  bfr[0] = 0x15;
  bfr[1] = g_voltage;
  bfr[2] = g_voltage >> 8;
  return chg_setblock(bfr, sizeof(bfr));
}

static int chg_set_current(uint16_t current) {
  uint8_t bfr[3];

  if (current > 8064)
    return -1;

  current &= 0x1f80;

  g_current = current;
  bfr[0] = 0x14;
  bfr[1] = g_current;
  bfr[2] = g_current >> 8;
  return chg_setblock(bfr, sizeof(bfr));
}

static int chg_set_input(uint16_t input) {
  uint8_t bfr[3];

  if (input > 11004)
    return -1;

  input >>= 1;
  input &= 0x1f80;

  g_input = input;
  bfr[0] = 0x3f;
  bfr[1] = g_input;
  bfr[2] = g_input >> 8;
  return chg_setblock(bfr, sizeof(bfr));
}

int chgSetAll(uint16_t current, uint16_t voltage, uint16_t input) {

  int ret = 0;

  ret |= chg_set_current(current);
  ret |= chg_set_voltage(voltage);
  ret |= chg_set_input(input);

  return ret;
}

int chgSet(uint16_t current, uint16_t voltage) {
  int ret = 0;

  ret |= chg_set_current(current);
  ret |= chg_set_voltage(voltage);

  return ret;
}

int chgRefresh(uint16_t *current, uint16_t *voltage, uint16_t *input) {

  int ret = 0;

  ret |= chg_getblock(0x3f, &g_input, 2);
  ret |= chg_getblock(0x14, &g_current, 2);
  ret |= chg_getblock(0x15, &g_voltage, 2);

  if (input)
    *input = g_input << 1;

  if (current)
    *current = g_current;

  if (voltage)
    *voltage = g_voltage;

  return ret;
}

int chgGetManuf(uint16_t *word) {
  *word = 0;
  return chg_getblock(0xfe, word, 2);
}

int chgGetDevice(uint16_t *word) {
  *word = 0;
  return chg_getblock(0xff, word, 2);
}

static void set_chgfet(int newval) {
  ggSetChgFET(newval);
}

static void set_dsgfet(int newval) {
  ggSetDsgFET(newval);
}

#include "chprintf.h"
static THD_WORKING_AREA(waChgThread, 256);
static msg_t chg_thread(void *arg) {
  (void)arg;

  uint16_t status, state;
  chRegSetThreadName("charge controller");
  chThdSleepMilliseconds(200);

  senokoI2cAcquireBus();
  chg_set_input(WALL_CURRENT);

  while (1) {
    int ret;
    static uint16_t cell_capacity;
    static uint16_t voltage, current;

    senokoI2cReleaseBus();
    chThdSleepMilliseconds(THREAD_SLEEP_MS);
    senokoI2cAcquireBus();

    /*
     * Use the design capacity to determine if the gas gauge has been
     * programmed or not.
     * We can also use this to determine if the gas gauge is responding
     * at all.
     */
    cell_capacity = 0;
    ret = ggDesignCapacity(&cell_capacity);
    if (ret != MSG_OK) {

      /* Try again.  The bus might just be busy, or the GG is off. */
      ret = ggDesignCapacity(&cell_capacity);
      if (ret != MSG_OK) {
        /*
         * If we failed twice in a row to get the cell capacity, then the
         * gas gauge might be asleep.  It does that sometimes, particularly
         * on first powerup.
         * Turn on the charger to ensure the gas gauge wakes up.  Reset the
         * error count, because this isn't a charge-related error.
         */
        chgSet(CHARGE_GG_WAKEUP_CURRENT, CHARGE_GG_WAKEUP_VOLTAGE);
        continue;
      }
    }

    /* 6336 is watt-hours */
    if ((cell_capacity != 6336) && (cell_capacity != CELL_CAPACITY)) {
      chprintf(stream, "Cell capacity is %d, not %d.  Setting defaults...\r\n", cell_capacity, CELL_CAPACITY);
      ggSetDefaults(CELL_COUNT, CELL_CAPACITY, CHARGE_CURRENT);
      continue;
    }

    /*
     * The gas gauge will only attempt to charge if the current is positive.
     * In Senoko, a "neutral" system is one in which the current is around
     * -8 mA.
     * If the board is in discharge mode, AC is connected, and if charging
     * is allowed, then turn the charger on at low voltage.
     */
    if (ggStatus(&status))
      continue;

    /* Don't have the "terminate charge alarm" or "overcharge alarm".*/
    if ( (!(status & (1 << 14))) && (!(status & (1 << 15))) ) {
      if (ggState(&state))
        continue;

      /* State is "normal discharge" */
      if ((state & 0xf) == 1) {

        uint16_t chgstatus;
        if (ggChargingStatus(&chgstatus))
          continue;

        if ( !(chgstatus & (1 << 15)) ) {
          chgSet(KICKSTART_CURRENT, KICKSTART_VOLTAGE);
          set_chgfet(1);
          continue;
        }
      }
    }

    /*
     * Since we're having to manually kickstart things, we have to
     * manually manipulate the FETs as well.  Enable the DSG FET if
     * the system doesn't think we should turn off.
     */
    if (status & ((1 << 11) | (1 << 9) | (1 << 8)))
      set_dsgfet(0);
    else
      set_dsgfet(1);

    voltage = 0;
    if (ggVoltage(&voltage))
      continue;
    if ((!voltage) || (voltage == 0x5555))
      continue;
    if (voltage < 9000)
      set_dsgfet(0);
    else
      set_dsgfet(1);

    /* Figure out what the gas gauge wants us to charge at.*/
    ret = ggChargingVoltage(&voltage);
    if (ret)
      continue;

    ret = ggChargingCurrent(&current);
    if (ret)
      continue;

    if (!current && !voltage)
      set_chgfet(0);
    else
      set_chgfet(1);
    chgSet(current, voltage);
  }
  return 0;
}

int chgPresent(void) {
  uint16_t word;
  return !chg_getblock(0xff, &word, 2);
}

void chgInit(void) {
  chThdCreateStatic(waChgThread, sizeof(waChgThread),
                    HIGHPRIO - 10, chg_thread, NULL);
}
