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

#include "radio.h"
#include "phage2014.h"

void cmd_threads(BaseSequentialStream *chp, int argc, char *argv[]);
void cmd_mem(BaseSequentialStream *chp, int argc, char *argv[]);

static const ShellCommand shellCommands[] = {
  {"mem", cmd_mem},
  {"threads", cmd_threads},
  {"radio", cmdRadio},
  {NULL, NULL}
};

static const ShellConfig shellConfig = {
  stream,
  shellCommands
};

static thread_t *shellTp = NULL;
static THD_WORKING_AREA(waShellThread, 2048);

int shellTerminated(void)
{
  if (!shellTp)
    return TRUE;
  if (chThdTerminatedX(shellTp)) {
    /* Recovers memory of the previous shell. */
    chThdRelease(shellTp);
    return TRUE;
  }
  return FALSE;
}

void shellRestart(void)
{
  shellTp = shellCreateStatic(&shellConfig, waShellThread,
                              sizeof(waShellThread), NORMALPRIO);
}
