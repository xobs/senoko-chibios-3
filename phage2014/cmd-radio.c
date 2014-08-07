#include "ch.h"
#include "hal.h"

#include <stdlib.h>
#include "chprintf.h"

#include "phage.h"
#include "phage-radio.h"

static uint32_t addr = 0;
void cmd_radio(BaseSequentialStream *chp, int argc, char *argv[])
{
  uint8_t dat[11];

  (void)argv;

  chprintf(chp, "argc: %d\r\n", argc);
  if (argc == 1) {
    chprintf(chp, "Sending broadcast message...\r\n");
//    radioSetChannel(addr);
    radioSend('x');
  }

  chprintf(chp, "Status value: ");
  radioGetStatus(dat);
  chprintf(chp, "%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\r\n",
      dat[0], dat[1], dat[2], dat[3], dat[4], dat[5], dat[6], dat[7], dat[8], dat[9], dat[10]);
  chprintf(chp, "\r\n");

  chprintf(chp, "TX address: 0x%08x\r\n", radioGetAddress());

  return;
}
