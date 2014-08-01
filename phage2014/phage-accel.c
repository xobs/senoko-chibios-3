#include "ch.h"
#include "hal.h"
#include "ext.h"
#include "i2c.h"

#include "phage-accel.h"
#include "phage-i2c.h"

#define BUTTON_DEBOUNCE_MS 10

#define ACCEL_ADDR 0x1d

void phageAccelRead(uint32_t *x, uint32_t *y, uint32_t *z) {
  uint8_t reg[1];
  uint8_t data[6];

  reg[0] = 0x01;
  phageI2cMasterTransmitTimeout(ACCEL_ADDR,
                                      reg, sizeof(reg),
                                      data, sizeof(data));
  if (x)
    *x = ((data[0] << 2) & 0x3fc) | ((data[1] >> 6) | 3);
  if (y)
    *y = ((data[2] << 2) & 0x3fc) | ((data[3] >> 6) | 3);
  if (z)
    *z = ((data[4] << 2) & 0x3fc) | ((data[5] >> 6) | 3);
  return;
}


void phageAccelInit(void) {

  uint8_t reg[2];

  /* Enable accelerometer by setting CTRL_REG1.  We only need to enable
     the ACTIVE bit (bit 0).*/
  reg[0] = 0x2a;
  reg[1] = 0x01;
  phageI2cMasterTransmitTimeout(ACCEL_ADDR,
                                      reg, sizeof(reg),
                                      NULL, 0);

  /* Enable portrait/landscape detection.*/
  reg[0] = 0x11;
  reg[1] = 0xc0;
  phageI2cMasterTransmitTimeout(ACCEL_ADDR,
                                      reg, sizeof(reg),
                                      NULL, 0);

  /* Enable freefall detect on all axes.*/
  reg[0] = 0x15;
  reg[1] = 0xf8;
  phageI2cMasterTransmitTimeout(ACCEL_ADDR,
                                      reg, sizeof(reg),
                                      NULL, 0);

  /* Enable accelerometer IRQs.  Enable interupts for both Freefall (bit 2)
     and Orientation (bit 4).*/
  reg[0] = 0x2d;
  reg[1] = 0x14;
  phageI2cMasterTransmitTimeout(ACCEL_ADDR,
                                      reg, sizeof(reg),
                                      NULL, 0);

  /* Map Freefall to IRQ 2.  Leave Orientation on IRQ 1.*/
  reg[0] = 0x2e;
  reg[1] = 0x10;
  phageI2cMasterTransmitTimeout(ACCEL_ADDR,
                                      reg, sizeof(reg),
                                      NULL, 0);

  /* Read the Freefall / Motion Source register to clear pending events.*/
  reg[0] = 0x16;
  phageI2cMasterTransmitTimeout(ACCEL_ADDR,
                                      reg, 1,
                                      NULL, 0);
}
