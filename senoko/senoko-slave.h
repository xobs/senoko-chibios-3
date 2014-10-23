#ifndef __SENOKO_SLAVE_H__
#define __SENOKO_SLAVE_H__

struct i2c_registers {
  uint8_t signature;        /* 0x00 */
  uint8_t version_major;    /* 0x01 */
  uint8_t version_minor;    /* 0x02 */
  uint8_t uptime[4];        /* 0x03 - 0x06 */
  uint8_t power;            /* 0x07 */
  uint8_t wdt_seconds;      /* 0x08 */
  uint8_t ier;              /* 0x09 */
  uint8_t irq_status;       /* 0x0a */
  uint8_t key_status;       /* 0x0b */
  uint8_t padding0[4];      /* 0x0c - 0x0f */

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
  uint8_t padding1[6];      /* 0x1a - 0x1f */

  /* -- RTC block -- */
  uint8_t seconds[4];       /* 0x20 - 0x23 */
};

extern struct i2c_registers registers;

void senokoSlaveDispatch(void *bfr, uint32_t size);
void senokoSlaveInit(void);

#endif /* __SENOKO_SLAVE_H__ */
