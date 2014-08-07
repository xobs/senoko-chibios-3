#include "ch.h"
#include "hal.h"
#include "ext.h"

#include "phage-events.h"

#define BUTTON_DEBOUNCE_MS 10

event_source_t power_button_pressed;
event_source_t power_button_released;
event_source_t accel_int1;
event_source_t accel_int2;
event_source_t key_left_pressed;
event_source_t key_right_pressed;
event_source_t key_up_pressed;
event_source_t key_down_pressed;
event_source_t key_left_released;
event_source_t key_right_released;
event_source_t key_up_released;
event_source_t key_down_released;
event_source_t radio_address_matched;
event_source_t radio_data_received;
event_source_t radio_carrier_detect;


enum gpio_pin_names {
  pin_power,
  pin_accel_int1,
  pin_accel_int2,
  pin_key_up,
  pin_key_down,
  pin_key_left,
  pin_key_right,
  pin_radio_address_matched,
  pin_radio_data_received,
  pin_radio_carrier_detect,
  __pin_last,
};

static uint32_t gpio_states[__pin_last];

static void refresh_gpios(uint32_t *states) {
  states[pin_power] = palReadPad(GPIOB, PB14);
  states[pin_accel_int1] = palReadPad(GPIOA, PA8);
  states[pin_accel_int2] = palReadPad(GPIOB, PB15);
  states[pin_key_up] = palReadPad(GPIOB, PB4);
  states[pin_key_down] = palReadPad(GPIOB, PB5);
  states[pin_key_left] = palReadPad(GPIOA, PA13);
  states[pin_key_right] = palReadPad(GPIOA, PA15);
  states[pin_radio_address_matched] = palReadPad(GPIOA, PA0);
  states[pin_radio_data_received] = palReadPad(GPIOA, PA11);
  states[pin_radio_carrier_detect] = palReadPad(GPIOA, PA12);
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

  /* Accelerometer Interrupt 1 */
  if (new_states[pin_accel_int1] != gpio_states[pin_accel_int1]) {
    if (new_states[pin_accel_int1])
      chEvtBroadcastI(&accel_int1);
    gpio_states[pin_accel_int1] = new_states[pin_accel_int1];
  }

  /* Accelerometer Interrupt 2 */
  if (new_states[pin_accel_int2] != gpio_states[pin_accel_int2]) {
    if (new_states[pin_accel_int2])
      chEvtBroadcastI(&accel_int2);
    gpio_states[pin_accel_int2] = new_states[pin_accel_int2];
  }

  /* Up button */
  if (new_states[pin_key_up] != gpio_states[pin_key_up]) {
    if (new_states[pin_key_up])
      chEvtBroadcastI(&key_up_released);
    else
      chEvtBroadcastI(&key_up_pressed);
    gpio_states[pin_key_up] = new_states[pin_key_up];
  }

  /* Down button */
  if (new_states[pin_key_down] != gpio_states[pin_key_down]) {
    /* Note inverted logic here (pin is wired inverted).*/
    if (new_states[pin_key_down])
      chEvtBroadcastI(&key_down_pressed);
    else
      chEvtBroadcastI(&key_down_released);
    gpio_states[pin_key_down] = new_states[pin_key_down];
  }

  /* Left button */
  if (new_states[pin_key_left] != gpio_states[pin_key_left]) {
    if (new_states[pin_key_left])
      chEvtBroadcastI(&key_left_released);
    else
      chEvtBroadcastI(&key_left_pressed);
    gpio_states[pin_key_left] = new_states[pin_key_left];
  }

  /* Right button */
  if (new_states[pin_key_right] != gpio_states[pin_key_right]) {
    if (new_states[pin_key_right])
      chEvtBroadcastI(&key_right_released);
    else
      chEvtBroadcastI(&key_right_pressed);
    gpio_states[pin_key_right] = new_states[pin_key_right];
  }

  /* Radio Address Matched */
  if (new_states[pin_radio_address_matched] != gpio_states[pin_radio_address_matched]) {
    if (new_states[pin_radio_address_matched])
      chEvtBroadcastI(&radio_address_matched);
    gpio_states[pin_radio_address_matched] = new_states[pin_radio_address_matched];
  }

  /* Radio Data Received */
  if (new_states[pin_radio_data_received] != gpio_states[pin_radio_data_received]) {
    if (new_states[pin_radio_data_received])
      chEvtBroadcastI(&radio_data_received);
    gpio_states[pin_radio_data_received] = new_states[pin_radio_data_received];
  }

  /* Radio Carrier Detect */
  if (new_states[pin_radio_carrier_detect] != gpio_states[pin_radio_carrier_detect]) {
    if (new_states[pin_radio_carrier_detect])
      chEvtBroadcastI(&radio_carrier_detect);
    gpio_states[pin_radio_carrier_detect] = new_states[pin_radio_carrier_detect];
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
    //    {EXT_CH_MODE_BOTH_EDGES               /* Px0  */
    //        | EXT_CH_MODE_AUTOSTART
    //        | EXT_MODE_GPIOA, gpio_callback},
    {EXT_CH_MODE_DISABLED, NULL},         /* Px0  */
    {EXT_CH_MODE_DISABLED, NULL},         /* Px1  */
    {EXT_CH_MODE_DISABLED, NULL},         /* Px2  */
    {EXT_CH_MODE_DISABLED, NULL},         /* Px3  */
    {EXT_CH_MODE_BOTH_EDGES               /* Px4  */
        | EXT_CH_MODE_AUTOSTART
        | EXT_MODE_GPIOB, gpio_callback},
    {EXT_CH_MODE_BOTH_EDGES               /* Px5  */
        | EXT_CH_MODE_AUTOSTART
        | EXT_MODE_GPIOB, gpio_callback},
    {EXT_CH_MODE_DISABLED, NULL},         /* Px6  */
    {EXT_CH_MODE_DISABLED, NULL},         /* Px7  */
    {EXT_CH_MODE_BOTH_EDGES               /* Px8  */
        | EXT_CH_MODE_AUTOSTART
        | EXT_MODE_GPIOA, gpio_callback},
    {EXT_CH_MODE_DISABLED, NULL},         /* Px9  */
    {EXT_CH_MODE_DISABLED, NULL},         /* Px10 */
    //    {EXT_CH_MODE_BOTH_EDGES               /* Px11 */
    //        | EXT_CH_MODE_AUTOSTART
    //        | EXT_MODE_GPIOA, gpio_callback},
    {EXT_CH_MODE_DISABLED, NULL},         /* Px11 */
    //    {EXT_CH_MODE_BOTH_EDGES               /* Px12 */
    //        | EXT_CH_MODE_AUTOSTART
    //        | EXT_MODE_GPIOA, gpio_callback},
    {EXT_CH_MODE_DISABLED, NULL},         /* Px12 */
    {EXT_CH_MODE_BOTH_EDGES               /* Px13 */
        | EXT_CH_MODE_AUTOSTART
        | EXT_MODE_GPIOA, gpio_callback},
    {EXT_CH_MODE_BOTH_EDGES               /* Px14 */
        | EXT_CH_MODE_AUTOSTART
        | EXT_MODE_GPIOB, gpio_callback},
    {EXT_CH_MODE_BOTH_EDGES               /* Px15 */
        | EXT_CH_MODE_AUTOSTART
        | EXT_MODE_GPIOA, gpio_callback},
  }
};

void phageEventsInit(void) {

  chEvtObjectInit(&power_button_pressed);
  chEvtObjectInit(&power_button_released);
  chEvtObjectInit(&accel_int1);
  chEvtObjectInit(&accel_int2);
  chEvtObjectInit(&key_up_pressed);
  chEvtObjectInit(&key_down_pressed);
  chEvtObjectInit(&key_left_pressed);
  chEvtObjectInit(&key_right_pressed);
  chEvtObjectInit(&key_up_released);
  chEvtObjectInit(&key_down_released);
  chEvtObjectInit(&key_left_released);
  chEvtObjectInit(&key_right_released);

  //  chEvtObjectInit(&radio_data_received);

  refresh_gpios(gpio_states);

  /* Begin listening to GPIOs (e.g. buttons).*/
  extStart(&EXTD1, &ext_config);
}
