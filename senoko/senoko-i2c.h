#ifndef __SENOKO_I2C_H__
#define __SENOKO_I2C_H__

#include "ch.h"
#include "hal.h"
#include "i2c.h"

#ifdef I2C_LOGGING
#define I2C_LOG_ENTRIES 40
#define I2C_LOG_DATA_SIZE 6
struct i2clog_entry {
  uint8_t type;
  uint8_t size;
  uint8_t data[I2C_LOG_DATA_SIZE];
};

struct i2clog {
  uint32_t head;
  struct i2clog_entry entries[I2C_LOG_ENTRIES];
};

extern struct i2clog i2clog;
#define I2C_ENTRY_TYPE_NONE 0
#define I2C_ENTRY_TYPE_READ 1
#define I2C_ENTRY_TYPE_WRITE 2
#define I2C_ENTRY_TYPE_START 3
#endif /* I2C_LOGGING */

void senokoI2cInit(void);
msg_t senokoI2cMasterTransmitTimeout(i2caddr_t addr,
                                     const uint8_t *txbuf, size_t txbytes,
                                     uint8_t *rxbuf, size_t rxbytes);
void senokoI2cAcquireBus(void);
void senokoI2cReleaseBus(void);
int senokoI2cErrors(void);

#endif /* __SENOKO_I2C_H__ */
