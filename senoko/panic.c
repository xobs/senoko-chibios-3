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

#include "ch.h"
#include "uart.h"
#include "hal.h"

#include "senoko.h"

#include "serial_lld.h"
#include "bionic.h"

extern int emerg_printf(const char *fmt, ...);

static void emerg_putc(uint8_t c) {
  USART_TypeDef *u = serialDriver->usart;
  uint16_t sr = u->SR;

  /* Transmission buffer empty.*/
  while (!(sr & USART_SR_TXE))
    sr = u->SR;

  u->DR = c;
}

void emerg_puts(const char *str) {
  unsigned int i;
  for (i = 0; str[i]; i++) {
    if (str[i] == '\n')
      emerg_putc('\r');
    emerg_putc(str[i]);
  }
}

static inline int list_registers(void)
{
  int var;

  emerg_printf("Registers:\n");

  asm volatile ("mov %0, r12":"=r" (var));
  emerg_printf("IP: 0x%.8lx  ", var);

  asm volatile ("mov %0, r13":"=r" (var));
  emerg_printf("SP: 0x%.8lx\n", var);

  asm volatile ("mov %0, r14":"=r" (var));
  emerg_printf("LR: 0x%.8lx  ", var);

  asm volatile ("mov %0, r15":"=r" (var));
  emerg_printf("PC: 0x%.8lx\n", var);

#if 0
  asm volatile ("mrs %0, cpsr":"=r" (var));
  emerg_printf("CPSR: %.8lx  ", var);

  asm volatile ("mrs %0, spsr":"=r" (var));
  emerg_printf("SPSR: %.8lx\n", var);
#endif

  emerg_printf("R0-3:  ");

  asm volatile ("mov %0, r0":"=r" (var));
  emerg_printf("0x%.8lx  ", var);

  asm volatile ("mov %0, r1":"=r" (var));
  emerg_printf("0x%.8lx  ", var);

  asm volatile ("mov %0, r2":"=r" (var));
  emerg_printf("0x%.8lx  ", var);

  asm volatile ("mov %0, r3":"=r" (var));
  emerg_printf("0x%.8lx\n", var);

  emerg_printf("R4-7:  ");

  asm volatile ("mov %0, r4":"=r" (var));
  emerg_printf("0x%.8lx  ", var);

  asm volatile ("mov %0, r5":"=r" (var));
  emerg_printf("0x%.8lx  ", var);

  asm volatile ("mov %0, r6":"=r" (var));
  emerg_printf("0x%.8lx  ", var);

  asm volatile ("mov %0, r7":"=r" (var));
  emerg_printf("0x%.8lx\n", var);

  emerg_printf("R8-11: ");

  asm volatile ("mov %0, r8":"=r" (var));
  emerg_printf("0x%.8lx  ", var);

  asm volatile ("mov %0, r9":"=r" (var));
  emerg_printf("0x%.8lx  ", var);

  asm volatile ("mov %0, r10":"=r" (var));
  emerg_printf("0x%.8lx  ", var);

  asm volatile ("mov %0, r11":"=r" (var));
  emerg_printf("0x%.8lx\n", var);

  return 0;
}

void senokoHandleHalt(const char *reason) {
  static const char *states[] = {CH_STATE_NAMES};
  thread_t *tp;
  emerg_puts("\n\nSystem halt!\n");
  list_registers();
  emerg_printf("Reason: %s\n", reason);
  emerg_puts("Threads:\n");
  emerg_puts("    addr      pc    stack prio refs   state time  name\n");
  tp = ch.rlist.r_newer;
  do {

    emerg_printf("%.8lx %.8lx %.8lx %4lu %4lu %12s  %-10s\n",
      (uint32_t)tp, (uint32_t)tp->p_ctx.r13->lr, (uint32_t)tp->p_ctx.r13,
      (uint32_t)tp->p_prio, (uint32_t)(tp->p_refs - 1),
      states[tp->p_state],
      tp->p_name);

    tp = tp->p_newer;
    if (tp == (thread_t *)&ch.rlist)
      tp = NULL;
  } while (tp != NULL);

  tp = chThdGetSelfX();
  uint32_t *stack;
  asm volatile ("mov %0, r13":"=r" (stack));
  emerg_printf("Some stack (%.8lx - %.8lx):\n",
      (uint32_t)stack, (uint32_t)tp);

  int i;
  for (i = 0; i < 32; i++) {
    emerg_printf("0x%.8lx ", stack[i]);
    if (!((i - 3) & 3))
      emerg_printf("\n");
  }

  emerg_printf("System will reboot now\n");
  while(1);
}
