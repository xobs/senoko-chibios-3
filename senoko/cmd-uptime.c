/*
    ChibiOS - Copyright (C) 2006-2014 Giovanni Di Sirio

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
#include "senoko.h"
#include "chprintf.h"

void cmd_uptime(BaseSequentialStream *chp, int argc, char *argv[]) {

  (void)argv;
  if (argc > 0) {
    chprintf(chp, "Usage: uptime\r\n");
    return;
  }
  uint32_t uptime_msec = senoko_uptime;
  uint32_t uptime_sec = uptime_msec / 1000;
  uptime_msec -= (uptime_sec * 1000);
  chprintf(chp, "%lu.%03lu seconds\r\n", uptime_sec, uptime_msec);
}
