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
#include "shell.h"
#include "chprintf.h"
#include "senoko.h"

/* Global stream variable, lets modules use chprintf().*/
void *stream;

void cmd_chg(BaseSequentialStream *chp, int argc, char *argv[]);
void cmd_date(BaseSequentialStream *chp, int argc, char *argv[]);
void cmd_gg(BaseSequentialStream *chp, int argc, char *argv[]);
void cmd_gpio(BaseSequentialStream *chp, int argc, char *argv[]);
void cmd_i2clog(BaseSequentialStream *chp, int argc, char *argv[]);
void cmd_leds(BaseSequentialStream *chp, int argc, char *argv[]);
void cmd_mem(BaseSequentialStream *chp, int argc, char *argv[]);
void cmd_power(BaseSequentialStream *chp, int argc, char *argv[]);
void cmd_stats(BaseSequentialStream *chp, int argc, char *argv[]);
void cmd_reboot(BaseSequentialStream *chp, int argc, char *argv[]);
void cmd_threads(BaseSequentialStream *chp, int argc, char *argv[]);
void cmd_uptime(BaseSequentialStream *chp, int argc, char *argv[]);
void cmd_version(BaseSequentialStream *chp, int argc, char *argv[]);

static const ShellCommand shellCommands[] = {
  {"chg", cmd_chg},
  {"date", cmd_date},
  {"gg", cmd_gg},
  {"gpio", cmd_gpio},
  {"i2clog", cmd_i2clog},
  {"leds", cmd_leds},
  {"mem", cmd_mem},
  {"power", cmd_power},
  {"stats", cmd_stats},
  {"reboot", cmd_reboot},
  {"threads", cmd_threads},
  {"uptime", cmd_uptime},
  {"version", cmd_version},
  {NULL, NULL}
};

static const ShellConfig shellConfig = {
  stream_driver,
  shellCommands
};

static const SerialConfig serialConfig = {
  115200,
  0,
  0,
  0,
};

static thread_t *shell_tp = NULL;
static THD_WORKING_AREA(waShellThread, 1024);

void senokoShellInit(void) {
  sdStart(serialDriver, &serialConfig);
  stream = stream_driver;

  shellInit();
}

void senokoShellRestart(void) {
  /* Recovers memory of the previous shell. */
  if (shell_tp && chThdTerminatedX(shell_tp))
    chThdRelease(shell_tp);
  shell_tp = shellCreateStatic(&shellConfig, waShellThread,
                              sizeof(waShellThread), NORMALPRIO - 5);
}
