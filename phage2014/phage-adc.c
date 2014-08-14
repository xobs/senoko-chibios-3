#include "ch.h"
#include "hal.h"
#include "adc.h"
#include "adc_lld.h"

#include "phage.h"
#include "chprintf.h"

#define ADC_NUM_CHANNELS   1
#define ADC_BUF_DEPTH      1
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
  FALSE,                 /* Circular buffer */
  ADC_NUM_CHANNELS,
  adccallback,
  adcerrorcallback,
  0, 0,           /* CR1, CR2 */
  0,                            /* SMPR1 */
  ADC_SMPR2_SMP_AN1(ADC_SAMPLE_41P5),
  ADC_SQR1_NUM_CH(ADC_NUM_CHANNELS),
  0,
  /*ADC_SQR3_SQ6_N(ADC_CHANNEL_IN1)   | ADC_SQR3_SQ5_N(ADC_CHANNEL_IN1) |
  ADC_SQR3_SQ4_N(ADC_CHANNEL_IN1)   | ADC_SQR3_SQ3_N(ADC_CHANNEL_IN1) |
  ADC_SQR3_SQ2_N(ADC_CHANNEL_IN1)   | */ADC_SQR3_SQ1_N(ADC_CHANNEL_IN1)
};

void phageAdcInit(void) {
  uint32_t cr2;

  /*
   * Activates the ADC1 driver and the temperature sensor.
   */
  adcStart(&ADCD1, NULL);

  /* ADC setup.*/
  ADCD1.adc->CR1   = adcgrpcfg.cr1 | ADC_CR1_SCAN;
  cr2 = adcgrpcfg.cr2 | ADC_CR2_ADON | ADC_CR2_CONT;
  ADCD1.adc->CR2   = adcgrpcfg.cr2 | cr2;
  ADCD1.adc->SMPR1 = adcgrpcfg.smpr1;
  ADCD1.adc->SMPR2 = adcgrpcfg.smpr2;
  ADCD1.adc->SQR1  = adcgrpcfg.sqr1;
  ADCD1.adc->SQR2  = adcgrpcfg.sqr2;
  ADCD1.adc->SQR3  = adcgrpcfg.sqr3;

  /* ADC start by writing ADC_CR2_ADON a second time.*/
  ADCD1.adc->CR2   = cr2;
}

adcsample_t phageAdcGet(void) {
  return ADCD1.adc->DR;
}
