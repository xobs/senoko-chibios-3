#ifndef __KEYPRESS_H__
#define __KEYPRESS_H__

#define KEY_DEBOUNCE_MS 20

void keyISR(EXTDriver *extp, expchannel_t channel);
void keyInit(void);
void keyRegisterListener(event_listener_t *el);

enum keychar {
  KEY_0,
  KEY_1,
  KEY_2,
  KEY_3,
};

#endif /* __KEYPRESS_H__ */
