#include "ch.h"
#include "hal.h"

#include "chg.h"
#include "board-type.h"

static enum board_type board_type = unknown;

static void get_board_type(void) {
  if (chgPresent())
    board_type = senoko_full;
  else
    board_type = senoko_half;
}

enum board_type boardType(void) {
  if (board_type == unknown)
    get_board_type();
  return board_type;
}
