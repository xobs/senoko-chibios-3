#include "ch.h"
#include "hal.h"

#include "ac.h"

int acPlugged(void)
{
  return !!palReadPad(GPIOA, PA8);
}

int acRemoved(void)
{
  return !palReadPad(GPIOA, PA8);
}
