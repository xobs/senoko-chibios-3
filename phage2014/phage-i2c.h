#ifndef __PHAGE_I2C_H__
#define __PHAGE_I2C_H__

#include "ch.h"
#include "hal.h"
#include "i2c.h"

void phageI2cInit(void);
msg_t phageI2cMasterTransmitTimeout(i2caddr_t addr,
                                    const uint8_t *txbuf, size_t txbytes,
                                    uint8_t *rxbuf, size_t rxbytes);
void phageI2cAcquireBus(void);
void phageI2cReleaseBus(void);
int phageI2cErrors(void);

#endif /* __PHAGE_I2C_H__ */
