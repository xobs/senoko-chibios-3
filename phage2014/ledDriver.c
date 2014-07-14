/*
 * LEDDriver.c
 *
 *  Created on: Aug 26, 2013
 *      Author: Omri Iluz
 */

#include "ch.h"
#include "hal.h"
#include "pwm.h"
#include "ledDriver.h"

static int maxPixels;
static GPIO_TypeDef *sPort;
static uint32_t sMask;
static uint32_t dma_source[1];
static uint32_t dmaBuffer[24 * 2]; // Two-pixel DMA buffer
static int currentPixel; // Current pixel being DMAed

enum pin_state {
  pin_clear = 0,
  pin_set = 1,
};

/* Timer 2 as master, active for data transmission and inactive to disable
   transmission during reset period (50uS). */
static const PWMConfig pwmc2 = {
  36000000 / 45, /* 800Khz PWM clock frequency. 1/45 of PWMC3. */

  /* Total period is 50ms (20FPS), including maxPixels cycles + reset length
     for ws2812b and FB writes. */
  (36000000 / 45) * 0.05,
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
  36000000,/* 36Mhz PWM clock frequency. */
  45, /* 45 cycles period (1.25 uS per period @36Mhz. */
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

void ledSetRGB(void *ptr, int x, uint8_t r, uint8_t g, uint8_t b)
{
  uint8_t *buf = ((uint8_t *)ptr) + (3 * x);
  buf[0] = g / 8;
  buf[1] = r / 8;
  buf[2] = b / 8;
}

/**
 * @brief   Unpack a framebuffer pixel for GPIO output.
 * @details Take the next 24 bits of pixel data and stripe it across 96
 *          bytes of dma data, to be written out the GPIO port.  Write data
 *          either to the upper half or the lower half of the outgoing DMA
 *          buffer.
 * @note    This function updates the DMA data mid-transaction.
 *
 * @param[in] ptr       pointer to the framebuffer data to update
 * @param[in] flags     DMA engine IRQ flags
 *
 */
static void unpackFramebuffer(void *ptr, uint32_t flags)
{
  int color, bit;
  uint8_t *fb = ptr;
  uint8_t *srcBuffer = ((uint8_t *)fb) + (currentPixel * 3);
  uint32_t *destBuffer;

  if ( (flags & STM32_DMA_ISR_TCIF) != 0) {
    /* Finished the second pixel, so update the second pixel */
    destBuffer = dmaBuffer + 24;
  }
  else if ( (flags & STM32_DMA_ISR_HTIF) != 0) {
    destBuffer = dmaBuffer;
    /* Finished first pixel, moving on to the second.
       Unpack the next pixel into the first half of the buffer. */
  }
  else
    return;

  /* Copy the three color components, bit-by-bit */
  for (color = 0; color < 3; color++) {
    for (bit = 0; bit < 8; bit++) {
       *destBuffer++ = ((*srcBuffer << bit) & 0b10000000 ? 0x0 : sMask);
    }
    srcBuffer++;
  }

  currentPixel++;
  if (currentPixel >= maxPixels)
    currentPixel = 0;
}

/**
 * @brief   Initialize Led Driver
 * @details Initialize the Led Driver based on parameters.
 *          Following initialization, the frame buffer would automatically be
 *          exported to the supplied port and pins in the right timing to drive
 *          a chain of WS2812B controllers
 * @note    The function assumes the controller is running at 36Mhz
 * @note    Timing is critical for WS2812. While all timing is done in hardware
 *          need to verify memory bandwidth is not exhausted to avoid DMA delays
 *
 * @param[in] leds      length of the LED chain controlled by each pin
 * @param[in] port      which port would be used for output
 * @param[in] mask      Which pins would be used for output, each pin is a full chain
 * @param[out] fb       initialized frame buffer
 *
 */

void ledDriverInit(int leds, GPIO_TypeDef *port, uint32_t mask, uint8_t **fb)
{
  int j;
  maxPixels = leds;
  sPort = port;
  sMask = (mask << 16) & 0xffff0000;

  (*fb) = chHeapAlloc(NULL, maxPixels * 3);
  for (j = 0; j < maxPixels * 3; j++)
    (*fb)[j] = 0;

  /* "SET" bits */
  dma_source[pin_set] = mask & 0xffff;
  dma_source[pin_clear] = (mask << 16) & 0xffff0000;

  return;
}

void ledDriverStart(uint8_t *fb)
{
  /* DMA stream 2, triggered by channel3 pwm signal.  If FB indicates,
     reset output value early to indicate "0" bit to ws2812. */
  dmaStreamAllocate(STM32_DMA1_STREAM2, 10, unpackFramebuffer, fb);
  dmaStreamSetPeripheral(STM32_DMA1_STREAM2, &(sPort->BSRR));
  dmaStreamSetMemory0(STM32_DMA1_STREAM2, dmaBuffer);
  dmaStreamSetTransactionSize(STM32_DMA1_STREAM2, 2 * 24);
  dmaStreamSetMode(
      STM32_DMA1_STREAM2,
      STM32_DMA_CR_DIR_M2P | STM32_DMA_CR_MINC | STM32_DMA_CR_PSIZE_WORD
      | STM32_DMA_CR_HTIE | STM32_DMA_CR_TCIE
      | STM32_DMA_CR_MSIZE_WORD | STM32_DMA_CR_CIRC | STM32_DMA_CR_PL(2));

  /* DMA stream 3, triggered by pwm update event. output high at the
     beginning of signal. */
  dmaStreamAllocate(STM32_DMA1_STREAM3, 10, NULL, NULL);
  dmaStreamSetPeripheral(STM32_DMA1_STREAM3, &(sPort->BSRR));
  dmaStreamSetMemory0(STM32_DMA1_STREAM3, &dma_source[pin_set]);
  dmaStreamSetTransactionSize(STM32_DMA1_STREAM3, 1);
  dmaStreamSetMode(
      STM32_DMA1_STREAM3, STM32_DMA_CR_TEIE |
      STM32_DMA_CR_DIR_M2P | STM32_DMA_CR_PSIZE_WORD | STM32_DMA_CR_MSIZE_WORD
      | STM32_DMA_CR_CIRC | STM32_DMA_CR_PL(3));

  /* DMA stream 6, triggered by channel1 update event. reset output value
     late to indicate "1" bit to ws2812.  Always triggers but no affect if
     dma stream 2 already change output value to 0. */
  dmaStreamAllocate(STM32_DMA1_STREAM6, 10, NULL, NULL);
  dmaStreamSetPeripheral(STM32_DMA1_STREAM6, &(sPort->BSRR));
  dmaStreamSetMemory0(STM32_DMA1_STREAM6, &dma_source[pin_clear]);
  dmaStreamSetTransactionSize(STM32_DMA1_STREAM6, 1);
  dmaStreamSetMode(
      STM32_DMA1_STREAM6,
      STM32_DMA_CR_DIR_M2P | STM32_DMA_CR_PSIZE_WORD | STM32_DMA_CR_MSIZE_WORD
      | STM32_DMA_CR_CIRC | STM32_DMA_CR_PL(3));

  pwmStart(&PWMD2, &pwmc2);
  pwmStart(&PWMD3, &pwmc3);

  // set pwm3 as slave, triggerd by pwm2 oc1 event. disables pwmd2 for synchronization.
  PWMD3.tim->SMCR |= TIM_SMCR_SMS_0 | TIM_SMCR_SMS_2 | TIM_SMCR_TS_0;
  PWMD2.tim->CR1 &= ~TIM_CR1_CEN;
  // set pwm values.
  // 28 (duty in ticks) / 90 (period in ticks) * 1.25uS (period in S) = 0.39 uS
  pwmEnableChannel(&PWMD3, 2, 14);
  // 58 (duty in ticks) / 90 (period in ticks) * 1.25uS (period in S) = 0.806 uS
  pwmEnableChannel(&PWMD3, 0, 29);
  // active during transfer of 90 cycles * maxPixels * 24 bytes * 1/90 multiplier
  pwmEnableChannel(&PWMD2, 0, 45 * maxPixels * 24 / 45);
  // stop and reset counters for synchronization
  PWMD2.tim->CNT = 0;

  // Slave (TIM3) needs to "update" immediately after master (TIM2) start in order to start in sync.
  // this initial sync is crucial for the stability of the run
  PWMD3.tim->CNT = 44;
  PWMD3.tim->DIER |= TIM_DIER_CC3DE | TIM_DIER_CC1DE | TIM_DIER_UDE;
  dmaStreamEnable(STM32_DMA1_STREAM3);
  dmaStreamEnable(STM32_DMA1_STREAM6);
  dmaStreamEnable(STM32_DMA1_STREAM2);
  // all systems go! both timers and all channels are configured to resonate
  // in complete sync without any need for CPU cycles (only DMA and timers)
  // start pwm2 for system to start resonating
  PWMD2.tim->CR1 |= TIM_CR1_CEN;
}

void ledDriverStop(void)
{
  dmaStreamDisable(STM32_DMA1_STREAM2);
  dmaStreamDisable(STM32_DMA1_STREAM6);
  dmaStreamDisable(STM32_DMA1_STREAM3);
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

static Color Wheel(uint8_t wheelPos) {
  Color c;

  if (wheelPos < 85) {
    c.r = wheelPos * 3;
    c.g = 255 - wheelPos * 3;
    c.b = 0;
  }
  else if (wheelPos < 170) {
    wheelPos -= 85;
    c.r = 255 - wheelPos * 3;
    c.g = 0;
    c.b = wheelPos * 3;
  }
  else {
    wheelPos -= 170;
    c.r = 0;
    c.g = wheelPos * 3;
    c.b = 255 - wheelPos * 3;
  }
  return c;
}

static void calmPatternFB(uint8_t *fb, int count)
{
  int i;
  static int j = 0;
  count &= 0xff;
  
  j = j % (256 * 5);
  for (i = 0; i < maxPixels; i++) {
    Color c;
    c = Wheel( (i * (256 / maxPixels) + j) & 0xFF );
    ledSetRGB(fb, i, c.r, c.g, c.b);
  }
  j += 2;
}

static void testPatternFB(uint8_t *fb)
{
  int i = 0;

  ledSetRGB(fb, i++, 0, 0, 0);
  ledSetRGB(fb, i++, 255, 0, 0);
  ledSetRGB(fb, i++, 0, 255, 0);
  ledSetRGB(fb, i++, 0, 0, 255);
  ledSetRGB(fb, i++, 255, 0, 255);
  ledSetRGB(fb, i++, 255, 255, 0);
  ledSetRGB(fb, i++, 0, 255, 255);

  for (; i < maxPixels; i++)
    ledSetRGB(fb, i, 255, 255, 255);
}

static void shootPatternFB(uint8_t *fb, int count)
{
  count %= maxPixels;
  int i;

  for (i = 0; i < maxPixels; i++) {
    if (count == i)
      ledSetRGB(fb, i, 255, 255, 255);
    else
      ledSetRGB(fb, i, 0, 0, 0);
  }
}

void runPatternFB(uint8_t *fb, enum pattern p, int count, uint32_t param)
{
  (void)param;
  if (p == patternShoot)
    shootPatternFB(fb, count);
  else if (p == patternCalm)
    calmPatternFB(fb, count);
  else if (p == patternTest)
    testPatternFB(fb);
}
