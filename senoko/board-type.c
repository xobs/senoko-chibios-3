#include "ch.h"
#include "hal.h"

#include "board-type.h"
#include "chg.h"
#include "senoko-i2c.h"

static enum board_type board_type = senoko_unknown;

enum board_type boardType(void)
{
  if (board_type == senoko_unknown)
    boardTypeInit();
  return board_type;
}

void boardTypeInit(void)
{
  board_type = senoko_passthru;
  if (chgPresent())
    board_type = senoko_full;
}

