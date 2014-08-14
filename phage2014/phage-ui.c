#include "ch.h"
#include "hal.h"
#include "chprintf.h"
#include "phage-ui.h"
#include "phage-radio.h"
#include "ledDriver.h"
#include "effects.h"

#include "phage.h"

static THD_WORKING_AREA(waUiThread, 32);
static msg_t ui_thread(void *arg) {
  (void)arg;
  uint8_t rxbuf[3];
  enum pattern oldpat;

  chRegSetThreadName("ui polling thread");

  while (1) {
    if(palReadPad(GPIOA, PA11)) {
      //ledDriverPause();
      // data receive gets highest prio
      radioGetRxPayload(rxbuf);
      //      chprintf( stream, "%02x ", rxbuf[2] );
      if( rxbuf[2] == 'x' ) {
        palWritePad(GPIOA, PA2, PAL_HIGH);
        palWritePad(GPIOA, PA3, PAL_HIGH);
        oldpat = effectsGetPattern();
        effectsSetPattern(patternStrobe);
        chThdSleepMilliseconds(200);
        palWritePad(GPIOA, PA2, PAL_LOW);
        palWritePad(GPIOA, PA3, PAL_LOW);
        effectsSetPattern(oldpat);
      }
      //ledDriverResume();
    }
    else {
      // check everything else here
      if(!palReadPad(GPIOA, PA13)) {
        //ledDriverPause();
        chprintf(stream, "x");
        radioSend('x');
        chThdSleepMilliseconds(50);  // an extra sleep here to prevent spamming
        //ledDriverResume();
      }
    }

    chThdSleepMilliseconds(30);
  }
  return MSG_OK;
}

void phageUiInit(void) {

  chThdCreateStatic(waUiThread, sizeof(waUiThread),
                    NORMALPRIO-10, ui_thread, NULL);
}
