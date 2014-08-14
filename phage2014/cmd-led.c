#include "ch.h"
#include "hal.h"

#include <stdlib.h>
#include "chprintf.h"

#include "phage.h"
#include "ledDriver.h"

static uint32_t addr = 0;
void cmd_led(BaseSequentialStream *chp, int argc, char *argv[])
{
  uint8_t dat[11];

  (void)argv;

  chprintf(chp, "argc: %d\r\n", argc);
  if (argc == 1) {
    chprintf(chp, "Pausing LED...\r\n");
    ledDriverPause();
  } else { 
    chprintf(chp, "Resuming LED...\r\n");
    ledDriverResume();
  }

  return;
}
