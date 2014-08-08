/*
    ChibiOS/RT - Copyright (C) 2006-2013 Giovanni Di Sirio

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#include "ch.h"
#include "hal.h"
#include "phage-accel.h"
#include "chprintf.h"

#include "phage-i2c.h"
#define ACCEL_ADDR 0x1d

void cmd_accel(BaseSequentialStream *chp, int argc, char *argv[]) {
  (void)argc;
  (void)argv;
  uint32_t x, y, z;

  uint8_t reg[1];
  uint8_t data[5];
  uint8_t wreg[2];

  phageAccelRead(&x, &y, &z);
  chprintf(chp, "X: %d\r\n", x);
  chprintf(chp, "Y: %d\r\n", y);
  chprintf(chp, "Z: %d\r\n", z);

  reg[0] = 0x15;
  phageI2cMasterTransmitTimeout(ACCEL_ADDR,
                                      reg, sizeof(reg),
                                      data, sizeof(data));
  chprintf(chp, "%02x %02x %02x %02x %02x\r\n", data[0], data[1], data[2], data[3], data[4] );

  reg[0] = 0x2A;
  phageI2cMasterTransmitTimeout(ACCEL_ADDR,
                                      reg, sizeof(reg),
                                      data, sizeof(data));
  chprintf(chp, "%02x %02x %02x %02x %02x\r\n", data[0], data[1], data[2], data[3], data[4] );

  wreg[0] = 0x15;
  wreg[1] = 0x60;
  phageI2cMasterTransmitTimeout(ACCEL_ADDR,
                                wreg, sizeof(wreg),
                                NULL, 0);

  return;
}
