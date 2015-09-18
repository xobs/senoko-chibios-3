#ifndef __SENOKO_SLAVE_H__
#define __SENOKO_SLAVE_H__

struct i2c_registers {
  uint8_t signature;        /* 0x00 */
  uint8_t version_major;    /* 0x01 */
  uint8_t version_minor;    /* 0x02 */
  uint8_t features;         /* 0x03 */
  uint8_t uptime[4];        /* 0x04 - 0x07 */
  uint8_t irq_enable;       /* 0x08 */
  uint8_t irq_status;       /* 0x09 */
  uint8_t padding0[5];      /* 0x0a - 0x0e */
  uint8_t power;            /* 0x0f */

  /* -- GPIO block -- */
  uint8_t gpio_dir_a;       /* 0x10 */
  uint8_t gpio_dir_b;       /* 0x11 */
  uint8_t gpio_val_a;       /* 0x12 */
  uint8_t gpio_val_b;       /* 0x13 */
  uint8_t gpio_irq_rise_a;  /* 0x14 */
  uint8_t gpio_irq_rise_b;  /* 0x15 */
  uint8_t gpio_irq_fall_a;  /* 0x16 */
  uint8_t gpio_irq_fall_b;  /* 0x17 */
  uint8_t gpio_irq_stat_a;  /* 0x18 */
  uint8_t gpio_irq_stat_b;  /* 0x19 */
  uint8_t gpio_pull_ena_a;  /* 0x1a */
  uint8_t gpio_pull_ena_b;  /* 0x1b */
  uint8_t gpio_pull_dir_a;  /* 0x1c */
  uint8_t gpio_pull_dir_b;  /* 0x1d */
  uint8_t uart;	            /* 0x1e */
  uint8_t padding1;   	    /* 0x1f */

  /* -- RTC block -- */
  uint8_t seconds[4];       /* 0x20 - 0x23 */
  uint8_t alarm_seconds[4]; /* 0x24 - 0x27 */
  uint8_t wdt_seconds;      /* 0x28 */
};

#define REG_FEATURES 0x03
#define REG_FEATURES_BATTERY (1 << 0)
#define REG_FEATURES_GPIO (1 << 1)

#define REG_IRQ_ENABLE 0x08
#define REG_IRQ_STATUS 0x09
#define REG_IRQ_GPIO_MASK         (1 << 0)
#define REG_IRQ_KEYPAD_MASK       (1 << 1)
#define REG_IRQ_POWER_MASK        (1 << 2)
#define REG_IRQ_ALARM_MASK        (1 << 3)

#define REG_POWER 0x0f
#define REG_POWER_STATE_MASK      (3 << 0)
#define REG_POWER_STATE_ON        (0 << 0)
#define REG_POWER_STATE_OFF       (1 << 0)
#define REG_POWER_STATE_REBOOT    (2 << 0)
#define REG_POWER_WDT_MASK        (1 << 2)
#define REG_POWER_WDT_DISABLE     (0 << 2)
#define REG_POWER_WDT_ENABLE      (1 << 2)
#define REG_POWER_WDT_STATE       (1 << 2)
#define REG_POWER_AC_STATUS_MASK  (1 << 3)
#define REG_POWER_AC_STATUS_SHIFT (3)
#define REG_POWER_PB_STATUS_MASK  (1 << 4)
#define REG_POWER_PB_STATUS_SHIFT (4)
#define REG_POWER_KEY_MASK        (3 << 6)
#define REG_POWER_KEY_READ        (1 << 6)
#define REG_POWER_KEY_WRITE       (2 << 6)

#define REG_UART 0x1e
#define REG_UART_STATE_MASK       (1 << 0)
#define REG_UART_STATE_ON         (0 << 0)
#define REG_UART_STATE_OFF        (1 << 0)

#define REG_WATCHDOG_SECONDS 0x28

extern struct i2c_registers registers;

void senokoSlaveDispatch(void *bfr, uint32_t size);
void senokoSlavePrepTransaction(void);
void senokoSlaveInit(void);

#endif /* __SENOKO_SLAVE_H__ */
