#ifndef __SENOKO_CHG_H__
#define __SENOKO_CHG_H__

int chgSet(uint16_t current, uint16_t voltage);
int chgSetInput(uint16_t current);
int chgSetAll(uint16_t current, uint16_t voltage, uint16_t input);
int chgRefresh(uint16_t *current, uint16_t *voltage, uint16_t *input);
int chgGetManuf(uint16_t *word);
int chgGetDevice(uint16_t *word);
int chgPresent(void);
void chgPause(void);
void chgResume(void);
bool chgPaused(void);
void chgInit(void);

#endif /* __SENOKO_CHG_H__ */
