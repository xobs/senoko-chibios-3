#include "ch.h"
#include "hal.h"
#include "i2c.h"

#include "senoko.h"
#include "senoko-i2c.h"
#include "gg.h"
#include "bionic.h"

#define GG_ADDR 0xb

static const struct cell_cfg {
  uint16_t pov_threshold;
  uint16_t pov_recovery;
  uint16_t puv_threshold;
  uint16_t puv_recovery;
  uint16_t sov_threshold;
  uint16_t charging_voltage;
  uint16_t depleted_voltage;
  uint16_t depleted_recovery;
  uint16_t design_voltage;
  uint16_t flash_update_ok_voltage;
  uint16_t shutdown_voltage;
  uint16_t term_voltage;
} cell_cfgs[] = {
  [2] = {
    .pov_threshold = 8700,
    .pov_recovery = 8400,
    .puv_threshold = 5400,
    .puv_recovery = 5700,
    .sov_threshold = 9000,
    .charging_voltage = 8400,
    .depleted_voltage = 5000,
    .depleted_recovery = 5500,
    .design_voltage = 7200,
    .flash_update_ok_voltage = 6000,
    .shutdown_voltage = 5000,
    .term_voltage = 6000,
  },
  [3] = {
    .pov_threshold = 13000,
    .pov_recovery = 12600,
    .puv_threshold = 8100,
    .puv_recovery = 8500,
    .sov_threshold = 13500,
    .charging_voltage = 12600,
    .depleted_voltage = 8000,
    .depleted_recovery = 8500,
    .design_voltage = 10800,
    .flash_update_ok_voltage = 7500,
    .shutdown_voltage = 7000,
    .term_voltage = 9000,
  },
  [4] = {
    .pov_threshold = 17500,
    .pov_recovery = 16000,
    .puv_threshold = 11000,
    .puv_recovery = 12000,
    .sov_threshold = 18000,
    .charging_voltage = 16800,
    .depleted_voltage = 11000,
    .depleted_recovery = 11500,
    .design_voltage = 14400,
    .flash_update_ok_voltage = 7500,
    .shutdown_voltage = 7000,
    .term_voltage = 12000,
  },
};

static const uint8_t subclass_size[] = {
  [0] = 22,
  [1] = 25,
  [2] = 10,
  [3] = 1,

  [16] = 12,
  [17] = 6,
  [18] = 9,
  [19] = 3,
  [20] = 5,
  [21] = 3,

  [32] = 6,
  [33] = 8,
  [34] = 10,
  [35] = 8,
  [36] = 13,
  [37] = 2,
  [38] = 22,

  [48] = 51,
  [49] = 14,

  [56] = 10,

  [58] = 32,
  [59] = 30,
  [60] = 4,

  [64] = 10,

  [67] = 20,
  [68] = 20,

  [80] = 68,
  [81] = 8,
  [82] = 27,
  [88] = 32,
  [89] = 32,
  [90] = 32,
  [91] = 32,
  [92] = 32,
  [93] = 32,
  [94] = 32,
  [95] = 32,
  [96] = 30,
  [97] = 9,

  [104] = 21,
  [105] = 19,
  [106] = 24,
  [107] = 3,
};

static const struct gg_string {
  uint8_t size;
  uint8_t reg;
} gg_strings[] = {
  {
    .size = 12,
    .reg  = 0x20,
  },
  {
    .size = 8,
    .reg  = 0x21,
  },
  {
    .size = 5,
    .reg  = 0x22,
  },
};

int ggInit(void) {
  /* Pull GG_SYSPRES low to bring gas gauge out of reset.*/
  palWritePad(GPIOA, PA11, 0);
  return 0;
}

static int gg_getmfgr(uint16_t reg, void *data, int size)
{
  msg_t status;
  uint8_t bfr[3];
  uint8_t *data8 = data;
  uint8_t tmp;

  bfr[0] = 0;
  bfr[1] = reg;
  bfr[2] = reg >> 8;

  status = senokoI2cMasterTransmitTimeout(GG_ADDR,
                                          bfr, 3,
                                          NULL, 0);
  if (data && size)
    status = senokoI2cMasterTransmitTimeout(GG_ADDR,
                                            bfr, 1,
                                            data, size);
  if (status != MSG_OK)
    return senokoI2cErrors();

  if (data && size == 2) {
    tmp = data8[0];
    data8[0] = data8[1];
    data8[1] = tmp;
  }
  return 0;
}

static int gg_getblock(uint8_t reg, void *data, int size)
{
  msg_t status;

  status = senokoI2cMasterTransmitTimeout(GG_ADDR,
                                          &reg, sizeof(reg),
                                          data, size);

  if (status != MSG_OK)
    return senokoI2cErrors();
  return 0;
}

static int gg_getflash(uint8_t subclass, uint8_t offset, void *data, int size) {
  msg_t status;
  uint8_t bfr[3];
  uint8_t *cdata = data;
  uint8_t reg;
  int ptr;

  for (ptr = 0; ptr < size; ptr++)
    cdata[ptr] = 0;

  bfr[0] = 0x77; /* SetSubclassID register */
  bfr[1] = subclass;
  bfr[2] = subclass >> 8;


  status = senokoI2cMasterTransmitTimeout(GG_ADDR,
                                          bfr, sizeof(bfr),
                                          NULL, 0);
  if (status != MSG_OK) {
    status = 1;
    goto err;
  }

  reg = (offset / 32) + 0x78;
  while (size > 0) {
    /*
     * If we're starting at a non-divisible offset,
     * copy the data into an intermediate buffer first.
     */
    if (offset & 31) {
      uint8_t temp_buffer[33];
      int i = 1 + (offset & 31);
      status = senokoI2cMasterTransmitTimeout(GG_ADDR,
                                              &reg, sizeof(reg),
                                              temp_buffer, 33);
      while ((offset & 31) && (size > 0)) {
        *cdata++ = temp_buffer[i++];
        offset++;
        size--;
      }
    }
    else {
      uint8_t temp_buffer[33];
      int to_read;

      to_read = 33;
      if (size < 32)
        to_read = size + 1;

      status = senokoI2cMasterTransmitTimeout(GG_ADDR,
                                              &reg, sizeof(reg),
                                              temp_buffer, to_read);
      memcpy(cdata, temp_buffer + 1, to_read - 1);
      size  -= 32;
      cdata += 32;
    }
    if (status != MSG_OK) {
      status = 2;
      goto err;
    }
    reg++;
  }

  return 0;

err:
  return (status << 24) | senokoI2cErrors();
}

static int gg_setflash(uint8_t subclass, uint8_t offset, void *data, int size) {
  msg_t status;
  uint8_t bfr[3];
  int ret;
  int ptr;
  int start = (offset/32);
  int end = (32*((offset+size)/32) + 32*(!!(offset+size)))/32;
  uint8_t eeprom_cache[256];

  if ((offset + size > subclass_size[subclass]) || size <= 0)
    return -1;

  chThdSleepMilliseconds(50);
  ret = gg_getflash(subclass, 0, eeprom_cache, subclass_size[subclass]);
  if (ret < 0)
    return -2;

  memcpy(eeprom_cache + offset, data, size);

  chThdSleepMilliseconds(25);

  bfr[0] = 0x77; /* SetSubclassID register */
  bfr[1] = subclass;
  bfr[2] = subclass>>8;
  status = senokoI2cMasterTransmitTimeout(GG_ADDR,
                                          bfr, sizeof(bfr),
                                          NULL, 0);
  if (status < 0) {
    status = 3;
    goto err;
  }

  for (ptr = start; ptr < end; ptr++) {
    uint8_t temp_buffer[34];
    int write_size;

    write_size = 32;
    if ( (ptr + 1) * 32 > subclass_size[subclass])
      write_size = (subclass_size[subclass] & 31);

    /* Add an extra byte for the 'register' command */
    write_size++;

    /* Add an extra byte for the 'byte count' packet */
    write_size++;

    temp_buffer[0] = 0x78 + ptr;
    temp_buffer[1] = write_size - 2;
    memcpy(temp_buffer + 2, eeprom_cache + (32 * ptr), write_size - 2);

    status = senokoI2cMasterTransmitTimeout(GG_ADDR,
                                            temp_buffer, write_size,
                                            NULL, 0);
    if (status != MSG_OK) {
      status = 4;
      goto err;
    }

    /* Let the flash write out */
    chThdSleepMilliseconds(100);
  }

  return 0;

err:
  return (status << 24) | senokoI2cErrors();
}

static int gg_setflash_word(uint8_t subclass, uint8_t offset, uint16_t data) {
  uint16_t val;
  val = ((data >> 8) & 0xff) | ((data << 8) & 0xff00);
  return gg_setflash(subclass, offset, &val, 2);
}

static int gg_getflash_word(uint8_t subclass, uint8_t offset, uint16_t *data) {
  int ret;
  ret = gg_getflash(subclass, offset, data, 2);
  if (ret < 0)
    return ret;
  *data = ((*data >> 8) & 0xff) | ((*data << 8) & 0xff00);
  return 0;
}

static int gg_getword(uint8_t reg, void *word) {
  return gg_getblock(reg, word, 2);
}

static int gg_getbyte(uint8_t reg, void *byte) {
  return gg_getblock(reg, byte, 1);
}

static int gg_setblock(uint8_t reg, void *data, int size) {
  uint8_t bfr[size+1];
  msg_t status;

  bfr[0] = reg;
  if (data && size)
    memcpy(bfr+1, data, size);

  status = senokoI2cMasterTransmitTimeout(GG_ADDR,
                                          bfr, size+1,
                                          NULL, 0);

  if (status != MSG_OK)
    return senokoI2cErrors();
  return 0;
}

static int gg_setword(uint8_t reg, uint16_t word) {
  return gg_setblock(reg, &word, 2);
}

static int gg_getstring(uint8_t addr, uint8_t *data, int size) {
  int ret;
  int i;

  ret = gg_getblock(addr, data, size);
  if (ret < 0)
    return ret;

  size--;
  if (data[0] < size)
    size = data[0];
  for (i=0; i<size; i++)
    data[i] = data[i+1];
  data[i] = '\0';
  return size;
}

int ggSetManufacturer(uint8_t name[11]) {
  return gg_setflash(48, 26 + 1, name, 11);
}

int ggSetChemistry(uint8_t chem[4]) {
  return gg_setflash(48, 46 + 1, chem, 4);
}

int ggSetCellCount(int cells) {
  int ret;
  uint8_t cfg_a[2];

  if (cells < 2 || cells > 4)
    return -1;

  /* Set the number of cells */

  ret = gg_getflash(64, 0, cfg_a, sizeof(cfg_a));
  if (ret < 0)
    return ret;

  cfg_a[0] &= ~3;

  if (cells == 2)
    cfg_a[0] |= 1;
  else if (cells == 3)
    cfg_a[0] |= 2;
  else if (cells == 4)
    cfg_a[0] |= 3;
  else
    return -1;

  ret = gg_setflash(64, 0, cfg_a, sizeof(cfg_a));
  if (ret < 0)
    return ret;

  /* Set various over/undervoltage flags */
  
  ret = gg_setflash_word(0, 7, cell_cfgs[cells].pov_threshold);
  if (ret < 0)
    return ret;

  ret = gg_setflash_word(0, 10, cell_cfgs[cells].pov_recovery);
  if (ret < 0)
    return ret;

  ret = gg_setflash_word(0, 17, cell_cfgs[cells].puv_threshold);
  if (ret < 0)
    return ret;

  ret = gg_setflash_word(0, 20, cell_cfgs[cells].puv_recovery);
  if (ret < 0)
    return ret;

  ret = gg_setflash_word(16, 0, cell_cfgs[cells].sov_threshold);
  if (ret < 0)
    return ret;
  
  ret = gg_setflash_word(34, 2, cell_cfgs[cells].charging_voltage);
  if (ret < 0)
    return ret;
  
  ret = gg_setflash_word(38, 8, cell_cfgs[cells].depleted_voltage);
  if (ret < 0)
    return ret;
  
  ret = gg_setflash_word(38, 11, cell_cfgs[cells].depleted_recovery);
  if (ret < 0)
    return ret;
  
  ret = gg_setflash_word(48, 8, cell_cfgs[cells].design_voltage);
  if (ret < 0)
    return ret;
  
  ret = gg_setflash_word(68, 0, cell_cfgs[cells].flash_update_ok_voltage);
  if (ret < 0)
    return ret;
  
  ret = gg_setflash_word(68, 2, cell_cfgs[cells].shutdown_voltage);
  if (ret < 0)
    return ret;
  
  ret = gg_setflash_word(80, 45, cell_cfgs[cells].term_voltage);
  if (ret < 0)
    return ret;

  return 0;
}

int ggCellCount(uint8_t *cells) {
  int ret;
  uint8_t cfg_a[2];

  ret = gg_getflash(64, 0, cfg_a, sizeof(cfg_a));
  if (ret < 0)
    return ret;

  /* Cell count, in the current chip, happens to be cfg_a[1:0] + 1 */
  *cells = (cfg_a[0] & 0x3) + 1;

  return 0;
}

int ggManufacturer(uint8_t *manuf) {
  return gg_getstring(gg_strings[0].reg, manuf, gg_strings[0].size);
}

int ggPartName(uint8_t name[8]) {
  return gg_getstring(gg_strings[1].reg, name, gg_strings[1].size);
}

int ggChemistry(uint8_t *chem) {
  return gg_getstring(gg_strings[2].reg, chem, gg_strings[2].size);
}

int ggSerial(uint16_t *serial) {
  return gg_getword(0x1c, serial);
}

int ggPercent(uint8_t *capacity) {
  return gg_getbyte(0x0d, capacity);
}

int ggCellVoltage(int cell, uint16_t *voltage) {
  if (cell <= 0 || cell > 4)
    return -1;
  cell--;
  cell = 3-cell;
  return gg_getword(0x3c+cell, voltage);
}

int ggSetCapacity(int cells, uint16_t capacity) {
  int cell;
  int ret;

  if (cells < 2 || cells > 4)
    return 1;

  /* Set capacity of known cells */
  for (cell=0; cell < cells; cell++) {
    if (cell < cells)
      ret = gg_setflash_word(82, cell*2, capacity);
    else
      /* Set other capacities to 0 */
      ret = gg_setflash_word(82, cell*2, 0);
    if (ret < 0)
      return ret;
  }

  /* Set Qmax Pack */
  ret = gg_setflash_word(82, 8, capacity);
  if (ret < 0)
    return ret;

  /* Set the SBS value */
  ret = gg_setflash_word(48, 22, capacity);
  if (ret < 0)
    return ret;

  /* Set tracking support */
  uint8_t reg;
  reg = 0x03;
  ret = gg_setflash(82, 12, &reg, 1);
  if (ret < 0)
    return ret;

  return 0;
}

int ggMode(uint16_t *word) {
  return gg_getword(0x03, word);
}

int ggSetPrimary(void) {
  uint16_t reg;
  int ret;
  ret = ggMode(&reg);
  if (ret < 0)
    return ret;
  reg |= (1 << 1);
  return gg_setblock(0x03, &reg, 2);
}

int ggSetSecondary(void) {
  uint16_t reg;
  int ret;
  ret = ggMode(&reg);
  if (ret < 0)
    return ret;
  reg &= ~(1 << 1);
  return gg_setblock(0x03, &reg, 2);
}

int ggTemperature(int16_t *word) {
  int16_t *temp = word;
  int ret;
  ret = gg_getword(0x08, temp);
  if (ret < 0)
    return ret;
  *temp = *temp - 2730;
  return 0;
}

int ggTimeToFull(uint16_t *minutes) {
  return gg_getword(0x13, minutes);
}

int ggTimeToEmpty(uint16_t *minutes) {
  return gg_getword(0x12, minutes);
}

int ggVoltage(uint16_t *word) {
  return gg_getword(0x09, word);
}

int ggCurrent(int16_t *word) {
  return gg_getword(0x0a, word);
}

int ggChargingCurrent(int16_t *word) {
  return gg_getword(0x14, word);
}

int ggChargingVoltage(uint16_t *word) {
  return gg_getword(0x15, word);
}

int ggFullCapacity(uint16_t *word) {
  return gg_getword(0x10, word);
}

int ggDesignCapacity(uint16_t *word) {
  return gg_getword(0x18, word);
}

int ggAverageCurrent(int16_t *word) {
  int ret;
  ret = gg_getword(0xb, word);
  if (ret < 0)
    return ret;
  return 0;
}

int ggStatus(uint16_t *word) {
  return gg_getword(0x16, word);
}

int ggFirmwareVersion(uint16_t *word) {
  return gg_getmfgr(0x0001, word, 2);
}

int ggState(uint16_t *word) {
  int ret;
  ret = gg_getmfgr(0x0006, word, 2);
  if (ret < 0)
    return ret;
  return 0;
}

int ggSetLeds(int state) {
  switch(state) {
  case 1:
    return gg_getmfgr(0x0032, NULL, 0);
  case -1:
    return gg_getmfgr(0x0033, NULL, 0);
  default:
    return gg_getmfgr(0x0034, NULL, 0);
  }
}

int ggSetITEnable(void) {
  return gg_getmfgr(0x0021, NULL, 0);
}

int ggSetChargeControl(int state) {
  uint8_t reg[2];
  int ret;

  /* Turn on charge control */
  ret = gg_getblock(0x03, reg, 2);
  if (ret < 0)
    return ret;
  if (state == 0) /* Inverse logic */
    reg[0] |= 1 << 6;
  else
    reg[0] &= ~(1 << 6);
  ret = gg_setblock(0x03, reg, 2);
  if (ret < 0)
    return ret;

  return ret;
}

int ggForceDischarge(int state) {
  uint16_t val;
  int ret;
  ret = gg_getword(0x46, &val);
  if (ret)
    return ret;
  if (state)
    val |= (1<<1);
  else
    val &= ~(1<<1);
  return gg_setword(0x46, val);
}

int ggPermanentFailureFlags(uint16_t *flags) {
  return gg_getflash_word(96, 0, flags);
}

int ggFuseFlag(uint16_t *flags) {
  return gg_getflash_word(96, 2, flags);
}

int ggPermanentFailureVoltage(uint16_t *voltage) {
  return gg_getflash_word(96, 4, voltage);
}

int ggPermanentFailureCellVoltage(int cell, uint16_t *voltage) {
  /* Cell 4 voltage is at offset 6, cell 1 voltage is at offset 12.*/
  return gg_getflash_word(96, 6 + ((3 - cell) * 2), voltage);
}

int ggPermanentFailureCurrent(int16_t *current) {
  return gg_getflash_word(96, 14, (uint16_t *)current);
}

int ggPermanentFailureTemperature(int16_t *temperature) {
  return gg_getflash_word(96, 16, (uint16_t *)temperature);
}

int ggPermanentFailureBatteryStatus(uint16_t *stat) {
  return gg_getflash_word(96, 18, stat);
}

int ggPermanentFailureRemainingCapacity(uint16_t *capacity) {
  return gg_getflash_word(96, 20, capacity);
}

int ggPermanentFailureChargeStatus(uint16_t *stat) {
  return gg_getflash_word(96, 24, stat);
}

int ggPermanentFailureSafetyStatus(uint16_t *stat) {
  return gg_getflash_word(96, 26, stat);
}

int ggPermanentFailureFlags2(uint16_t *flags) {
  return gg_getflash_word(96, 28, flags);
}

#include "chprintf.h"
int ggFullReset(void) {
  return gg_getmfgr(0x0041, NULL, 0);
}

int ggPermanentFailureReset(void) {
  msg_t status;
  uint8_t tx_bfr[3];
  uint8_t rx_bfr[5];

  tx_bfr[0] = 0x62; /* Get PFKey command.*/
  status = senokoI2cMasterTransmitTimeout(GG_ADDR,
                                          tx_bfr, 1,
                                          rx_bfr, sizeof(rx_bfr));
  if (status != MSG_OK)
    return (1 << 24) | senokoI2cErrors();
  chprintf(stream, "PFKey: %02x %02x %02x %02x %02x", rx_bfr[0], rx_bfr[1], rx_bfr[2], rx_bfr[3], rx_bfr[4]);

  tx_bfr[0] = 0x00; /* Manufacturer command.*/
  tx_bfr[1] = rx_bfr[2]; /* PFkey is in SBS order, not TI order.  Reverse it.*/
  tx_bfr[2] = rx_bfr[1]; /* PFkey is in SBS order, not TI order.  Reverse it.*/
  status = senokoI2cMasterTransmitTimeout(GG_ADDR,
                                          tx_bfr, sizeof(tx_bfr),
                                          NULL, 0);
  if (status != MSG_OK)
    return (2 << 24) | senokoI2cErrors();

  tx_bfr[0] = 0x00;
  tx_bfr[1] = rx_bfr[4]; /* PFkey is in SBS order, not TI order.  Reverse it.*/
  tx_bfr[2] = rx_bfr[3]; /* PFkey is in SBS order, not TI order.  Reverse it.*/
  status = senokoI2cMasterTransmitTimeout(GG_ADDR,
                                          tx_bfr, sizeof(tx_bfr),
                                          NULL, 0);
  if (status != MSG_OK)
    return (3 << 24) | senokoI2cErrors();

  return 0;
}
