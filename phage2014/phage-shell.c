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

#include "phage.h"

/* Forward declarations of available shell commands.*/
void cmd_accel(BaseSequentialStream *chp, int argc, char *argv[]);
void cmd_adc(BaseSequentialStream *chp, int argc, char *argv[]);
void cmd_mem(BaseSequentialStream *chp, int argc, char *argv[]);
void cmd_radio(BaseSequentialStream *chp, int argc, char *argv[]);
void cmd_reboot(BaseSequentialStream *chp, int argc, char *argv[]);
void cmd_threads(BaseSequentialStream *chp, int argc, char *argv[]);
void cmd_led(BaseSequentialStream *chp, int argc, char *argv[]);

/* Global stream variable, lets modules use chprintf().*/
void *stream;

static const ShellCommand shell_commands[] = {
  {"accel", cmd_accel},
  {"adc", cmd_adc},
  {"led", cmd_led},
  {"mem", cmd_mem},
  {"radio", cmd_radio},
  {"reboot", cmd_reboot},
  {"threads", cmd_threads},
  {NULL, NULL}
};

static const ShellConfig shellConfig = {
  stream_driver,
  shell_commands
};

static const SerialConfig serialConfig = {
  115200,
  0,
  0,
  0,
};

static thread_t *shell_tp = NULL;
static THD_WORKING_AREA(waShellThread, 2048);

void phageShellInit(void) {
  sdStart(serialDriver, &serialConfig);
  stream = stream_driver;

  shellInit();
}

void phageShellRestart(void) {
  /* Recovers memory of the previous shell. */
  if (shell_tp && chThdTerminatedX(shell_tp))
    chThdRelease(shell_tp);
  shell_tp = shellCreateStatic(&shellConfig, waShellThread,
                              sizeof(waShellThread), LOWPRIO);
}
