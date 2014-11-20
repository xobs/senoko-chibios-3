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
#include "senoko.h"
#include "senoko-i2c.h"
#include "gg.h"

static int is_command(int argc, char *argv[], const char *match) {
  return argc > 0 && !strcasecmp(argv[0], match);
}

void cmd_gg(BaseSequentialStream *chp, int argc, char *argv[]) {
  int ret;

  /* Force the discharge FET on or off.*/
  if (is_command(argc, argv, "dsg")) {

    if (argc == 2 && (argv[1][0] == '+' || argv[1][0] == '-')) {
      if (argv[1][0] == '+') {
        if ( !(ret = ggSetDsgFET(1)))
          chprintf(chp, "Discharge FET forced on\r\n");
      } else {
        if ( ! (ret = ggSetDsgFET(0)))
          chprintf(chp, "Discharge FET allowed to turn off\r\n");
      }
      if (ret < 0)
        chprintf(chp, "Unable to modify  DSG fet: %d\r\n", ret);
    }
    else {
      chprintf(chp, "Usage: gg dsg +/-\r\n");
      return;
    }
  }
  else if (is_command(argc, argv, "chg")) {

    if (argc == 2 && (argv[1][0] == '+' || argv[1][0] == '-')) {
      if (argv[1][0] == '+') {
        if ( !(ret = ggSetChgFET(1)))
          chprintf(chp, "Charge FET forced on\r\n");
      } else {
        if ( ! (ret = ggSetChgFET(0)))
          chprintf(chp, "Charge FET allowed to turn off\r\n");
      }
      if (ret < 0)
        chprintf(chp, "Unable to modify  DSG fet: %d\r\n", ret);
    }
    else {
      chprintf(chp, "Usage: gg chg +/-\r\n");
      return;
    }
  }
  else if (is_command(argc, argv, "tempsource")) {

    if (argc == 2) {
      if (!strcasecmp(argv[1], "internal")) {
        ggSetTemperatureSource(temp_internal);
        return;
      }
      else if (!strcasecmp(argv[1], "ts1")) {
        ggSetTemperatureSource(temp_ts1);
        return;
      }
      else if (!strcasecmp(argv[1], "greater")) {
        ggSetTemperatureSource(temp_greater_ts1_or_ts2);
        return;
      }
      else if (!strcasecmp(argv[1], "average")) {
        ggSetTemperatureSource(temp_average_ts1_and_ts2);
        return;
      }
    }
    chprintf(chp, "Usage: gg tempsource [internal | ts1 | greater | average]\r\n");
    chprintf(chp, "    internal - Use internal temperature sensor\r\n");
    chprintf(chp, "    ts1      - Use temperature sensor attached to TS1\r\n");
    chprintf(chp, "    greater  - Use TS1 or TS2, whichever is greater\r\n");
    chprintf(chp, "    average  - Use the average of TS1 and TS2\r\n");
    return;
  }
  else if (is_command(argc, argv, "templow")) {
    if (argc != 2) {
      int16_t temp;
      ggInhibitLow(&temp);
      chprintf(chp, "Lower-bound temperature inhibit: %d\r\n", temp);
      return;
    }
    ggSetInhibitLow(strtoul(argv[1], NULL, 0));
  }
  else if (is_command(argc, argv, "temphigh")) {
    if (argc != 2) {
      int16_t temp;
      ggInhibitHigh(&temp);
      chprintf(chp, "Upper-bound temperature inhibit: %d\r\n", temp);
      return;
    }
    ggSetInhibitHigh(strtoul(argv[1], NULL, 0));
  }
  else if (is_command(argc, argv, "prechg")) {
    if (argc != 2) {
      int16_t temp;
      ggPrechgTemp(&temp);
      chprintf(chp, "Pre-charge temperature: %d\r\n", temp);
      return;
    }
    ggSetPrechgTemp(strtoul(argv[1], NULL, 0));
  }
  else if (is_command(argc, argv, "deadband")) {
    if (argc != 2) {
      uint8_t db;
      ggDeadband(&db);
      chprintf(chp, "Gas gauge deadband: +/- %d mA\r\n", db);
      return;
    }
    if (ggSetDeadband(strtoul(argv[1], NULL, 0)))
      chprintf(chp, "Error\r\n");
    else
      chprintf(chp, "Ok\r\n");
  }
  else if (is_command(argc, argv, "capacity")) {
    uint16_t capacity;
    int cells;
    if (argc != 3) {
      chprintf(chp, "Usage: gg capacity [cells] [capacity in mAh]\r\n");
      return;
    }
    cells = strtoul(argv[1], NULL, 0);
    capacity = strtoul(argv[2], NULL, 0);

    chprintf(chp, "Setting capacity... ");
    ret = ggSetCapacity(cells, capacity);
    if (ret < 0)
      chprintf(chp, "Unable to set capacity: 0x%x\r\n", ret);
    else
      chprintf(chp, "Set capacity of %d cells to %d mAh\r\n",
          cells, capacity);
  }
  else if (is_command(argc, argv, "cells")) {
    if (argc == 1) {
      chprintf(chp, "Usage: gg cells [3/4]\r\n");
    }
    else {
      if (argv[1][0] == '3') {
        ret = ggSetCellCount(3);
        if (ret < 0)
          chprintf(chp, "Unable to set 3 cells: 0x%x\r\n", ret);
        else
          chprintf(chp, "Set 3-cell mode\r\n");
      }
      else if (argv[1][0] == '4') {
        ret = ggSetCellCount(4);
        if (ret < 0)
          chprintf(chp, "Unable to set 4 cells: 0x%x\r\n", ret);
        else
          chprintf(chp, "Set 4-cell mode\r\n");
      }
      else {
        chprintf(chp, "Unknown cell count: %c\r\n",
            argv[1][0]);
      }
    }
  }
  else if (is_command(argc, argv, "auto")) {
    int handled = 0;
    if (argc == 2) {
      if (argv[1][0] == '1') {
        ggSetBroadcast(1);
        handled = 1;
      }
      else if (argv[1][0] == '0') {
        ggSetBroadcast(0);
        handled = 1;
      }
    }

    if (!handled) {
      chprintf(chp, "Usage: gg auto [0|1]\r\n");
    }
  }
  else if (is_command(argc, argv, "pfreset")) {
    chprintf(chp, "Resetting permanent failure flags...");
    ret = ggPermanentFailureReset();
    if (ret != MSG_OK)
      chprintf(chp, " Error: %x\r\n", ret);
    else
      chprintf(chp, " ok.\r\n");
  }
  else if (is_command(argc, argv, "reset")) {
    chprintf(chp, "Resetting gas gauge completely...");
    ret = ggFullReset();
    if (ret != MSG_OK)
      chprintf(chp, " Error: %x\r\n", ret);
    else
      chprintf(chp, " ok.\r\n");
  }
  else if (is_command(argc, argv, "cal")) {
    if (argc != 4) {
      chprintf(chp, "Usage: gg cal [voltage] [current] [temperature]\r\n",
                    "    voltage - Millivolts of something, not sure what\r\n"
                    "    current - Milliamps (negative) of something, not sure what\r\n"
                    "temperature - Temperature in degrees C\r\n"
          );
    }
    else {
      int16_t voltage;
      int16_t current;
      int16_t cells = 3; /* Only support three-cell configuration */
      uint16_t temperature;

      voltage = strtol(argv[1], NULL, 0);
      current = strtol(argv[2], NULL, 0);
      temperature = (strtol(argv[3], NULL, 0) + 273) * 10;

      chprintf(chp, "Calibrating battery.\r\n");
      chprintf(chp, "        Cells: %d\r\n", cells);
      chprintf(chp, "      Voltage: %d mV\r\n", voltage);
      chprintf(chp, "      Current: %d mA\r\n", current);
      chprintf(chp, "  Temperature: %d dK\r\n", temperature);
      ggCalibrate(voltage, current, temperature, cells);
      chprintf(chp, " ok.\r\n");
    }
  }

  else {
    chprintf(chp,
      "Usage:\r\n"
      "gg dsg +/-       Force dsg fet on or let it turn off\r\n"
      "gg chg +/-       Force chg fet on or let it turn off\r\n"
      "gg cells [3|4]   Set cell count\r\n"
      "gg cal           Calibrate battery pack\r\n"
      "gg templow [t]   Set charge-inhibit low temperature\r\n"
      "gg temphigh [t]  Set charge-inhibit high temperature\r\n"
      "gg prechg [t]    Set pre-chg temperature\r\n"
      "gg pfreset       Reset permanent failure fuse\r\n"
      "gg reset         Reset gas gauge completely\r\n"
      "gg deadband      Modify deadband, which is where 0mA is considered\r\n"
      "gg tempsource    Set how the temperature is sensed\r\n"
      "gg auto [0|1]    Whether the gas gauge can run the charger\r\n"
      );
    return;
  }

}
