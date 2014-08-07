#ifndef __SENOKO_I2C_H__
#define __SENOKO_I2C_H__

<<<<<<< HEAD
#include "ch.h"
#include "hal.h"
#include "i2c.h"

void senokoI2cInit(void);
msg_t senokoI2cMasterTransmitTimeout(i2caddr_t addr,
                                     const uint8_t *txbuf, size_t txbytes,
                                     uint8_t *rxbuf, size_t rxbytes);
void senokoI2cAcquireBus(void);
void senokoI2cReleaseBus(void);
int senokoI2cErrors(void);
=======
void senokoI2cInit(void);
>>>>>>> 4177a65a07b748bb28ca7f5533e1ca3dadba5e2c

#endif /* __SENOKO_I2C_H__ */
