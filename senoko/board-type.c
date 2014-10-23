#include "ch.h"
#include "hal.h"

#include "chg.h"
#include "board-type.h"
#include "senoko-i2c.h"

static enum board_type board_type = unknown;

static void get_board_type(void) {
  senokoI2cAcquireBus();
  if (chgPresent())
    board_type = senoko_full;
  else
    board_type = senoko_half;
  senokoI2cReleaseBus();
}

enum board_type boardType(void) {
  if (board_type == unknown)
    get_board_type();
  return board_type;
}
