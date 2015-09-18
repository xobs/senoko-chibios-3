#include "hal.h"
#include "senoko.h"
#include "senoko-shell.h"

void uartOff(void) {  
  // Tristate the UART pins.
  palSetPadMode(GPIOA,  9, PAL_MODE_INPUT);
  palSetPadMode(GPIOA, 10, PAL_MODE_INPUT);
  
  return;
}

void uartOn(void) {
  senokoShellRestart();
  
  return;
}
