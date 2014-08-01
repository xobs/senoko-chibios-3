#include "ch.h"
#include "hal.h"
#include "adc.h"
#include "adc_lld.h"

#include "phage.h"
#include "chprintf.h"

#define ADC_NUM_CHANNELS   2
#define ADC_BUF_DEPTH      16
static adcsample_t samples[ADC_NUM_CHANNELS * ADC_BUF_DEPTH];

/*
 * ADC streaming callback.
 */
size_t nx = 0, ny = 0;
static void adccallback(ADCDriver *adcp, adcsample_t *buffer, size_t n) {

  (void)adcp;
  (void)buffer;
  (void)n;
  /*
  if (samples == buffer) {
    nx += n;
  }
  else {
    ny += n;
  }
*/
//  chSysLockFromISR();
//  chSysUnlockFromISR();
}

static void adcerrorcallback(ADCDriver *adcp, adcerror_t err) {

  (void)adcp;
  (void)err;

//  chSysLockFromISR();
//  chSysUnlockFromISR();
}

/*
 * ADC conversion group.
 * Mode:        Continuous, 16 samples of 8 channels, SW triggered.
 * Channels:    IN10, IN11, IN10, IN11, IN10, IN11, Sensor, VRef.
 */
static const ADCConversionGroup adcgrpcfg = {
  TRUE,
  ADC_NUM_CHANNELS,
  adccallback,
  adcerrorcallback,
  0, ADC_CR2_TSVREFE,           /* CR1, CR2 */
  ADC_SMPR1_SMP_AN11(ADC_SAMPLE_41P5) | ADC_SMPR1_SMP_AN10(ADC_SAMPLE_41P5) |
  ADC_SMPR1_SMP_SENSOR(ADC_SAMPLE_239P5) | ADC_SMPR1_SMP_VREF(ADC_SAMPLE_239P5),
  0,                            /* SMPR2 */
  ADC_SQR1_NUM_CH(ADC_NUM_CHANNELS),
  ADC_SQR2_SQ8_N(ADC_CHANNEL_SENSOR) | ADC_SQR2_SQ7_N(ADC_CHANNEL_VREFINT),
  ADC_SQR3_SQ6_N(ADC_CHANNEL_IN11)   | ADC_SQR3_SQ5_N(ADC_CHANNEL_IN10) |
  ADC_SQR3_SQ4_N(ADC_CHANNEL_IN11)   | ADC_SQR3_SQ3_N(ADC_CHANNEL_IN10) |
  ADC_SQR3_SQ2_N(ADC_CHANNEL_IN11)   | ADC_SQR3_SQ1_N(ADC_CHANNEL_IN10)
};


void phageAdcInit(void) {
  /*
   * Activates the ADC1 driver and the temperature sensor.
   */
  adcStart(&ADCD1, NULL);

  /*
   * Starts an ADC continuous conversion.
   */
  adcStartConversion(&ADCD1, &adcgrpcfg, samples, ADC_BUF_DEPTH);
}
