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
#include "chprintf.h"

#include "bionic.h"
#include "board-type.h"
#include "chg.h"
#include "senoko.h"
#include "senoko-i2c.h"

void cmd_chg(BaseSequentialStream *chp, int argc, char *argv[]) {
  int ret;

  if (boardType() != senoko_full) {
    chprintf(chp, "Charger not present on this board.\r\n");
    return;
  }


  if (argc >= 3 && !strcasecmp(argv[0], "set")) {
    uint32_t current, voltage, input;

    input = 65536; /* invalid value */
    current = strtoul(argv[1], NULL, 0);
    voltage = strtoul(argv[2], NULL, 0);
    if (argc > 3)
      input = strtoul(argv[3], NULL, 0);

    /* Figure/check current */
    if (current > 8064) {
      chprintf(chp, "Error: That's too much current\r\n");
      goto out;
    }
    /* Figure/check voltage */
    if (voltage > 19200) {
      chprintf(chp, "Error: 19.2V is the max voltage\r\n");
      goto out;
    }

    if (input != 65536) {

      /* Figure/check input current */
      if (input > 11004) {
        chprintf(chp,
        "Error: 11004 mA is the max supported input current\r\n");
        goto out;
      }
      if (input < 256) {
        chprintf(chp, "Error: Input current must be at least 256 mA\r\n");
        goto out;
      }

      chprintf(chp, "Setting charger: %dmA @ %dmV (input: %dmA)... ",
          current, voltage, input);
      ret = chgSetAll(current, voltage, input);
    }
    else {
      chprintf(chp, "Setting charger: %dmA @ %dmV... ", current, voltage);
      ret = chgSet(current, voltage);
    }

    if (ret < 0)
      chprintf(chp, "Error: 0x%x\r\n", ret);
    else
      chprintf(chp, "Ok\r\n");
  }

  else if (argc == 2 && !strcasecmp(argv[0], "input")) {
    uint32_t input = strtoul(argv[1], NULL, 0);

    /* Figure/check input current */
    if (input > 11004) {
      chprintf(chp,
      "Error: 11004 mA is the max supported input current\r\n");
      goto out;
    }
    if (input < 256) {
      chprintf(chp, "Error: Input current must be at least 256 mA\r\n");
      goto out;
    }

    chprintf(chp, "Setting charger input current to %dmA\r\n", input);
    ret = chgSetInput(input);
  }

  else if (argc == 1 && !strcasecmp(argv[0], "pause")) {
    chprintf(chp, "Pausing charging\r\n");
    chgPause();
  }

  else if (argc == 1 && !strcasecmp(argv[0], "resume")) {
    chprintf(chp, "Resuming charging\r\n");
    chgResume();
  }

  else {
    uint16_t word = 0;
    uint16_t current, voltage, input;

    chprintf(chp, "Charger information:\r\n");
    if (chgPaused())
      chprintf(chp, "\tCharge thread:    PAUSED\r\n");
    else
      chprintf(chp, "\tCharge thread:    running\r\n");

    senokoI2cAcquireBus();

    ret = chgGetManuf(&word);
    if (ret < 0)
      chprintf(chp, "\tError getting manufacturer: 0x%x\r\n", ret);
    else
      chprintf(chp, "\tManufacturer ID:  0x%04x\r\n", word);

    chgGetDevice(&word);
    chprintf(chp, "\tDevice ID:        0x%04x\r\n", word);

    ret = chgRefresh(&current, &voltage, &input);
    chprintf(chp, "\tCurrent:          %d mA\r\n", current);
    chprintf(chp, "\tVoltage:          %d mV\r\n", voltage);
    chprintf(chp, "\tInput:            %d mA\r\n", input);

    chprintf(chp, "Command usage:\r\n");
    chprintf(chp, "  chg set [c] [v] [[i]]  Set charge current to c mA @ v mV\r\n");
    chprintf(chp, "  chg input [i]          Set max input AC current to [i] mA\r\n");
    chprintf(chp, "  chg pause              Pause charge control\r\n");
    chprintf(chp, "  chg resume             Resume charge control\r\n");
  }


out:
  senokoI2cReleaseBus();
  return;
}
