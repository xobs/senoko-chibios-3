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
#include "gg.h"
#include "senoko.h"
#include "senoko-i2c.h"

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

static void print_str(BaseSequentialStream *chp,
                          const char *item,
                          void *func,
                          const char *fmt) {
  static uint8_t bfr[32];
  int ret;
  int (*_func)(void *) = func;

  ret = _func(bfr);
  if (ret < 0)
    chprintf(chp, "%-19s error 0x%x\r\n", item, ret);
  else {
    chprintf(chp, "%-19s ", item);
    chprintf(chp, fmt, bfr);
    chprintf(chp, "\r\n");
  }

  return;
}

static void print_byte(BaseSequentialStream *chp,
                       const char *item,
                       int (*func)(uint8_t *),
                       const char *fmt) {
  int ret;
  uint8_t bfr;

  ret = func(&bfr);
  if (ret)
    chprintf(chp, "%-19s error 0x%x\r\n", item, ret);
  else {
    chprintf(chp, "%-19s ", item);
    chprintf(chp, fmt, bfr);
    chprintf(chp, "\r\n");
  }

  return;
}

static void print_word(BaseSequentialStream *chp,
                       const char *item,
                       int (*func)(uint16_t *),
                       const char *fmt) {
  int ret;
  uint16_t bfr;

  ret = func(&bfr);
  if (ret)
    chprintf(chp, "%-19s error 0x%x\r\n", item, ret);
  else {
    chprintf(chp, "%-19s ", item);
    chprintf(chp, fmt, bfr);
    chprintf(chp, "\r\n");
  }

  return;
}

static void print_signed_word(BaseSequentialStream *chp,
                              const char *item,
                              int (*func)(int16_t *),
                              const char *fmt) {
  int ret;
  int16_t bfr;

  ret = func(&bfr);
  if (ret)
    chprintf(chp, "%-19s error 0x%x\r\n", item, ret);
  else {
    chprintf(chp, "%-19s ", item);
    chprintf(chp, fmt, bfr);
    chprintf(chp, "\r\n");
  }

  return;
}

void cmd_stats(BaseSequentialStream *chp, int argc, char *argv[]) {
  uint16_t word;
  int cell;
  int ret;
  (void)argc;
  (void)argv;

  if (boardType() != senoko_full) {
    chprintf(chp, "Gas gauge not present on this board.\r\n");
    return;
  }

  senokoI2cAcquireBus();

  print_str(chp, "Manufacturer:", ggManufacturer, "%s");
  print_str(chp, "Part name:", ggPartName, "%s");
  print_word(chp, "Firmware version:", ggFirmwareVersion, "0x%x");
  print_word(chp, "Config version:", ggConfigVersion, "0x%x");

  ret = ggState(&word);
  if (ret)
    chprintf(chp, "State:              error 0x%x\r\n", ret);
  else {
    int chgfet, dsgfet;
    switch ((word >> 6) & 3) {
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
    chprintf(chp, "Charge FET:         %s\r\n", chgfet ? "on" : "off");
    chprintf(chp, "Discharge FET:      %s\r\n", dsgfet ? "on" : "off");
    chprintf(chp, "State:              %s\r\n", mfgr_states[word & 0xf]);
    if ((word & 0xf) == 0x9) {
      chprintf(chp, "Permanent failure:  %s\r\n",
          permafailures[(word >> 4) & 3]);

      print_word(chp, "Fuse flag:", ggFuseFlag, "0x%x");
      print_word(chp, "PF flags:", ggPermanentFailureFlags, "0x%x");
      print_word(chp, "PF flags 2:", ggPermanentFailureFlags2, "0x%x");
      print_word(chp, "PF voltage:", ggPermanentFailureFlags, "%d mV");
      print_signed_word(chp, "PF current:", ggPermanentFailureCurrent, "%d mA");
      print_signed_word(chp, "PF temperature:", ggPermanentFailureTemperature, "%d mC");
      print_word(chp, "PF remaining capacity:", ggPermanentFailureRemainingCapacity, "%d mA");
      print_word(chp, "PF battery status:", ggPermanentFailureBatteryStatus, "0x%x");
      print_word(chp, "PF charge status:", ggPermanentFailureChargeStatus, "0x%x");
      print_word(chp, "PF safety status:", ggPermanentFailureSafetyStatus, "0x%x");
//      int ggPermanentFailureCellVoltage(int cell, uint16_t *voltage);

    }
  }

  print_word(chp, "Time until full:", ggTimeToFull, "%d minutes");
  print_word(chp, "Time until empty:", ggTimeToEmpty, "%d minutes");
  print_str(chp, "Chemistry:", ggChemistry, "%s");
  print_word(chp, "Serial number:", ggSerial, "0x%04x");
  print_word(chp, "Cycle count:", ggCycleCount, "%u");
  print_word(chp, "Battery health:", ggHealth, "%d%%");
  {
    uint8_t byte;
    if (ggWakeCurrent(&byte))
      chprintf(chp, "Wake current:       error\r\n");
    else if (!byte)
      chprintf(chp, "Wake current:       disabled\r\n");
    else {
      const char *current[] = { "Disabled", "2.5", "5", "10" };
      const char *thresh[] = {"0.5", "1.0"};
      chprintf(chp, "Wake current:       %sA, %s mOhm\r\n",
          thresh[(byte >> 2) & 1],
          current[byte & 3]);
    }
  }
  print_byte(chp, "Charge:", ggPercent, "%d%%");

  ggMode(&word);
  if (word & (1 << 15)) {
    print_word(chp, "Max capacity:", ggFullCapacity, "%d0 mWh");
    print_word(chp, "Design capacity:", ggDesignCapacity, "%d0 mWh");
  }
  else {
    print_word(chp, "Max capacity:", ggFullCapacity, "%d mAh");
    print_word(chp, "Design capacity:", ggDesignCapacity, "%d mAh");
  }

  {
    int16_t temperature;
    ret = ggTemperature(&temperature);
    chprintf(chp, "Temperature:        %d.%d C\r\n",
             temperature / 10,
             temperature - (10 * (temperature / 10)));
  }

  print_word(chp, "Voltage:", ggVoltage, "%d mV");
  print_signed_word(chp, "Current:", ggCurrent, "%d mA");
  print_signed_word(chp, "Average current:", ggAverageCurrent, "%d mA");
  print_word(chp, "Target voltage:", ggChargingVoltage, "%d mV");
  print_word(chp, "Target current:", ggChargingCurrent, "%d mA");

  print_byte(chp, "Number of cells:", ggCellCount, "%d cells");
  for (cell = 1; cell <= 4; cell++) {
    ret = ggCellVoltage(cell, &word);
    if (ret < 0)
      chprintf(chp, "Cell %d voltage:     error 0x%x\r\n",
          cell, ret);
    else
      chprintf(chp, "Cell %d voltage:     %d mV\r\n",
          cell, word);
  }
  {
    uint16_t status;
    ggChargingStatus(&status);
    chprintf(chp, "Charge status:      0x%x\r\n", status);
    chprintf(chp, "    Charging allowed?   %s\r\n", (status & (1 << 15)) ?
                                                "no" : "yes");
    chprintf(chp, "    Can suspend?        %s\r\n", (status & (1 << 14)) ?
                                                "suspended" : "no");
    chprintf(chp, "    Can precharge?      %s\r\n", (status & (1 << 13)) ?
                                                "yes" : "no");
    chprintf(chp, "    Can maintenance?    %s\r\n", (status & (1 << 12)) ?
                                                "yes" : "no");
    chprintf(chp, "    Temperature limit?  %s\r\n", (status & (1 << 11)) ?
                                                "yes" : "no");
    chprintf(chp, "    Temperature limit?  %s\r\n", (status & (1 << 10)) ?
                                                "yes" : "no");
    chprintf(chp, "    Can fastcharge?     %s\r\n", (status & (1 << 9)) ?
                                                "yes" : "no");
    chprintf(chp, "    Pulse charging?     %s\r\n", (status & (1 << 8)) ?
                                                "yes" : "no");
    chprintf(chp, "    Pulse disable CHG?  %s\r\n", (status & (1 << 7)) ?
                                                "yes" : "no");
    chprintf(chp, "    Cell balancing?     %s\r\n", (status & (1 << 6)) ?
                                                "in-progress" : "no");
    chprintf(chp, "    Precharge timeout?  %s\r\n", (status & (1 << 5)) ?
                                                "yes" : "no");
    chprintf(chp, "    Fastcharge timeout? %s\r\n", (status & (1 << 4)) ?
                                                "yes" : "no");
    chprintf(chp, "    Overcharge OV?      %s\r\n", (status & (1 << 3)) ?
                                                "yes" : "no");
    chprintf(chp, "    Overcharge OC?      %s\r\n", (status & (1 << 2)) ?
                                                "yes" : "no");
    chprintf(chp, "    Overcharge?         %s\r\n", (status & (1 << 1)) ?
                                                "yes" : "no");
    chprintf(chp, "    Battery empty?      %s\r\n", (status & (1 << 0)) ?
                                                "yes" : "no");
  }

  chprintf(chp, "Alarms:\r\n");
  ret = ggStatus(&word);
  /* N.b.: stat byte is swapped.*/
  if (word & (1 << 15))
    chprintf(chp, "    OVERCHARGED ALARM\r\n");
  if (word & (1 << 14))
    chprintf(chp, "    TERMINATE CHARGE ALARM\r\n");
  if (word & (1 << 12))
    chprintf(chp, "    OVER TEMP ALARM\r\n");
  if (word & (1 << 11))
    chprintf(chp, "    TERMINATE DISCHARGE ALARM\r\n");
  if (word & (1 << 9))
    chprintf(chp, "    REMAINING CAPACITY ALARM\r\n");
  if (word & (1 << 8))
    chprintf(chp, "    REMAINING TIME ALARM\r\n");

  chprintf(chp, "Charge state:\r\n");
  if (word & (1 << 7))
    chprintf(chp, "    Battery initialized\r\n");
  if (word & (1 << 6))
    chprintf(chp, "    Battery discharging/relaxing\r\n");
  if (word & (1 << 5))
    chprintf(chp, "    Battery fully charged\r\n");
  if (word & (1 << 4))
    chprintf(chp, "    Battery fully discharged\r\n");

  if (word & 0xf)
    chprintf(chp, "STATUS ERROR CODE: 0x%x\r\n", word & 0xf);
  else
    chprintf(chp, "No errors detected\r\n");

  ret = ggSafetyAlert(&word);
  if (ret) {
    chprintf(chp, "Unable to read safety alerts\r\n");
  }
  else if (word) {
    chprintf(chp, "Safety alerts:\r\n");
    if (word & (1 << 15))
      chprintf(chp, "    Discharge overtemperature alert\r\n");
    if (word & (1 << 14))
      chprintf(chp, "    Charge overtemperature alert\r\n");
    if (word & (1 << 13))
      chprintf(chp, "    Discharge overcurrent alert\r\n");
    if (word & (1 << 12))
      chprintf(chp, "    Charge overcurrent alert\r\n");
    if (word & (1 << 11))
      chprintf(chp, "    Tier-2 discharge overcurrent  alert\r\n");
    if (word & (1 << 10))
      chprintf(chp, "    Tier-2 charge overcurrent  alert\r\n");
    if (word & (1 << 9))
      chprintf(chp, "    Pack undervoltage alert\r\n");
    if (word & (1 << 8))
      chprintf(chp, "    Pack overvoltage alert\r\n");
    if (word & (1 << 7))
      chprintf(chp, "    Cell undervoltage alert\r\n");
    if (word & (1 << 6))
      chprintf(chp, "    Permanent failure alert\r\n");
    if (word & (1 << 5))
      chprintf(chp, "    Cell overvoltage alert\r\n");
    if (word & (1 << 4))
      chprintf(chp, "    Host watchdog alert\r\n");
    if (word & (1 << 3))
      chprintf(chp, "    AFE watchdog alert\r\n");
    if (word & (1 << 2))
      chprintf(chp, "    AFE discharge overcurrent alert\r\n");
    if (word & (1 << 1))
      chprintf(chp, "    Charge short-circuit alert\r\n");
    if (word & (1 << 0))
      chprintf(chp, "    Discharge short-circuit alert\r\n");
  }
  else
    chprintf(chp, "No safety alerts\r\n");

  ret = ggSafetyStatus(&word);
  if (ret) {
    chprintf(chp, "Unable to read safety status\r\n");
  }
  else if (word) {
    chprintf(chp, "Safety status:\r\n");
    if (word & (1 << 15))
      chprintf(chp, "    Discharge overtemperature condition\r\n");
    if (word & (1 << 14))
      chprintf(chp, "    Charge overtemperature condition\r\n");
    if (word & (1 << 13))
      chprintf(chp, "    Discharge overcurrent condition\r\n");
    if (word & (1 << 12))
      chprintf(chp, "    Charge overcurrent condition\r\n");
    if (word & (1 << 11))
      chprintf(chp, "    Tier-2 discharge overcurrent condition\r\n");
    if (word & (1 << 10))
      chprintf(chp, "    Tier-2 charge overcurrent condition\r\n");
    if (word & (1 << 9))
      chprintf(chp, "    Pack undervoltage condition\r\n");
    if (word & (1 << 8))
      chprintf(chp, "    Pack overvoltage condition\r\n");
    if (word & (1 << 7))
      chprintf(chp, "    Cell undervoltage condition\r\n");
    if (word & (1 << 6))
      chprintf(chp, "    Permanent failure condition\r\n");
    if (word & (1 << 5))
      chprintf(chp, "    Cell overvoltage condition\r\n");
    if (word & (1 << 4))
      chprintf(chp, "    Host watchdog condition\r\n");
    if (word & (1 << 3))
      chprintf(chp, "    AFE watchdog condition\r\n");
    if (word & (1 << 2))
      chprintf(chp, "    AFE discharge overcurrent condition\r\n");
    if (word & (1 << 1))
      chprintf(chp, "    Charge short-circuit condition\r\n");
    if (word & (1 << 0))
      chprintf(chp, "    Discharge short-circuit condition\r\n");
  }
  else
    chprintf(chp, "No safety status messages\r\n");


  senokoI2cReleaseBus();

  return;
}
