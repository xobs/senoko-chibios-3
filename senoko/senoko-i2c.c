#include "ch.h"
#include "hal.h"
#include "i2c.h"

#if !defined(HAL_USE_I2C)
#error "I2C is not enabled"
#endif

#define i2cBus (&I2CD2)

static const I2CConfig senokoI2CHost = {
  OPMODE_SMBUS_HOST,
  100000,
  STD_DUTY_CYCLE,
};

static const I2CConfig senokoI2CDevice = {
  OPMODE_SMBUS_DEVICE,
  100000,
  STD_DUTY_CYCLE,
};

void senokoI2cInit(void)
{
  i2cStart(i2cBus, &senokoI2CHost);
  return;
}
