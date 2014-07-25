#ifndef __SENOKO_GG_H__
#define __SENOKO_GG_H__

int ggInit(void);
int ggRefresh(int property);
int ggManufacturer(uint8_t *manuf);
int ggPartName(uint8_t name[8]);
int ggChemistry(uint8_t *chem);
int ggSerial(void *serial);
int ggPercent(uint8_t *capacity);
int ggCellVoltage(int cell, void *voltage);
int ggMode(void *word);
int ggSetPrimary(void);
int ggSetSecondary(void);
int ggTemperature(int16_t *word);
int ggVoltage(void *word);
int ggCurrent(void *word);
int ggChargingVoltage(void *word);
int ggChargingCurrent(void *word);
int ggFullCapacity(int16_t *word);
int ggDesignCapacity(int16_t *word);
int ggCurrent(void *word);
int ggAverageCurrent(void *word);
int ggStatus(void *word);
int ggFirmwareVersion(void *word);
int ggState(void *word);
int ggSetLeds(int state);
int ggSetChargeControl(int state);
int ggForceDischarge(int state);
int ggSetManufacturer(uint8_t name[11]);
int ggSetChemistry(uint8_t chem[4]);
int ggSetCellCount(int cells);
int ggCellCount(uint8_t *cells);
int ggSetCapacity(int cells, uint16_t capacity);
int ggSetITEnable(void);
int ggTimeToEmpty(uint16_t *minutes);
int ggTimeToFull(uint16_t *minutes);
int ggCalibrate(int16_t voltage, int16_t current,
                uint16_t temperature, int cells);

#endif /* __SENOKO_GG_H__ */
