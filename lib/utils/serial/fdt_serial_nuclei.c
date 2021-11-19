/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) Nuclei Corporation or its affiliates.
 *
 * Authors:
 *   hqfang <hqfang@nucleisys.com>
 */

#include <sbi_utils/fdt/fdt_helper.h>
#include <sbi_utils/serial/fdt_serial.h>
#include <sbi_utils/serial/nuclei-uart.h>

static int serial_nuclei_init(void *fdt, int nodeoff,
				const struct fdt_match *match)
{
	int rc;
	struct platform_uart_data uart;

	rc = fdt_parse_nuclei_uart_node(fdt, nodeoff, &uart);
	if (rc)
		return rc;

	return nuclei_uart_init(uart.addr, uart.freq, uart.baud);
}

static const struct fdt_match serial_nuclei_match[] = {
	{ .compatible = "nuclei,uart0" },
	{ },
};

struct fdt_serial fdt_serial_nuclei = {
	.match_table = serial_nuclei_match,
	.init = serial_nuclei_init,
	.getc = nuclei_uart_getc,
	.putc = nuclei_uart_putc
};
