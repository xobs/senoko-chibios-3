#include "ch.h"
#include "hal.h"
#include "i2c.h"

#include "phage.h"

#if !HAL_USE_I2C
#error "I2C is not enabled"
#endif

#define i2cBus (&I2CD2)

static const systime_t timeout = MS2ST(25);

static binary_semaphore_t client_sem, transmit_sem;

static const I2CConfig phageI2cMode = {
  OPMODE_I2C,
  100000,
  STD_DUTY_CYCLE,
};

void phageI2cInit(void)
{
  chBSemObjectInit(&client_sem, 0);
  chBSemObjectInit(&transmit_sem, 0);
  i2cStart(i2cBus, &phageI2cMode);
  return;
}

/**
 * @brief   Sends data over I2C on the only bus available
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
#define I2C_MAX_TRIES 10
msg_t phageI2cMasterTransmitTimeout(i2caddr_t addr,
                                    const uint8_t *txbuf,
                                    size_t txbytes,
                                    uint8_t *rxbuf,
                                    size_t rxbytes) {
  msg_t ret;
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

  chBSemWait(&transmit_sem);

  /* Work around DMA bug, where it locks up if we transfer only one byte.*/
  if (rxbytes == 1)
    rxbytes = 2;

  /* Try multiple times, since this is a multi-master system.*/
  for (tries = 0; tries < I2C_MAX_TRIES; tries++) {

    /* Perform the transaction (now operating in master mode).*/
    ret = i2cMasterTransmitTimeout(i2cBus, addr,
                                   txbuf, txbytes,
                                   rxbuf, rxbytes,
                                   timeout);
    if (ret == MSG_OK)
      break;
  }

  /* Fixup one-byte copies (if necessary).*/
  if (rxbuf_do_hack)
    rxbuf_orig[0] = rxbuf_hack[0];

  chBSemSignal(&transmit_sem);

  return ret;
}

void phageI2cAcquireBus(void) {
  chBSemWait(&client_sem);     /* Prevent use of the bus from others.*/
  chBSemWait(&transmit_sem);   /* Prevent transmissions while switching modes.*/

  i2cStop(i2cBus);
  i2cStart(i2cBus, &phageI2cMode);
  i2cAcquireBus(i2cBus);

  chBSemSignal(&transmit_sem); /* Allow transmissions again.*/
}

void phageI2cReleaseBus(void) {
  chBSemWait(&transmit_sem);   /* Prevent transmissions while switching modes.*/

  i2cReleaseBus(i2cBus);
  i2cStop(i2cBus);

  chBSemSignal(&transmit_sem); /* Allow transmissions again.*/
  chBSemSignal(&client_sem);   /* Allow use of the bus.*/
}

int phageI2cErrors(void) {
  return i2cGetErrors(i2cBus);
}
