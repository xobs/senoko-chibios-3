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

static const char *permafailures[] = {
  "fuse is blown",
  "cell imbalance",
  "safety voltage failure",
  "FET failure",
};

static const char *mfgr_states[] = {
  "wake up",
  "normal discharge",
  "???",
  "pre-charge",
  "???",
  "charge",
  "???",
  "charge termination",
  "fault charge terminate",
  "permanent failure",
  "overcurrent",
  "overtemperature",
  "battery failure",
  "sleep",
  "reserved",
  "battery removed",
};

void cmd_stats(BaseSequentialStream *chp, int argc, char *argv[]) {
  uint8_t str[16];
  uint8_t stats[2];
  int16_t word;
  uint16_t minutes;
  uint8_t byte;
  int cell;
  int ret;
  (void)argc;
  (void)argv;

  ret = ggManufacturer(str);
  if (ret < 0)
    chprintf(chp, "Manufacturer:       error 0x%x\r\n", ret);
  else
    chprintf(chp, "Manufacturer:       %s\r\n", str);

  ret = ggPartName(str);
  if (ret < 0)
    chprintf(chp, "Part name:          error 0x%x\r\n", ret);
  else
    chprintf(chp, "Part name:          %s\r\n", str);

  ret = ggFirmwareVersion(&word);
  if (ret < 0)
    chprintf(chp, "Firmware ver:       error 0x%x\r\n", ret);
  else
    chprintf(chp, "Firmware ver:       0x%04x\r\n", word);

  ret = ggState(stats);
  if (ret < 0)
    chprintf(chp, "State:              error 0x%x\r\n", ret);
  else {
    int chgfet, dsgfet;
    switch (stats[0]>>6) {
    case 0:
      chgfet = 1;
      dsgfet = 1;
      break;
    case 1:
      chgfet = 0;
      dsgfet = 1;
      break;
    case 2:
      chgfet = 0;
      dsgfet = 0;
      break;
    case 3:
    default:
      chgfet = 1;
      dsgfet = 0;
      break;
    }
    chprintf(chp, "Charge FET:         %s\r\n", chgfet?"on":"off");
    chprintf(chp, "Discharge FET:      %s\r\n", dsgfet?"on":"off");
    chprintf(chp, "State:              %s\r\n", mfgr_states[stats[0] & 0xf]);
    if ((stats[0] & 0xf) == 0x9)
      chprintf(chp, "PermaFailure:       %s\r\n",
          permafailures[(stats[0] >> 4) & 3]);
  }

  ret = ggTimeToFull(&minutes);
  if (ret < 0)
    chprintf(chp, "Time until full:    error 0x%x\r\n", ret);
  else
    chprintf(chp, "Time until full:    %d minutes\r\n", minutes);

  ret = ggTimeToEmpty(&minutes);
  if (ret < 0)
    chprintf(chp, "Time until empty:   error 0x%x\r\n", ret);
  else
    chprintf(chp, "Time until empty:   %d minutes\r\n", minutes);

  ret = ggChemistry(str);
  if (ret < 0)
    chprintf(chp, "Chemistry:          error 0x%x\r\n", ret);
  else
    chprintf(chp, "Chemistry:          %s\r\n", str);

  ret = ggSerial(&word);
  if (ret < 0)
    chprintf(chp, "Serial number:      error 0x%x\r\n", ret);
  else
    chprintf(chp, "Serial number:      0x%04x\r\n", word);

  ret = ggPercent(&byte);
  if (ret < 0)
    chprintf(chp, "Capacity:           error 0x%x\r\n", ret);
  else
    chprintf(chp, "Capacity:           %d%%\r\n", byte);

  ret = ggFullCapacity(&word);
  if (ret < 0)
    chprintf(chp, "Full capacity:      error 0x%x\r\n", ret);
  else
    chprintf(chp, "Full capacity:      %d mAh\r\n", word);

  ret = ggDesignCapacity(&word);
  if (ret < 0)
    chprintf(chp, "Design capacity:    error 0x%x\r\n", ret);
  else
    chprintf(chp, "Design capacity:    %d mAh\r\n", word);

  ret = ggTemperature(&word);
  if (ret < 0)
    chprintf(chp, "Temperature:        error 0x%x\r\n", ret);
  else
    chprintf(chp, "Temperature:        %d.%d C\r\n", word/10, word-(10*(word/10)));

  ret = ggCellCount(&byte);
  if (ret < 0)
    chprintf(chp, "Cell count:         error 0x%x\r\n", ret);
  else
    chprintf(chp, "Cell count:         %d cells\r\n", byte);

  ret = ggVoltage(&word);
  if (ret < 0)
    chprintf(chp, "Voltage:            error 0x%x\r\n", ret);
  else
    chprintf(chp, "Voltage:            %d mV\r\n", word);

  ret = ggCurrent(&word);
  if (ret < 0)
    chprintf(chp, "Current:            error 0x%x\r\n", ret);
  else
    chprintf(chp, "Current:            %d mA\r\n", word);

  ret = ggChargingCurrent(&word);
  if (ret < 0)
    chprintf(chp, "Charging current:   error 0x%x\r\n", ret);
  else
    chprintf(chp, "Charging current:   %d mA\r\n", word);

  ret = ggChargingVoltage(&word);
  if (ret < 0)
    chprintf(chp, "Charging voltage:   error 0x%x\r\n", ret);
  else
    chprintf(chp, "Charging voltage:   %d mV\r\n", word);

  ret = ggAverageCurrent(&word);
  if (ret < 0)
    chprintf(chp, "Avg current:        error 0x%x\r\n", ret);
  else
    chprintf(chp, "Avg current:        %d mA\r\n", word);

  for (cell=1; cell<=4; cell++) {
    ret = ggCellVoltage(cell, &word);
    if (ret < 0)
      chprintf(chp, "Cell %d voltage:     error 0x%x\r\n",
          cell, ret);
    else
      chprintf(chp, "Cell %d voltage:     %d mV\r\n",
          cell, word);
  }

  chprintf(chp, "Alarms:\r\n");
  ret = ggStatus(stats);
  uint8_t tmp = stats[0];
  stats[0] = stats[1];
  stats[1] = tmp;
  if (stats[0] & (1<<7))
    chprintf(chp, "    OVERCHARGED ALARM\r\n");
  if (stats[0] & (1<<6))
    chprintf(chp, "    TERMINATE CHARGE ALARM\r\n");
  if (stats[0] & (1<<4))
    chprintf(chp, "    OVER TEMP ALARM\r\n");
  if (stats[0] & (1<<3))
    chprintf(chp, "    TERMINATE DISCHARGE ALARM\r\n");
  if (stats[0] & (1<<1))
    chprintf(chp, "    REMAINING CAPACITY ALARM\r\n");
  if (stats[0] & (1<<0))
    chprintf(chp, "    REMAINING TIME ALARM\r\n");

  chprintf(chp, "Charge state:\r\n");
  if (stats[1] & (1<<7))
    chprintf(chp, "    Battery initialized\r\n");
  if (stats[1] & (1<<6))
    chprintf(chp, "    Battery discharging/relaxing\r\n");
  if (stats[1] & (1<<5))
    chprintf(chp, "    Battery fully charged\r\n");
  if (stats[1] & (1<<4))
    chprintf(chp, "    Battery fully discharged\r\n");

  if (stats[1] & 0xf)
    chprintf(chp, "STATUS ERROR CODE: 0x%x\r\n", stats[1]&0xf);
  else
    chprintf(chp, "No errors detected\r\n");

  return;
}
