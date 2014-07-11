/*
 * LEDDriver.c
 *
 *  Created on: Aug 26, 2013
 *      Author: Omri Iluz
 */

#include "ch.h"
#include "hal.h"
#include "pwm.h"
#include "LEDDriver.h"

static uint8_t *fb;
static int sLeds;
static GPIO_TypeDef *sPort;
static uint32_t sMask;
static uint8_t dma_source[1];

enum pin_state {
  pin_clear = 0,
  pin_set = 1,
};

/* Timer 2 as master, active for data transmission and inactive to disable
   transmission during reset period (50uS). */
static const PWMConfig pwmc2 = {
  //36000000 / 45, /* 800Khz PWM clock frequency. 1/45 of PWMC3. */
  8000000,/* 36Mhz PWM clock frequency. */

  /* Total period is 50ms (20FPS), including sLeds cycles + reset length
     for ws2812b and FB writes. */
  //(36000000 / 45) * 0.05,
  6,
  NULL,
  {
    {PWM_OUTPUT_ACTIVE_HIGH, NULL}, /* PWM3 Channel 1 (TIM3_CH3) */
    {PWM_OUTPUT_DISABLED, NULL},    /* PWM3 Channel 0 */
    {PWM_OUTPUT_DISABLED, NULL},    /* PWM3 Channel 3 */
    {PWM_OUTPUT_DISABLED, NULL},    /* PWM3 Channel 3 */
  },
  TIM_CR2_MMS_2, /* master mode selection */
  0,
};

/* Timer 3 as slave, during active time creates a 1.25 uS signal,
   with duty cycle controlled by frame buffer values. */

static const PWMConfig pwmc3 = {
  8000000,/* 36Mhz PWM clock frequency. */
  6, /* 45 cycles period (1.25 uS per period @36Mhz. */
  NULL,
  {
    {PWM_OUTPUT_ACTIVE_HIGH, NULL}, /* PWM3 Channel 0 */
    {PWM_OUTPUT_ACTIVE_HIGH, NULL}, /* PWM3 Channel 1 */
    {PWM_OUTPUT_ACTIVE_HIGH, NULL}, /* PWM3 Channel 2 */
    {PWM_OUTPUT_ACTIVE_HIGH, NULL}, /* PWM3 Channel 3 */
  },
  0,
  0,
};

void setColor(uint8_t color, uint8_t *buf, uint32_t mask)
{
  int i;
  for (i = 0; i < 8; i++) {
    buf[i] = ((color << i) & 0b10000000 ? 0x0 : mask);
  }
}

void setColorRGB(Color c, uint8_t *buf, uint32_t mask)
{
  setColor(c.G, buf, mask);
  setColor(c.R, buf + 8, mask);
  setColor(c.B, buf + 16, mask);
}

/* There are a few PWMs in play here:
    - Master PWM runs at 800 kHz.  This is the "pixclk".  It indicates a new
      pixel is being transmitted, and writes a "1" out the pin.  It is
      1.3 uS long, with a 46% duty cycle.
    - The "1" clock, which runs 0.7uS after the master clock, and writes
      a "0" out the pin.
    - The "0" clock, which runs 0.35uS after the master pixclk IFF the value
      in the pixel is 1.  Otherwise, it does not get queued.
*/

/**
 * @brief   Initialize Led Driver
 * @details Initialize the Led Driver based on parameters.
 *          Following initialization, the frame buffer would automatically be
 *          exported to the supplied port and pins in the right timing to drive
 *          a chain of WS2812B controllers
 * @note    The function assumes the controller is running at 72Mhz
 * @note    Timing is critical for WS2812. While all timing is done in hardware
 *          need to verify memory bandwidth is not exhausted to avoid DMA delays
 *
 * @param[in] leds      length of the LED chain controlled by each pin
 * @param[in] port      which port would be used for output
 * @param[in] mask      Which pins would be used for output, each pin is a full chain
 * @param[out] o_fb     initialized frame buffer
 *
 */

static uint32_t temp_fb[24];
void ledDriverInit(int leds, GPIO_TypeDef *port, uint32_t mask, uint8_t **o_fb)
{
  int j;
  sLeds = leds;
  sPort = port;
  sMask = mask;

  fb = chHeapAlloc(NULL, ((sLeds) * 24) + 10);
  *o_fb=fb;
  for (j = 0; j < (sLeds) * 24; j++)
    fb[j] = (j & 1) ? mask : ~mask;

  for (j = 0; j < 24; j++)
    temp_fb[j] = (j & 1) ? 0xffff : 0xffff0000;

  /* "SET" bits */
  dma_source[0] = mask;

  /* DMA stream 2, triggered by channel3 pwm signal.  If FB indicates,
     reset output value early to indicate "0" bit to ws2812. */
  dmaStreamAllocate(STM32_DMA1_STREAM2, 10, NULL, NULL);
  dmaStreamSetPeripheral(STM32_DMA1_STREAM2, &(sPort->BSRR));
  dmaStreamSetMemory0(STM32_DMA1_STREAM2, temp_fb);
  dmaStreamSetTransactionSize(STM32_DMA1_STREAM2, 24 * 4);
  dmaStreamSetMode(
      STM32_DMA1_STREAM2,
      STM32_DMA_CR_DIR_M2P | STM32_DMA_CR_MINC | STM32_DMA_CR_PSIZE_BYTE
      | STM32_DMA_CR_MSIZE_BYTE | STM32_DMA_CR_CIRC | STM32_DMA_CR_PL(2));

#if 0
  /* DMA stream 3, triggered by pwm update event. output high at the
     beginning of signal. */
  dmaStreamAllocate(STM32_DMA1_STREAM3, 10, NULL, NULL);
  dmaStreamSetPeripheral(STM32_DMA1_STREAM3, &(sPort->BSRR));
  dmaStreamSetMemory0(STM32_DMA1_STREAM3, &dma_source[pin_clear]);
  dmaStreamSetTransactionSize(STM32_DMA1_STREAM3, 4);
  dmaStreamSetMode(
      STM32_DMA1_STREAM3, STM32_DMA_CR_TEIE |
      STM32_DMA_CR_DIR_M2P | STM32_DMA_CR_PSIZE_WORD | STM32_DMA_CR_MSIZE_BYTE
      | STM32_DMA_CR_CIRC | STM32_DMA_CR_PL(3));

  /* DMA stream 6, triggered by channel1 update event. reset output value
     late to indicate "1" bit to ws2812.  Always triggers but no affect if
     dma stream 2 already change output value to 0. */
  dmaStreamAllocate(STM32_DMA1_STREAM6, 10, NULL, NULL);
  dmaStreamSetPeripheral(STM32_DMA1_STREAM6, &(sPort->BSRR));
  dmaStreamSetMemory0(STM32_DMA1_STREAM6, &dma_source[pin_set]);
  dmaStreamSetTransactionSize(STM32_DMA1_STREAM6, 4);
  dmaStreamSetMode(
      STM32_DMA1_STREAM6,
      STM32_DMA_CR_DIR_M2P | STM32_DMA_CR_PSIZE_WORD | STM32_DMA_CR_MSIZE_BYTE
      | STM32_DMA_CR_CIRC | STM32_DMA_CR_PL(3));
#endif

  pwmStart(&PWMD2, &pwmc2);
  pwmStart(&PWMD3, &pwmc3);

  // set pwm3 as slave, triggerd by pwm2 oc1 event. disables pwmd2 for synchronization.
  PWMD3.tim->SMCR |= TIM_SMCR_SMS_0 | TIM_SMCR_SMS_2 | TIM_SMCR_TS_0;
  PWMD2.tim->CR1 &= ~TIM_CR1_CEN;
  // set pwm values.
  // 28 (duty in ticks) / 90 (period in ticks) * 1.25uS (period in S) = 0.39 uS
  pwmEnableChannel(&PWMD3, 2, 2);
  // 58 (duty in ticks) / 90 (period in ticks) * 1.25uS (period in S) = 0.806 uS
  pwmEnableChannel(&PWMD3, 0, 7);
  // active during transfer of 90 cycles * sLeds * 24 bytes * 1/90 multiplier
  pwmEnableChannel(&PWMD2, 0, 45 * sLeds * 24 / 45);
  // stop and reset counters for synchronization
  PWMD2.tim->CNT = 0;
  // Slave (TIM3) needs to "update" immediately after master (TIM2) start in order to start in sync.
  // this initial sync is crucial for the stability of the run
  PWMD3.tim->CNT = 20;
  PWMD3.tim->DIER |= TIM_DIER_CC3DE | TIM_DIER_CC1DE | TIM_DIER_UDE;
  dmaStreamEnable(STM32_DMA1_STREAM3);
  dmaStreamEnable(STM32_DMA1_STREAM6);
  dmaStreamEnable(STM32_DMA1_STREAM2);
  // all systems go! both timers and all channels are configured to resonate
  // in complete sync without any need for CPU cycles (only DMA and timers)
  // start pwm2 for system to start resonating
  PWMD2.tim->CR1 |= TIM_CR1_CEN;
}

#define ptr_start ((int *)0x08000000)
#define ptr_end ((int *)0x8008000)
static int rand(void)
{
  static int *ptr = ptr_start;
  if (ptr > ptr_end)
    ptr = ptr_start;
  return *ptr++;
}

void testPatternFB(uint8_t *fb)
{
  int i;
  Color tmpC = { rand()%256, rand()%256, rand()%256 };
  for (i = 0; i < sLeds; i++) {
    setColorRGB(tmpC, fb + 24 * i, sMask);
  }
}

