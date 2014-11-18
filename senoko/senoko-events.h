#ifndef __SENOKO_EVENTS_H__
#define __SENOKO_EVENTS_H__

extern event_source_t power_button_pressed;
extern event_source_t power_button_released;
extern event_source_t ac_unplugged;
extern event_source_t ac_plugged;
extern event_source_t powered_off;
extern event_source_t powered_on;

void senokoEventsInit(void);

#endif /* __SENOKO_EVENTS_H__ */
