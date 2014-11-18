#ifndef __SENOKO_POWER_H__
#define __SENOKO_POWER_H__

void powerOffI(void);
void powerOnI(void);
int powerIsOn(void);
int powerIsOff(void);
void powerToggle(void);
void powerInit(void);
void powerReboot(void);
void powerRebootI(void);

#endif /* __SENOKO_POWER_H__ */
