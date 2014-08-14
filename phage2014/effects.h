#ifndef __PHAGE_EFFECTS_H__
#define __PHAGE_EFFECTS_H__

enum pattern {
  patternCalm = 0,
  patternStrobe,
  patternRaindrop,
  patternRainbowdrop,
  patternWaveRainbow,
  patternTest,
  patternDirectedRainbow,
  patternLast
};

void effectsStart(void *_fb, int _count);
void effectsDraw(void);
void effectsSetPattern(enum pattern pattern);
enum pattern effectsGetPattern(void);
void bump(uint32_t amount);
void setShift(uint8_t s);
uint8_t getShift(void);
void effectsNextPattern(void);
void effectsPrevPattern(void);

#define EFFECTS_REDRAW_MS 35

#endif /* __PHAGE_EFFECTS_H__ */
