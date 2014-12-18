#include "ch.h"
#include "hal.h"
#include "i2c.h"

#include "ac.h"
#include "chg.h"
#include "gg.h"
#include "bionic.h"
#include "senoko.h"
#include "senoko-i2c.h"
#include "power.h"

#define CHG_ADDR 0x9

/* Number of milliseconds between runthrus of the charger thread */
#define THREAD_SLEEP_MS 2500

/* Number of times to try looking for the gas gauge during startup */
#define CHG_TRIES 50

/* How much current the wall adapter supplies */
#define WALL_CURRENT 3750

/* Minimum amount of current to move from 'normal discharge' to 'charging'.*/
#define KICKSTART_VOLTAGE 12600
#define KICKSTART_CURRENT 128

/* Voltages and currents for waking up gas gauge when it's asleep.*/
#define CHARGE_GG_WAKEUP_CURRENT 1024
#define CHARGE_GG_WAKEUP_VOLTAGE 12600

#define mark_line \
  do { \
    *((uint32_t *)(0x40006c00 + 0x28)) = 7; \
    *((uint32_t *)(0x40006c00 + 0x24)) = __LINE__; \
  } while(0)

static uint16_t g_current;
static uint16_t g_voltage;
static uint16_t g_input;
static bool chg_paused;
static bool chg_present;

static int chg_getblock(uint8_t reg, void *data, int size) {
  if (senokoI2cMasterTransmitTimeout(CHG_ADDR,
                                     &reg, sizeof(reg),
                                     data, size))
    return senokoI2cErrors() | 0x80000000;
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

int chgSetInput(uint16_t current) {
  return chg_set_input(current);
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

void chgPause(void) {
  chg_paused = true;
}

void chgResume(void) {
  chg_paused = false;
}

bool chgPaused(void) {
  return chg_paused;
}

static THD_WORKING_AREA(waChgThread, 256);
static msg_t chg_thread(void *arg) {
  (void)arg;

  chRegSetThreadName("charge controller");
  chThdSleepMilliseconds(200);

  senokoI2cAcquireBus();
  chg_set_input(WALL_CURRENT);

  while (1) {
    int ret;
    static uint16_t cell_capacity;
    static uint16_t voltage, current;
    static uint16_t state;
    static int16_t termvolt;
    static enum gg_state system_state = -1;

    senokoI2cReleaseBus();
    chThdSleepMilliseconds(THREAD_SLEEP_MS);
    senokoI2cAcquireBus();

    if (chg_paused)
      continue;

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

    ret = ggState(&state);
    if (ret != MSG_OK)
      continue;

    /*
     * When the system transitions into the wake_up state, ensure
     * that ImpedenceTrack is running.
     */
    if (((state & 0xf) == st_wake_up) && system_state != st_wake_up) {
      system_state = (state & 0xf);
      ggStartImpedenceTrackTM();
      continue;
    }

    /* Get the current pack voltage, and the absolute minimum voltage. */
    ret = ggVoltage(&voltage);
    if (ret != MSG_OK)
      continue;
    ret = ggTermVoltage(&termvolt);
    if (ret != MSG_OK)
      continue;

    /* If we're unplugged and low on power, turn off the mainboard. */
    if ((voltage <= termvolt) && powerIsOn() && acRemoved())
      powerOff();

    /* Figure out what the gas gauge wants us to charge at.*/
    ret = ggChargingVoltage(&voltage);
    if (ret)
      continue;

    ret = ggChargingCurrent(&current);
    if (ret)
      continue;

    chgSet(current, voltage);
  }
  return 0;
}

int chgPresent(void) {
  return chg_present;
}

void chgInit(void) {
  int i;

  chg_present = false;

  /*
   * If we're unplugged, then the charger won't respond.  However, it
   * obviously must exist.
   */
  if (acPlugged()) {
    /* If the charger can't refresh, then it likely doesn't exist */
    for (i = 0; (i < CHG_TRIES) && !chg_present; i++) {
      if (!chgRefresh(NULL, NULL, NULL))
        chg_present = true;
      else
        chThdSleepMilliseconds(5);
    }
  }
  else
    chg_present = true;

  /* No charger present means no charger thread to run */
  if (!chg_present)
    return;

  chg_paused = false;
  chThdCreateStatic(waChgThread, sizeof(waChgThread),
                    HIGHPRIO - 10, chg_thread, NULL);
}
