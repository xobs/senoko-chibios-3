#ifndef __SENOKO_GG_H__
#define __SENOKO_GG_H__

int ggInit(void);
int ggRefresh(int property);
int ggManufacturer(uint8_t *manuf);
int ggPartName(uint8_t name[8]);
int ggChemistry(uint8_t *chem);
int ggSerial(uint16_t *serial);
int ggPercent(uint8_t *capacity);
int ggCellVoltage(int cell, uint16_t *voltage);
int ggMode(uint16_t *word);
int ggSetPrimary(void);
int ggSetSecondary(void);
int ggTemperature(int16_t *word);
int ggVoltage(uint16_t *word);
int ggCurrent(int16_t *word);
int ggChargingVoltage(uint16_t *word);
int ggChargingCurrent(uint16_t *word);
int ggFullCapacity(uint16_t *word);
int ggDesignCapacity(uint16_t *word);
int ggCurrent(int16_t *word);
int ggAverageCurrent(int16_t *word);
int ggStatus(uint16_t *word);
int ggFirmwareVersion(uint16_t *word);
int ggState(uint16_t *word);
int ggSetLeds(int state);
int ggSetChargeControl(int state);
int ggSetDsgFET(int state);
int ggSetChgFET(int state);
int ggChargingStatus(uint16_t *status);
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
int ggSetInhibitLow(int16_t temp);
int ggSetInhibitHigh(int16_t temp);
int ggInhibitLow(int16_t *temp);
int ggInhibitHigh(int16_t *temp);
int ggPrechgTemp(int16_t *temp);
int ggSetPrechgTemp(int16_t temp);
int ggDeadband(uint8_t *deadband);
int ggSetDeadband(uint8_t deadband);

int ggPermanentFailureFlags(uint16_t *flags);
int ggFuseFlag(uint16_t *flags);
int ggPermanentFailureVoltage(uint16_t *voltage);
int ggPermanentFailureCellVoltage(int cell, uint16_t *voltage);
int ggPermanentFailureCurrent(int16_t *current);
int ggPermanentFailureTemperature(int16_t *temperature);
int ggPermanentFailureBatteryStatus(uint16_t *stat);
int ggPermanentFailureRemainingCapacity(uint16_t *capacity);
int ggPermanentFailureChargeStatus(uint16_t *stat);
int ggPermanentFailureSafetyStatus(uint16_t *stat);
int ggPermanentFailureFlags2(uint16_t *flags);
int ggPermanentFailureReset(void);
int ggFullReset(void);
int ggSetBroadcast(int broadcast);
int ggSetFastChargeCurrent(int current);
int ggSetDefaults(int cells, int capacity, int current);

enum gg_temp_source {
  temp_internal,
  temp_ts1,
  temp_greater_ts1_or_ts2,
  temp_average_ts1_and_ts2,
};
int ggSetTemperatureSource(enum gg_temp_source);

#endif /* __SENOKO_GG_H__ */
