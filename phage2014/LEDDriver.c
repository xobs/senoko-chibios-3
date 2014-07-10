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

// configure pwm timers -
// timer 4 as master, active for data transmission and inactive to disable transmission during reset period (50uS)
// timer 3 as slave, during active time creates a 1.25 uS signal, with duty cycle controlled by frame buffer values
/* master mode selection */
static const PWMConfig pwmc3 = {
  72000000,/* 72Mhz PWM clock frequency.   */
  90, /* 90 cycles period (1.25 uS per period @72Mhz       */
  NULL,
  {
    {PWM_OUTPUT_ACTIVE_HIGH, NULL},
    {PWM_OUTPUT_ACTIVE_HIGH, NULL},
    {PWM_OUTPUT_ACTIVE_HIGH, NULL},
    {PWM_OUTPUT_ACTIVE_HIGH, NULL}
  },
  0,
  0,
};
static const PWMConfig pwmc4 = {
  72000000 / 90, /* 800Khz PWM clock frequency. 1/90 of PWMC3   */
  (72000000 / 90) * 0.05, /*Total period is 50ms (20FPS), including sLeds cycles + reset length for ws2812b and FB writes  */
  NULL,
  {
    {PWM_OUTPUT_ACTIVE_HIGH, NULL},
    {PWM_OUTPUT_DISABLED, NULL},
    {PWM_OUTPUT_DISABLED, NULL},
    {PWM_OUTPUT_DISABLED, NULL}
  },
  TIM_CR2_MMS_2, /* master mode selection */
  0,
};

void setColor(uint8_t color, uint8_t *buf,uint32_t mask)
{
  int i;
  for (i=0; i<8; i++) {
    buf[i] = ((color << i) & 0b10000000 ? 0x0 : mask);
  }
}

void setColorRGB(Color c, uint8_t *buf, uint32_t mask){
  setColor(c.G,buf, mask);
  setColor(c.R,buf+8, mask);
  setColor(c.B,buf+16, mask);
}

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
void ledDriverInit(int leds, GPIO_TypeDef *port, uint32_t mask, uint8_t **o_fb)
{
  int j;
  sLeds=leds;
  sPort=port;
  sMask=mask;

  fb = chHeapAlloc(NULL, ((sLeds) * 24) + 10);
  *o_fb=fb;
  for (j = 0; j < (sLeds) * 24; j++)
    fb[j] = 0;
  dma_source[0] = sMask;

  // DMA stream 2, triggered by channel3 pwm signal. if FB indicates, reset output value early to indicate "0" bit to ws2812
  dmaStreamAllocate(STM32_DMA1_STREAM2, 10, NULL, NULL);
  dmaStreamSetPeripheral(STM32_DMA1_STREAM2, &(sPort->BSRR/*clear*/));
  dmaStreamSetMemory0(STM32_DMA1_STREAM2, fb);
  dmaStreamSetTransactionSize(STM32_DMA1_STREAM2, (sLeds) * 24);
  dmaStreamSetMode(
      STM32_DMA1_STREAM2,
      STM32_DMA_CR_DIR_M2P | STM32_DMA_CR_MINC | STM32_DMA_CR_PSIZE_BYTE
      | STM32_DMA_CR_MSIZE_BYTE | STM32_DMA_CR_CIRC | STM32_DMA_CR_PL(2));
  // DMA stream 3, triggered by pwm update event. output high at beginning of signal
  dmaStreamAllocate(STM32_DMA1_STREAM3, 10, NULL, NULL);
  dmaStreamSetPeripheral(STM32_DMA1_STREAM3, &(sPort->BSRR/*set*/));
  dmaStreamSetMemory0(STM32_DMA1_STREAM3, dma_source);
  dmaStreamSetTransactionSize(STM32_DMA1_STREAM3, 1);
  dmaStreamSetMode(
      STM32_DMA1_STREAM3, STM32_DMA_CR_TEIE |
      STM32_DMA_CR_DIR_M2P | STM32_DMA_CR_PSIZE_BYTE | STM32_DMA_CR_MSIZE_BYTE
      | STM32_DMA_CR_CIRC | STM32_DMA_CR_PL(3));
  // DMA stream 6, triggered by channel1 update event. reset output value late to indicate "1" bit to ws2812.
  // always triggers but no affect if dma stream 2 already change output value to 0
  dmaStreamAllocate(STM32_DMA1_STREAM6, 10, NULL, NULL);
  dmaStreamSetPeripheral(STM32_DMA1_STREAM6, &(sPort->BSRR/*clear*/));
  dmaStreamSetMemory0(STM32_DMA1_STREAM6, dma_source);
  dmaStreamSetTransactionSize(STM32_DMA1_STREAM6, 1);
  dmaStreamSetMode(
      STM32_DMA1_STREAM6,
      STM32_DMA_CR_DIR_M2P | STM32_DMA_CR_PSIZE_BYTE | STM32_DMA_CR_MSIZE_BYTE
      | STM32_DMA_CR_CIRC | STM32_DMA_CR_PL(3));
  pwmStart(&PWMD4, &pwmc4);
  pwmStart(&PWMD3, &pwmc3);
  // set pwm3 as slave, triggerd by pwm2 oc1 event. disables pwmd2 for synchronization.
  PWMD3.tim->SMCR |= TIM_SMCR_SMS_0 | TIM_SMCR_SMS_2 | TIM_SMCR_TS_0;
  PWMD4.tim->CR1 &= ~TIM_CR1_CEN;
  // set pwm values.
  // 28 (duty in ticks) / 90 (period in ticks) * 1.25uS (period in S) = 0.39 uS
  pwmEnableChannel(&PWMD3, 2, 28);
  // 58 (duty in ticks) / 90 (period in ticks) * 1.25uS (period in S) = 0.806 uS
  pwmEnableChannel(&PWMD3, 0, 58);
  // active during transfer of 90 cycles * sLeds * 24 bytes * 1/90 multiplier
  pwmEnableChannel(&PWMD4, 0, 90 * sLeds * 24 / 90);
  // stop and reset counters for synchronization
  PWMD4.tim->CNT = 0;
  // Slave (TIM3) needs to "update" immediately after master (TIM2) start in order to start in sync.
  // this initial sync is crucial for the stability of the run
  PWMD3.tim->CNT = 89;
  PWMD3.tim->DIER |= TIM_DIER_CC3DE | TIM_DIER_CC1DE | TIM_DIER_UDE;
  dmaStreamEnable(STM32_DMA1_STREAM3);
  dmaStreamEnable(STM32_DMA1_STREAM6);
  dmaStreamEnable(STM32_DMA1_STREAM2);
  // all systems go! both timers and all channels are configured to resonate
  // in complete sync without any need for CPU cycles (only DMA and timers)
  // start pwm2 for system to start resonating
  PWMD4.tim->CR1 |= TIM_CR1_CEN;
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
  for (i=0; i<sLeds; i++) {
    setColorRGB(tmpC, fb + 24 * i, sMask);
  }
}

