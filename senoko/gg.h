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
int ggCycleCount(uint16_t *count);
int ggSetCycleCount(uint16_t count);
int ggDesignCapacity(uint16_t *word);
int ggCurrent(int16_t *word);
int ggAverageCurrent(int16_t *word);
int ggStatus(uint16_t *word);
int ggSafetyAlert(uint16_t *word);
int ggSafetyStatus(uint16_t *word);
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
int ggStartImpedenceTrackTM(void);
int ggTimeToEmpty(uint16_t *minutes);
int ggTimeToFull(uint16_t *minutes);
int ggSetRemovable(int removable);
int ggRemovable(void);
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
int ggReboot(void);
int ggSetBroadcast(int broadcast);
int ggFastChargeCurrent(int16_t *current);
int ggSetFastChargeCurrent(int current);
int ggSetDefaults(int cells, int capacity, int current);

int ggTermVoltage(int16_t *voltage);
int ggSetTermVoltage(int voltage);

enum gg_temp_source {
  temp_internal,
  temp_ts1,
  temp_greater_ts1_or_ts2,
  temp_average_ts1_and_ts2,
};
int ggSetTemperatureSource(enum gg_temp_source);

enum gg_state {
  st_wake_up,
  st_normal_discharge,
  st_res1,
  st_pre_charge,
  st_res2,
  st_charge,
  st_res3,
  st_charge_termination,
  st_fault_charge_terminate,
  st_permanent_failure,
  st_overcurrent,
  st_overtemperature,
  st_battery_failure,
  st_sleep,
  st_reserved,
  st_battery_removed,
};

#endif /* __SENOKO_GG_H__ */
