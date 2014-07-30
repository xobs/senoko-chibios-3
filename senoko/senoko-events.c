#include "ch.h"
#include "hal.h"
#include "ext.h"

#include "senoko-events.h"

#define BUTTON_DEBOUNCE_MS 10

event_source_t power_button_pressed;
event_source_t power_button_released;
event_source_t ac_plugged;
event_source_t ac_unplugged;

enum gpio_pin_names {
  pin_power,
  pin_acok,
  __pin_last,
};

static uint32_t gpio_states[__pin_last];

static void refresh_gpios(uint32_t *states) {
  states[pin_power] = palReadPad(GPIOB, PB14);
  states[pin_acok] = palReadPad(GPIOA, PA8);
}

static void debounce_button(void *arg) {

  (void)arg;
  uint32_t new_states[__pin_last];

  chSysLockFromISR();

  refresh_gpios(new_states);

  /* Power button */
  if (new_states[pin_power] != gpio_states[pin_power]) {
    if (new_states[pin_power])
      chEvtBroadcastI(&power_button_released);
    else
      chEvtBroadcastI(&power_button_pressed);
    gpio_states[pin_power] = new_states[pin_power];
  }

  /* AC_OK */
  if (new_states[pin_acok] != gpio_states[pin_acok]) {
    if (new_states[pin_acok])
      chEvtBroadcastI(&ac_plugged);
    else
      chEvtBroadcastI(&ac_unplugged);
    gpio_states[pin_acok] = new_states[pin_acok];
  }

  chSysUnlockFromISR();
}

static void gpio_callback(EXTDriver *extp, expchannel_t channel) {
  (void)extp;
  (void)channel;

  static virtual_timer_t debounce_vt;

  chSysLockFromISR();
  chVTResetI(&debounce_vt);
  chVTSetI(&debounce_vt, MS2ST(BUTTON_DEBOUNCE_MS), debounce_button, NULL);
  chSysUnlockFromISR();
}

static const EXTConfig ext_config ={
  {
    {EXT_CH_MODE_DISABLED, NULL},         /* Px0  */
    {EXT_CH_MODE_DISABLED, NULL},         /* Px1  */
    {EXT_CH_MODE_DISABLED, NULL},         /* Px2  */
    {EXT_CH_MODE_DISABLED, NULL},         /* Px3  */
    {EXT_CH_MODE_DISABLED, NULL},         /* Px4  */
    {EXT_CH_MODE_DISABLED, NULL},         /* Px5  */
    {EXT_CH_MODE_DISABLED, NULL},         /* Px6  */
    {EXT_CH_MODE_DISABLED, NULL},         /* Px7  */
    {EXT_CH_MODE_BOTH_EDGES               /* Px8  */
        | EXT_CH_MODE_AUTOSTART
        | EXT_MODE_GPIOA, gpio_callback},
    {EXT_CH_MODE_DISABLED, NULL},         /* Px9  */
    {EXT_CH_MODE_DISABLED, NULL},         /* Px10 */
    {EXT_CH_MODE_DISABLED, NULL},         /* Px11 */
    {EXT_CH_MODE_DISABLED, NULL},         /* Px12 */
    {EXT_CH_MODE_DISABLED, NULL},         /* Px13 */
    {EXT_CH_MODE_BOTH_EDGES               /* Px14 */
        | EXT_CH_MODE_AUTOSTART
        | EXT_MODE_GPIOB, gpio_callback},
    {EXT_CH_MODE_DISABLED, NULL}          /* Px15 */
  }
};

void senokoEventsInit(void) {

  chEvtObjectInit(&power_button_pressed);
  chEvtObjectInit(&power_button_released);
  chEvtObjectInit(&ac_plugged);
  chEvtObjectInit(&ac_unplugged);

  refresh_gpios(gpio_states);

  /* Begin listening to GPIOs (e.g. buttons).*/
  extStart(&EXTD1, &ext_config);
}
