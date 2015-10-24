#ifndef __SENOKO_WDT_H__
#define __SENOKO_WDT_H__

/*
 * @brief   Maximum number of ms before the watchdog reboots Senoko
 */
#define SENOKO_WATCHDOG_MS          1000

/*
 * @brief   Frequency of the watchdog thread.
 *          Must be less than SENOKO_WATCHDOG_MS.
 */
#define SENOKO_WATCHDOG_THREAD_MS   500

void senokoWatchdogInit(void);
int senokoWatchdogEnabled(void);
int senokoWatchdogTimeToReset(void);
void senokoWatchdogEnable(void);
void senokoWatchdogDisable(void);
void senokoWatchdogSet(int new_seconds);
void senokoWatchdogLedDisable(void);
void senokoWatchdogLedEnable(void);

#endif /* __SENOKO_WDT_H__ */
