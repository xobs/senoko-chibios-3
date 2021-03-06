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
#include "iwdg.h"
#include "chprintf.h"

static const IWDGConfig watchdogConfig = {
  MS2ST(1), /* counter */
  IWDG_DIV_64, /* div */
};

void cmd_reboot(BaseSequentialStream *chp, int argc, char *argv[]) {
  (void)argc;
  (void)argv;

  chprintf(chp, "Rebooting...\r\n");
  chThdSleepMilliseconds(50);
  iwdgStart(&IWDGD, &watchdogConfig);
  while(1);

  return;
}
