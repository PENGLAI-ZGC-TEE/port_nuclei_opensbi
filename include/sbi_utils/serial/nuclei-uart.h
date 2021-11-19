/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) Nuclei Corporation or its affiliates.
 *
 * Authors:
 *   hqfang <hqfang@nucleisys.com>
 */

#ifndef __SERIAL_NUCLEI_UART_H__
#define __SERIAL_NUCLEI_UART_H__

#include <sbi/sbi_types.h>

void nuclei_uart_putc(char ch);

int nuclei_uart_getc(void);

int nuclei_uart_init(unsigned long base, u32 in_freq, u32 baudrate);

#endif
