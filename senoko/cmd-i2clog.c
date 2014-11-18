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

#ifdef I2C_LOGGING
#define isprint(x) ((x < 0x20) && (x > 0x70))
int print_hex_offset(uint8_t *block, int count, int offset) {
    int byte;
    count += offset;
    block -= offset;
    for ( ; offset<count; offset+=16) {
        chprintf(stream, "%08x ", offset);

        for (byte=0; byte<16; byte++) {
            if (byte == 8)
                chprintf(stream, " ");
            if (offset+byte < count)
                chprintf(stream, " %02x", block[offset+byte]&0xff);
            else
                chprintf(stream, "   ");
        }

        chprintf(stream, "  |");
        for (byte=0; byte<16 && byte+offset<count; byte++)
            chprintf(stream, "%c", isprint(block[offset+byte]) ?
                                    block[offset+byte] :
                                    '.');
        chprintf(stream, "|\r\n");
    }
    return 0;
}

int print_hex(uint8_t *block, int count) {
    return print_hex_offset(block, count, 0);
}

static const char *i2ctype(uint8_t type) {
  switch (type) {
    case I2C_ENTRY_TYPE_NONE:  return "";
    case I2C_ENTRY_TYPE_READ:  return "recv";
    case I2C_ENTRY_TYPE_WRITE:  return "send";
    case I2C_ENTRY_TYPE_START:  return "start";
    default: return "unkn";
  }
}

void cmd_i2clog(BaseSequentialStream *chp, int argc, char *argv[]) {
  (void)argc;
  (void)argv;
  uint32_t idx;

  chprintf(chp, "I2C log head: %d/%d\r\n", i2clog.head, I2C_LOG_ENTRIES);

  for (idx = 0; idx < I2C_LOG_ENTRIES; idx++) {
    if (!i2clog.entries[idx].type)
      continue;
    chprintf(chp, "%6s %03d %5s %d\r\n", (idx == i2clog.head) ? " =>" : "",
                  idx,
		  i2ctype(i2clog.entries[idx].type),
		  i2clog.entries[idx].size);
    if (i2clog.entries[idx].type == I2C_ENTRY_TYPE_READ)
      print_hex(i2clog.entries[idx].data, i2clog.entries[idx].size);
  }

  memset(&i2clog, 0, sizeof(i2clog));
}
#else /* ! I2C_LOGGING */

void cmd_i2clog(BaseSequentialStream *chp, int argc, char *argv[]) {
  (void)argc;
  (void)argv;

  chprintf(chp, "I2C_LOGGING not enabled\r\n");
}

#endif /* I2C_LOGGING */
