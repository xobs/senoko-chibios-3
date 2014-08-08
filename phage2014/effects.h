#ifndef __PHAGE_EFFECTS_H__
#define __PHAGE_EFFECTS_H__

enum pattern {
  patternCalm,
  patternTest,
  patternShoot,
  patternLarson,
  patternStrobe
};

void effectsStart(void *_fb, int _count);
void effectsSetPattern(enum pattern pattern);
enum pattern effectsGetPattern(void);

#define EFFECTS_REDRAW_MS 20

#endif /* __PHAGE_EFFECTS_H__ */
