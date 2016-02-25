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
#include "senoko.h"
#include "senoko-i2c.h"
#include "gg.h"

static int is_command(int argc, char *argv[], const char *match) {
  return argc > 0 && !strcasecmp(argv[0], match);
}

void cmd_gg(BaseSequentialStream *chp, int argc, char *argv[]) {
  int ret;

  if (boardType() != senoko_full) {
    chprintf(chp, "Gas gauge not present on this board.\r\n");
    return;
  }

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
        ret = ggSetTemperatureSource(temp_internal);
      }
      else if (!strcasecmp(argv[1], "ts1")) {
        ret = ggSetTemperatureSource(temp_ts1);
      }
      else if (!strcasecmp(argv[1], "greater")) {
        ret = ggSetTemperatureSource(temp_greater_ts1_or_ts2);
      }
      else if (!strcasecmp(argv[1], "average")) {
        ret = ggSetTemperatureSource(temp_average_ts1_and_ts2);
      }
      else {
        chprintf(chp, "Unrecognized temp source \"%s\".\r\n", argv[1]);
        return;
      }

      if (ret) {
        chprintf(chp, "Unable to set temp source: 0x%x\r\n", ret);
        return;
      }

      chprintf(chp, "Set tempsource to %s\r\n", argv[1]);
      return;
    }
    chprintf(chp, "Usage: gg tempsource [internal | ts1 | greater | average]\r\n");
    chprintf(chp, "    internal - Use internal temperature sensor\r\n");
    chprintf(chp, "    ts1      - Use temperature sensor attached to TS1\r\n");
    chprintf(chp, "    greater  - Use TS1 or TS2, whichever is greater\r\n");
    chprintf(chp, "    average  - Use the average of TS1 and TS2\r\n");
    return;
  }
  else if (is_command(argc, argv, "wakecurrent")) {
    uint8_t byte;
    if (argc == 3) {
      byte = 0;

      if (!strcasecmp(argv[1], "0.5"))
        byte |= (0 << 2);
      else if (!strcasecmp(argv[1], "1.0"))
        byte |= (1 << 2);
      else {
        chprintf(chp, "Wake current threshold (arg 1) must be 0.5 or 1.0.\r\n");
        return;
      }

      if (!strcasecmp(argv[2], "2.5"))
        byte |= (1 << 0);
      else if (!strcasecmp(argv[2], "5"))
        byte |= (2 << 0);
      else if (!strcasecmp(argv[2], "10"))
        byte |= (3 << 0);
      else {
        chprintf(chp, "Wake current (arg 2) must be 2.5, 5, or 10.\r\n");
        return;
      }

      ret = ggSetWakeCurrent(byte);
      if (ret) {
        chprintf(chp, "Unable to set wake current: 0x%x\r\n", ret);
        return;
      }

      chprintf(chp, "Updated wake current successfully.\r\n");
    }
    else if (argc == 2 && !strcasecmp(argv[1], "off")) {
      byte = 0;
      ret = ggSetWakeCurrent(byte);
      if (ret) {
        chprintf(chp, "Unable to disable wake current: 0x%x\r\n", ret);
        return;
      }
      chprintf(chp, "Disabled wake current.\r\n");
    }
    else if (argc == 1) {
      ret = ggWakeCurrent(&byte);
      if (ret) {
        chprintf(chp, "Unable to get wake current setting: 0x%x\r\n", ret);
        return;
      }
      chprintf(chp, "Wake current: %02x\r\n", byte);
    }
    else {
      chprintf(chp, "Usage: gg wakecurrent [0.5 or 1.0] [2.5 or 5 or 10]\r\n"
          "    Sets the wake threshold to 0.5 or 1.0A at 2.5, 5, or 10 mOhm.\r\n"
          "    Use \"gg wakecurrent off\" to disable.\r\n");
    }
  }
  else if (is_command(argc, argv, "zvchg")) {
    if (argc != 2) {
      uint8_t val;
      ggZeroVoltMode(&val);
      chprintf(chp, "Zero volt mode: %d\r\n", val);
      return;
    }
    ggSetZeroVoltMode(strtoul(argv[1], NULL, 0));
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
  else if (is_command(argc, argv, "current")) {
    int16_t current;
    if (argc != 2) {
      ggFastChargeCurrent(&current);
      chprintf(chp, "Fastcharge current is now: %d mA\r\n", current);
      return;
    }

    current = strtoul(argv[1], NULL, 0);
    chprintf(chp, "Setting fastcharge curent... ");

    ret = ggSetFastChargeCurrent(current);
    if (ret < 0)
      chprintf(chp, "Unable to set fastcharge current: 0x%x\r\n", ret);
    else
      chprintf(chp, "Set fastcharge current to %d mA\r\n", current);
  }
  else if (is_command(argc, argv, "cycle")) {
    uint16_t count;
    if (argc != 2) {
      ggCycleCount(&count);
      chprintf(chp, "Cycle count is currently: %u\r\n", count);
      return;
    }

    count = strtoul(argv[1], NULL, 0);
    chprintf(chp, "Setting cycle count... ");

    ret = ggSetCycleCount(count);
    if (ret < 0)
      chprintf(chp, "Unable to set cycle count: 0x%x\r\n", ret);
    else
      chprintf(chp, "Set cycle count to %u\r\n", count);
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
      chprintf(chp, "Usage: gg cells [2|3|4]\r\n");
    }
    else {
      if (argv[1][0] >= '2' && argv[1][0] <= '4') {
        int cellCount = argv[1][0]-'0';
        ret = ggSetCellCount(cellCount);
        if (ret < 0)
          chprintf(chp, "Unable to set %d cells: 0x%x\r\n", cellCount, ret);
        else
          chprintf(chp, "Set %d-cell mode\r\n",cellCount);
      }
      else {
        chprintf(chp, "Unknown cell count: %c\r\n",
            argv[1][0]);
      }
    }
  }
  else if (is_command(argc, argv, "rm")) {
    int handled = 0;
    if (argc == 2) {
      if (argv[1][0] == '1') {
        ggSetRemovable(1);
        chprintf(chp, "Setting battery as removable\r\n");
        handled = 1;
      }
      else if (argv[1][0] == '0') {
        chprintf(chp, "Setting battery as non-removable\r\n");
        ggSetRemovable(0);
        handled = 1;
      }
    }
    else if (argc == 1) {
      if (ggRemovable())
        chprintf(chp, "Battery is set as removable\r\n");
      else
        chprintf(chp, "Battery is set as non-removable\r\n");
      handled = 1;
    }

    if (!handled) {
      chprintf(chp, "Usage: gg rm [0|1]\r\n");
    }
  }
  else if (is_command(argc, argv, "it")) {
    chprintf(chp, "Starting ImpedenceTrackTM algorithm... ");
    ret = ggStartImpedenceTrackTM();
    if (ret)
      chprintf(chp, "Error: 0x%08x\r\n", ret);
    else
      chprintf(chp, "Ok\r\n");
  }
  else if (is_command(argc, argv, "pfreset")) {
    chprintf(chp, "Resetting permanent failure flags...");
    ret = ggPermanentFailureReset();
    if (ret != MSG_OK)
      chprintf(chp, " Error: %x\r\n", ret);
    else
      chprintf(chp, " ok.\r\n");
  }
  else if (is_command(argc, argv, "reboot")) {
    chprintf(chp, "Rebooting the gas gauge chip...");
    ret = ggReboot();
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
      "gg capacity [m]  Set cell capacity to [mAh]\r\n"
      "gg cells [2|3|4]   Set cell count\r\n"
      "gg current [cur] Set fastcharge current to [cur]\r\n"
      "gg cal           Calibrate battery pack\r\n"
      "gg cycle [count] Set current battery cycle count\r\n"
      "gg zvchg [value] Set the Zero-Volt CHG type\r\n"
      "gg wakecurrent [t] [c]  Set wake current threshold to [t], current [c]\r\n"
      "gg templow [t]   Set charge-inhibit low temperature\r\n"
      "gg temphigh [t]  Set charge-inhibit high temperature\r\n"
      "gg prechg [t]    Set pre-chg temperature\r\n"
      "gg pfreset       Reset permanent failure fuse\r\n"
      "gg reboot        Reboot the gas gauge chip\r\n"
      "gg deadband      Modify deadband, which is where 0mA is considered\r\n"
      "gg tempsource    Set how the temperature is sensed\r\n"
      "gg it            Start a runthrough of the ImpedenceTrack algorithm\r\n"
      "gg rm [0|1]      Set or print whether battery is removable\r\n"
      );
    return;
  }

}
