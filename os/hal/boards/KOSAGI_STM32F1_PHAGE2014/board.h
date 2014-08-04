/*
    ChibiOS/RT - Copyright (C) 2006-2013 Giovanni Di Sirio

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#ifndef _BOARD_H_
#define _BOARD_H_

/*
 * Setup for Senoko STM32F1 board.
 */

/*
 * Board identifier.
 */
#define BOARD_KOSAGI_STM32F1_SENOKO
#define BOARD_NAME                  "Phage2014 STM32F1"


/*
 * Board oscillators-related settings.
 * NOTE: HSE not fitted.
 */
#if !defined(STM32_LSECLK)
#define STM32_LSECLK                32768
#endif

#if !defined(STM32_HSECLK)
#define STM32_HSECLK                0
#endif


/*
 * Board voltages.
 * Required for performance limits calculation.
 */
#define STM32_VDD                   330

/*
 * MCU type as defined in the ST header file stm32f1xx.h.
 */
#define STM32F10X_MD

/*
 * IO pins assignments.
 */
#define PA0   0
#define PA1   1
#define PA2   2
#define PA3   3
#define PA4   4
#define PA5   5
#define PA6   6
#define PA7   7
#define PA8   8
#define PA9   9
#define PA10  10
#define PA11  11
#define PA12  12
#define PA13  13
#define PA14  14
#define PA15  15

#define PB0   0
#define PB1   1
#define PB2   2
#define PB3   3
#define PB4   4
#define PB5   5
#define PB6   6
#define PB7   7
#define PB8   8
#define PB9   9
#define PB10  10
#define PB11  11
#define PB12  12
#define PB13  13
#define PB14  14
#define PB15  15

#define PC0   0
#define PC1   1
#define PC2   2
#define PC3   3
#define PC4   4
#define PC5   5
#define PC6   6
#define PC7   7
#define PC8   8
#define PC9   9
#define PC10  10
#define PC11  11
#define PC12  12
#define PC13  13
#define PC14  14
#define PC15  15

#define PD0   0
#define PD1   1
#define PD2   2
#define PD3   3
#define PD4   4
#define PD5   5
#define PD6   6
#define PD7   7
#define PD8   8
#define PD9   9
#define PD10  10
#define PD11  11
#define PD12  12
#define PD13  13
#define PD14  14
#define PD15  15

#define PE0   0
#define PE1   1
#define PE2   2
#define PE3   3
#define PE4   4
#define PE5   5
#define PE6   6
#define PE7   7
#define PE8   8
#define PE9   9
#define PE10  10
#define PE11  11
#define PE12  12
#define PE13  13
#define PE14  14
#define PE15  15

/*
 * I/O ports initial setup, this configuration is established soon after reset
 * in the initialization code.
 * Please refer to the STM32 Reference Manual for details.
 */
#define PIN_OSPEED_INPUT(n)         (0U << (((n) & 7) * 4))
#define PIN_OSPEED_10M(n)           (1U << (((n) & 7) * 4))
#define PIN_OSPEED_2M(n)            (2U << (((n) & 7) * 4))
#define PIN_OSPEED_50M(n)           (3U << (((n) & 7) * 4))

/* Input (cnf) */
#define PIN_MODE_ANALOG(n)          (0U << ((((n) & 7) * 4) + 2))
#define PIN_MODE_FLOATING(n)        (1U << ((((n) & 7) * 4) + 2))
#define PIN_MODE_INPUT(n)           (2U << ((((n) & 7) * 4) + 2))

/* Output (cnf) */
#define PIN_OTYPE_PUSHPULL(n)       (0U << ((((n) & 7) * 4) + 2))
#define PIN_OTYPE_OPENDRAIN(n)      (1U << ((((n) & 7) * 4) + 2))
#define PIN_OTYPE_AF_PUSHPULL(n)    (2U << ((((n) & 7) * 4) + 2))
#define PIN_OTYPE_AF_OPENDRAIN(n)   (3U << ((((n) & 7) * 4) + 2))

/* Set not-present pins to floating inputs */
#define PIN_NOTPRESENT(n)           (4U << (((n) & 7) * 4))
#define PIN_UNUSED(n)               PIN_NOTPRESENT(n)

/* Output data (ODR) */
#define PIN_ODR_HIGH(n)             (1U << ((n) & 7))
#define PIN_ODR_LOW(n)

#define VAL_GPIOA_CRL   ( 0 \
        \
        /* CHG_CRIT */ \
        | PIN_OSPEED_INPUT(PA0) \
        | PIN_MODE_FLOATING(PA0) \
        \
        | PIN_UNUSED(PA1) \
        \
        | PIN_OSPEED_2M(PA2) \
        | PIN_OTYPE_PUSHPULL(PA2) \
        \
        | PIN_UNUSED(PA3) \
        \
        /* Radio CSEL (NSS) */ \
        | PIN_OSPEED_10M(PA4) \
        | PIN_OTYPE_PUSHPULL(PA4) \
        \
        /* Radio SCK */ \
        | PIN_OSPEED_10M(PA5) \
        | PIN_OTYPE_AF_PUSHPULL(PA5) \
        \
        /* Radio MISO */ \
        | PIN_OSPEED_10M(PA6) \
        | PIN_OTYPE_AF_PUSHPULL(PA6) \
        \
        /* Radio MOSI */ \
        | PIN_OSPEED_10M(PA7) \
        | PIN_OTYPE_AF_PUSHPULL(PA7) \
        \
        | 0)

#define VAL_GPIOA_CRH   ( 0 \
        /* Accelerometer IRQ1 */ \
        | PIN_OSPEED_INPUT(PA8) \
        | PIN_MODE_FLOATING(PA8) \
        \
        /* USART1_TX */ \
        | PIN_OSPEED_2M(PA9) \
        | PIN_OTYPE_AF_PUSHPULL(PA9) \
        \
        /* USART1_RX */ \
        | PIN_OSPEED_INPUT(PA10) \
        | PIN_MODE_FLOATING(PA10) \
        \
        /* GG_SYSPRES */ \
        | PIN_OSPEED_2M(PA11) \
        | PIN_OTYPE_PUSHPULL(PA11) \
        \
        /* CHG_CE */ \
        | PIN_OSPEED_2M(PA12) \
        | PIN_OTYPE_PUSHPULL(PA12) \
        \
        /* Key 2 */ \
        | PIN_OSPEED_INPUT(PA13) \
        | PIN_MODE_FLOATING(PA13) \
        \
        /* JTCK / SWCLK */ \
        | PIN_OSPEED_INPUT(PA14) \
        | PIN_MODE_FLOATING(PA14) \
        \
        /* Key 1 */ \
        | PIN_MODE_INPUT(PA15) \
        | PIN_MODE_FLOATING(PA15) \
        \
        | 0)

/*
 * Pull GG_SYSPRES low to enable gas gauge,
 * and CHG_CE high to enable charging.
 * Make KEY_RIGHT a pullup, to prevent hardware bug.
 */
#define VAL_GPIOA_ODR (( 0U << 11 ) | ( 1U << 12 ) | (1U << 15))

#define VAL_GPIOB_CRL   ( 0 \
        \
        /* LED strip 1 out */ \
        | PIN_OSPEED_50M(PB0) \
        | PIN_OTYPE_PUSHPULL(PB0) \
        \
        /* LED strip 2 */ \
        | PIN_OSPEED_50M(PB1) \
        | PIN_OTYPE_PUSHPULL(PB1) \
        \
        /* Programming mode */ \
        | PIN_UNUSED(PB2) \
        \
        /* PM_JTDO */ \
        | PIN_OSPEED_2M(PB3) \
        | PIN_OTYPE_OPENDRAIN(PB3) \
        \
        /* Key 0 */ \
        | PIN_OSPEED_INPUT(PB4) \
        | PIN_MODE_FLOATING(PB4) \
        \
        /* Key 3 */ \
        | PIN_OSPEED_INPUT(PB5) \
        | PIN_MODE_FLOATING(PB5) \
        \
        | PIN_UNUSED(PB6) \
        | PIN_UNUSED(PB7) \
        \
        | 0)

#define VAL_GPIOB_CRH   ( 0 \
        \
        /* PM_REFLASH_ALRT */ \
        | PIN_UNUSED(PB8) \
        \
        /* NC */ \
        | PIN_UNUSED(PB9) \
        \
        /* CHG_SCL */ \
        | PIN_OSPEED_2M(PB10) \
        | PIN_OTYPE_AF_OPENDRAIN(PB10) \
        \
        /* CHG_SDA */ \
        | PIN_OSPEED_2M(PB11) \
        | PIN_OTYPE_AF_OPENDRAIN(PB11) \
        \
        /* Radio RF Power Up */ \
        | PIN_OSPEED_2M(PB12) \
        | PIN_OTYPE_PUSHPULL(PB12) \
        \
        /* Radio Tx enabled */ \
        | PIN_OSPEED_2M(PB13) \
        | PIN_OTYPE_PUSHPULL(PB13) \
        \
        /* Radio Tx/Rx selection */ \
        | PIN_OSPEED_INPUT(PB14) \
        | PIN_MODE_FLOATING(PB14) \
        \
        /* Accelerometer IRQ2 */ \
        | PIN_OSPEED_INPUT(PB15) \
        | PIN_MODE_FLOATING(PB15) \
        \
        | 0 )

/* Turn on radio RF, and enable TX */
#define VAL_GPIOB_ODR ((1U << 13) | (1U << 12))

#define VAL_GPIOC_CRL   ( 0 \
        | PIN_NOTPRESENT(PC0) \
        | PIN_NOTPRESENT(PC1) \
        | PIN_NOTPRESENT(PC2) \
        | PIN_NOTPRESENT(PC3) \
        | PIN_NOTPRESENT(PC4) \
        | PIN_NOTPRESENT(PC5) \
        | PIN_NOTPRESENT(PC6) \
        | PIN_NOTPRESENT(PC7) \
        )

#define VAL_GPIOC_CRH   ( 0 \
        | PIN_NOTPRESENT(PC8) \
        | PIN_NOTPRESENT(PC9) \
        | PIN_NOTPRESENT(PC10) \
        | PIN_NOTPRESENT(PC11) \
        | PIN_NOTPRESENT(PC12) \
        \
        /* TAMPER-RTC */ \
        | PIN_UNUSED(PC13) \
        \
        /* PM_OSC32_IN */ \
        | PIN_OSPEED_INPUT(PC14) \
        | PIN_MODE_INPUT(PC14) \
        \
        /* PM_OSC32_OUT */ \
        | PIN_OSPEED_INPUT(PC15) \
        | PIN_MODE_INPUT(PC15) \
        \
        | 0)

#define VAL_GPIOC_ODR 0


#define VAL_GPIOD_CRL   ( 0 \
        | PIN_UNUSED(PD0) \
        | PIN_UNUSED(PD1) \
        | PIN_NOTPRESENT(PD2) \
        | PIN_NOTPRESENT(PD3) \
        | PIN_NOTPRESENT(PD4) \
        | PIN_NOTPRESENT(PD5) \
        | PIN_NOTPRESENT(PD6) \
        | PIN_NOTPRESENT(PD7) \
        | 0)

#define VAL_GPIOD_CRH   ( 0 \
        | PIN_NOTPRESENT(PD8) \
        | PIN_NOTPRESENT(PD9) \
        | PIN_NOTPRESENT(PD10) \
        | PIN_NOTPRESENT(PD11) \
        | PIN_NOTPRESENT(PD12) \
        | PIN_NOTPRESENT(PD13) \
        | PIN_NOTPRESENT(PD14) \
        | PIN_NOTPRESENT(PD15) \
        | 0)

#define VAL_GPIOD_ODR 0

#define VAL_GPIOE_CRL   ( 0 \
        | PIN_NOTPRESENT(PE0) \
        | PIN_NOTPRESENT(PE1) \
        | PIN_NOTPRESENT(PE2) \
        | PIN_NOTPRESENT(PE3) \
        | PIN_NOTPRESENT(PE4) \
        | PIN_NOTPRESENT(PE5) \
        | PIN_NOTPRESENT(PE6) \
        | PIN_NOTPRESENT(PE7) \
        | 0)
#define VAL_GPIOE_CRH   ( 0 \
        | PIN_NOTPRESENT(PE8) \
        | PIN_NOTPRESENT(PE9) \
        | PIN_NOTPRESENT(PE10) \
        | PIN_NOTPRESENT(PE11) \
        | PIN_NOTPRESENT(PE12) \
        | PIN_NOTPRESENT(PE13) \
        | PIN_NOTPRESENT(PE14) \
        | PIN_NOTPRESENT(PE15) \
        | 0)
#define VAL_GPIOE_ODR 0


#if !defined(_FROM_ASM_)
#ifdef __cplusplus
extern "C" {
#endif
  void boardInit(void);
#ifdef __cplusplus
}
#endif
#endif /* _FROM_ASM_ */

#endif /* _BOARD_H_ */
