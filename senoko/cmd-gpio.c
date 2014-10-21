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

void cmd_gpio(BaseSequentialStream *chp, int argc, char *argv[]) {
  int bank, pad;
  (void)argc;
  (void)argv;

  if ((argc > 0) && !strcasecmp(argv[0], "set")) {
    chprintf(chp, "Setting PA0\r\n");
    palWritePad(GPIOA, PA0, 1);
  }
  else if ((argc > 0) && !strcasecmp(argv[0], "clr")) {
    chprintf(chp, "Clearing PA0\r\n");
    palWritePad(GPIOA, PA0, 0);
  }
  else if ((argc > 0) && !strcasecmp(argv[0], "val")) {
    chprintf(chp, "Value at PA0: %d\r\n", palReadPad(GPIOA, PA0));
  }
  else {
    for (bank = 'A'; bank <= 'E'; bank++) {
      chprintf(chp, "GPIO %c:\r\n", bank);
      for (pad = 0; pad < 16; pad++) {
        int val;

        if (bank == 'A')
      val = !!palReadPad(GPIOA, pad);
        else if (bank == 'B')
      val = !!palReadPad(GPIOB, pad);
        else if (bank == 'C')
      val = !!palReadPad(GPIOC, pad);

        chprintf(chp, "    P%c%d: %d  %s\r\n", bank, pad, val,
      gpio_name(bank, pad));
      }
    }
  }
}
