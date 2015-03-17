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

static const char *gpio_name(int bank, int pad) {
  switch (bank) {

  case 'A':
  case 0:
    switch (pad) {
      case 0: return "CHG_IRQ";
      case 4: return "ANA_METER";
      case 5: return "ADC_IN5";
      case 6: return "ADC_IN6";
      case 7: return "ADC_IN7";
      case 8: return "CHG_ACOK";
      case 9: return "USART1_TX";
      case 10: return "USART1_RX";
      case 11: return "GG_SYSPRES";
      case 12: return "CHG_CE";
      case 13: return "JTMS/SWDIO";
      case 14: return "JTCK/SWCLK";
      case 15: return "JTDI";
      default: return "";
    }

  case 'B':
  case 1:
    switch (pad) {
      case 0: return "CHG_ICOUT";
      case 2: return "REPROGRAM";
      case 3: return "PM_JTDO";
      case 4: return "PM_JRST";
      case 8: return "PM_REFLASH_ALRT";
      case 10: return "CHG_SCL";
      case 11: return "CHG_SDA";
      case 12: return "CHG_ALERT";
      case 14: return "CHG_PWRSWITCH";
      case 15: return "CHG_MASTERPWR";
      default: return "";
    }

  case 'C':
  case 2:
    switch (pad) {
      case 13: return "TAMPER/RTC";
      case 14: return "PM_OSC32_IN";
      case 15: return "PM_OSC32_OUT";
      default: return "";
    } 

  default:
    return "";
  }
}

static int gpio_set(int bank, int pad, int val)
{
  switch(bank) {
  case 'A':
  case 0:
    palWritePad(GPIOA, pad, val);
    break;
  case 'B':
  case 1:
    palWritePad(GPIOB, pad, val);
    break;
  case 'C':
  case 2:
    palWritePad(GPIOC, pad, val);
    break;
  case 'D':
  case 3:
    palWritePad(GPIOD, pad, val);
    break;
  case 'E':
  case 4:
    palWritePad(GPIOE, pad, val);
    break;
  default:
    return -1;
  }
  return 0;
}

static int gpio_get(int bank, int pad)
{
  switch(bank) {
  case 'A':
  case 0:
    return palReadPad(GPIOA, pad);
    break;
  case 'B':
  case 1:
    return palReadPad(GPIOB, pad);
    break;
  case 'C':
  case 2:
    return palReadPad(GPIOC, pad);
    break;
  case 'D':
  case 3:
    return palReadPad(GPIOD, pad);
    break;
  case 'E':
  case 4:
    return palReadPad(GPIOE, pad);
    break;
  default:
    return -1;
  }
}

void cmd_gpio(BaseSequentialStream *chp, int argc, char *argv[]) {
  int bank, pad;
  (void)argc;
  (void)argv;

  if ((argc > 0) && ((argv[0][0] == 'P') || (argv[0][0] == 'p'))) {
    int val;
    int pad;
    char bank;

    bank = toupper(argv[0][1]);
    pad = strtoul(&argv[0][2], NULL, 10);
    if ((bank < 'A') || (bank > 'E')) {
      chprintf(chp, "Usage: gpio PAD [val]\r\n");
      chprintf(chp, "       Where PAD is a pad such as PA0 or PB15, and\r\n");
      chprintf(chp, "       the optional [val] is a value to write,\r\n");
      chprintf(chp, "       either 0 or 1. If omitted, the value is read.\r\n");
      return;
    }

    if (argc > 1) {
      val = strtoul(argv[1], NULL, 0);
      chprintf(chp, "Setting P%c%d (%s) to %d\r\n", bank, pad,
          gpio_name(bank, pad), val);
      gpio_set(bank, pad, !!val);
    }
    else {
      val = gpio_get(bank, pad);
      chprintf(chp, "Value at P%c%d (%s): %d\r\n", bank, pad,
          gpio_name(bank, pad), val);
    }
  }
  else {
    for (bank = 'A'; bank <= 'E'; bank++) {
      chprintf(chp, "GPIO %c:\r\n", bank);
      for (pad = 0; pad < 16; pad++) {
        chprintf(chp, "    P%c%d: %d  %s\r\n", bank, pad, gpio_get(bank, pad),
          gpio_name(bank, pad));
      }
    }
  }
}
