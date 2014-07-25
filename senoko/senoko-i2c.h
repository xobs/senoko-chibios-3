#ifndef __SENOKO_I2C_H__
#define __SENOKO_I2C_H__

#include "ch.h"
#include "hal.h"
#include "i2c.h"

void senokoI2cInit(void);
msg_t senokoI2cMasterTransmitTimeout(i2caddr_t addr,
                                     const uint8_t *txbuf, size_t txbytes,
                                     uint8_t *rxbuf, size_t rxbytes,
                                     systime_t timeout);
void senokoI2cAcquireBus(void);
void senokoI2cReleaseBus(void);

#endif /* __SENOKO_I2C_H__ */
