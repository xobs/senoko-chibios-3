#ifndef __PHAGE_EVENTS_H__
#define __PHAGE_EVENTS_H__

extern event_source_t power_button_pressed;
extern event_source_t power_button_released;

extern event_source_t accel_int1;
extern event_source_t accel_int2;

extern event_source_t key_left_pressed;
extern event_source_t key_right_pressed;
extern event_source_t key_up_pressed;
extern event_source_t key_down_pressed;
extern event_source_t key_left_released;
extern event_source_t key_right_released;
extern event_source_t key_up_released;
extern event_source_t key_down_released;

extern event_source_t radio_address_matched;
extern event_source_t radio_data_received;
extern event_source_t radio_carrier_detect;

void phageEventsInit(void);

#endif /* __PHAGE_EVENTS_H__ */
