#ifndef __PHAGE_WDT_H__
#define __PHAGE_WDT_H__

/*
 * @brief   Maximum number of ms before the watchdog reboots Phage.
 */
#define PHAGE_WATCHDOG_MS          1000

/*
 * @brief   Frequency of the watchdog thread.
 *          Must be less than PHAGE_WATCHDOG_MS.
 */
#define PHAGE_WATCHDOG_THREAD_MS   500

void phageWatchdogInit(void);

#endif /* __PHAGE_WDT_H__ */
