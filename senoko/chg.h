#ifndef __SENOKO_CHG_H__
#define __SENOKO_CHG_H__

int chgSet(uint16_t current, uint16_t voltage, uint16_t input);
int chgRefresh(uint16_t *current, uint16_t *voltage, uint16_t *input);
int chgGetManuf(uint16_t *word);
int chgGetDevice(uint16_t *word);
void chgInit(void);

#endif /* __SENOKO_CHG_H__ */
