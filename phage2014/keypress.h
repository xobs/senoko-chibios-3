#ifndef __KEYPRESS_H__
#define __KEYPRESS_H__

#define KEY_DEBOUNCE_MS 20

enum keychar {
  KEY_NONE,
  KEY_0,
  KEY_1,
  KEY_2,
  KEY_3,
};

enum keychar keyIsPressed(void);
void keyISR(EXTDriver *extp, expchannel_t channel);
void keyInit(void);

#endif /* __KEYPRESS_H__ */
