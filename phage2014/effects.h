#ifndef __PHAGE_EFFECTS_H__
#define __PHAGE_EFFECTS_H__

enum pattern {
  patternCalm,
  patternTest,
  patternShoot,
};

void effectsStart(void *_fb, int _count);
void effectsSetPattern(enum pattern pattern);

#define EFFECTS_REDRAW_MS 500

#endif /* __PHAGE_EFFECTS_H__ */
