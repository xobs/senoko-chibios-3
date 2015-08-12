#include "ch.h"
#include "hal.h"
#include "i2c.h"

#include "senoko.h"
#include "power.h"
#include "bionic.h"
#include "senoko-slave.h"
#include "senoko-i2c.h"

#if !HAL_USE_I2C
#error "I2C is not enabled"
#endif

#define i2cBus (&I2CD2)

static binary_semaphore_t master_slave_sem;
static binary_semaphore_t i2c_bus_sem;

/*
 * A note on slave/master sharing:
 *
 *    - System is nominally in slave mode
 *    - When system is powered off, system goes into master mode
 *    - When system is powered on, system goes into slave mode
 *    - When a slave transaction occurs, take a semaphore
 *    - When a slave transaction finishes, release the semaphore
 */

static const systime_t timeout = MS2ST(25);

struct i2c_registers registers;
static uint8_t i2c_buffer[sizeof(registers) + 1];

enum client_mode {
  I2C_MODE_NONE,
  I2C_MODE_IDLE,
  I2C_MODE_SLAVE,
  I2C_MODE_MASTER,
};
enum client_mode client_mode;

static const I2CConfig senokoI2cMode = {
  OPMODE_I2C,
  100000,
  STD_DUTY_CYCLE,
};

#ifdef I2C_LOGGING
struct i2clog i2clog;
static void senokoI2cLogAppend(struct i2clog *log, int type,
                               void *buffer, size_t bytes)
{
  memcpy(log->entries[log->head].data, buffer, bytes);
  log->entries[log->head].size = bytes;
  log->entries[log->head].type = type;
    log->head++;
  if (log->head > I2C_LOG_ENTRIES)
    log->head = 0;
}
#else /* ! I2C_LOGGING */
#define senokoI2cLogAppend(log, type, buffer, bytes)
#endif /* ! I2C_LOGGING */

static void i2c_transaction_start(I2CDriver *i2cp)
{
  (void)i2cp;
  senokoI2cLogAppend(&i2clog, I2C_ENTRY_TYPE_START, NULL, 0);
  chSysLockFromISR();
  chBSemResetI(&master_slave_sem, 1);
  senokoSlavePrepTransaction();
  chSysUnlockFromISR();
}

static void i2c_rx_finished(I2CDriver *i2cp, size_t bytes)
{
  (void)i2cp;

  senokoI2cLogAppend(&i2clog, I2C_ENTRY_TYPE_READ, i2c_buffer, bytes);

  /* Shouldn't ever happen.*/
  if (!bytes)
    return;

  if (bytes) {
    uint8_t addr = i2c_buffer[0];

    if (bytes > 1)
      senokoSlaveDispatch(i2c_buffer, bytes);

    i2cSlaveSetTxOffset(i2cp, addr + bytes - 1);
  }

  chSysLockFromISR();
  chBSemSignalI(&master_slave_sem);
  senokoSlavePrepTransaction();
  chSysUnlockFromISR();
}

static void i2c_tx_finished(I2CDriver *i2cp, size_t bytes)
{
  (void)i2cp;
  (void)bytes;
  chSysLockFromISR();
  chBSemSignalI(&master_slave_sem);

  senokoI2cLogAppend(&i2clog, I2C_ENTRY_TYPE_WRITE, i2c_buffer, bytes);

  chSysUnlockFromISR();
}

static void senoko_i2c_mode_slave(void)
{
  /*
   * Only enable I2C slave mode when the mainboard is on.  The gas gauge
   * goes to sleep if it detects that there's nothing on the bus, which
   * is what it looks like when we're a slave device.
   */
  if (powerIsOn()) {
    i2cStop(i2cBus);
    i2cStart(i2cBus, &senokoI2cMode);
    i2cSlaveIoTimeout(i2cBus, SENOKO_I2C_SLAVE_ADDR,

                      /* Tx buffer */
                      (uint8_t *)&registers, sizeof(registers),

                      /* Rx buffer */
                      i2c_buffer, sizeof(i2c_buffer),

                      /* Event-done callbacks */
                      i2c_tx_finished, i2c_rx_finished,

                      /* Transfer-start callback */
                      i2c_transaction_start,

                      /* Timeout */
                      TIME_INFINITE);
  }
}

static THD_WORKING_AREA(waI2cUnstickThread, 128);
static msg_t i2c_unstick_thread(void *arg) {
  (void)arg;
  int stuck_count = 0;
  I2C_TypeDef *dp = i2cBus->i2c;

  chRegSetThreadName("unstick i2c");

  do {
    if (dp->SR2 & I2C_SR2_BUSY)
      stuck_count++;
    else
      stuck_count = 0;

    if (stuck_count > 4) {
      senoko_i2c_mode_slave();
      stuck_count = 0;
    }

    chThdSleepMilliseconds(20);
  } while(1);

  return MSG_OK;
}


void senokoI2cInit(void)
{
  chBSemObjectInit(&master_slave_sem, 0);
  chBSemObjectInit(&i2c_bus_sem, 0);
  i2cStart(i2cBus, &senokoI2cMode);
  senoko_i2c_mode_slave();

  chThdCreateStatic(waI2cUnstickThread, sizeof(waI2cUnstickThread),
                    HIGHPRIO - 15, i2c_unstick_thread, NULL);

  return;
}

/**
 * @brief   Pauses I2C slave mode to send data via the I2C bus.
 * @details Function designed to realize "read-through-write" transfer
 *          paradigm. If you want transmit data without any further read,
 *          than set @b rxbytes field to 0.
 *
 * @param[in] i2cp      pointer to the @p I2CDriver object
 * @param[in] addr      slave device address (7 bits) without R/W bit
 * @param[in] txbuf     pointer to transmit buffer
 * @param[in] txbytes   number of bytes to be transmitted
 * @param[out] rxbuf    pointer to receive buffer
 * @param[in] rxbytes   number of bytes to be received, set it to 0 if
 *                      you want transmit only
 *                      .
 *
 * @return              The operation status.
 * @retval MSG_OK       if the function succeeded.
 * @retval MSG_RESET    if one or more I2C errors occurred, the errors can
 *                      be retrieved using @p i2cGetErrors().
 * @retval MSG_TIMEOUT  if a timeout occurred before operation end.
 *
 * @api
 */
#define I2C_MAX_TRIES 50
msg_t senokoI2cMasterTransmitTimeout(i2caddr_t addr,
                                     const uint8_t *txbuf,
                                     size_t txbytes,
                                     uint8_t *rxbuf,
                                     size_t rxbytes) {
  msg_t ret = MSG_OK;
  uint8_t rxbuf_hack[2];
  uint8_t *rxbuf_orig;
  int rxbuf_do_hack;
  int tries;

  /* I2C locks up if we transfer only one byte, so copy at least two bytes.*/
  if (rxbytes == 1) {
    rxbytes = 2;
    rxbuf_do_hack = 1;
    rxbuf_orig = rxbuf;
    rxbuf = rxbuf_hack;
  }
  else
    rxbuf_do_hack = 0;

  /* Work around DMA bug, where it locks up if we transfer only one byte.*/
  if (rxbytes == 1)
    rxbytes = 2;

  /* Try multiple times, since this is a multi-master system.*/
  for (tries = 0; tries < I2C_MAX_TRIES; tries++) {

    chBSemWait(&master_slave_sem);
    i2cStop(i2cBus);
    i2cStart(i2cBus, &senokoI2cMode);

    /* Perform the transaction (now operating in master mode).*/
    ret = i2cMasterTransmitTimeout(i2cBus, addr,
                                   txbuf, txbytes,
                                   rxbuf, rxbytes,
                                   timeout);
    chBSemSignal(&master_slave_sem);
    if (ret == MSG_OK)
      break;

  }

  /* Fixup one-byte copies (if necessary).*/
  if (rxbuf_do_hack)
    rxbuf_orig[0] = rxbuf_hack[0];

  senoko_i2c_mode_slave();
  return ret;
}

void senokoI2cAcquireBus(void) {
//  i2cAcquireBus(i2cBus);
  chBSemWait(&i2c_bus_sem);
}

void senokoI2cReleaseBus(void) {
  chBSemSignal(&i2c_bus_sem);
//  i2cReleaseBus(i2cBus);
}

int senokoI2cErrors(void) {
  return i2cGetErrors(i2cBus);
}
