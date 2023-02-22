/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2019 Western Digital Corporation or its affiliates.
 * Modified to support Nuclei UART
 *
 * Authors:
 *   Anup Patel <anup.patel@wdc.com>
 */

#ifndef __SERIAL_NUCLEI_UART_H__
#define __SERIAL_NUCLEI_UART_H__

#include <sbi/sbi_types.h>

int nuclei_uart_init(unsigned long base, u32 in_freq, u32 baudrate);

#endif
