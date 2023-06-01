/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2023 Nuclei System Technology or its affiliates.
 *
 * Authors:
 *   Huaqi Fang <hqfang@nucleisys.com>
 */

#include <platform_override.h>
#include <sbi/riscv_asm.h>
#include <sbi_utils/fdt/fdt_helper.h>
#include <sbi_utils/fdt/fdt_fixup.h>

static const struct fdt_match nuclei_customsoc_match[] = {
	{ .compatible = "nuclei,customsoc" },
	{ .compatible = "nuclei,custom-soc" },
	{ .compatible = "nuclei,subsystemsoc" },
	{ .compatible = "nuclei,subsystem-soc" },
	{ },
};

static int nuclei_customsoc_final_init(bool cold_boot,
				   const struct fdt_match *match)
{
	if (cold_boot) { // Add cold boot initial steps
	}

	// Check mcfg_info.tee to see whether tee present
	if (csr_read(0xfc2) & 0x1) {
		// Enable U-Mode to access all regions by setting spmpcfg0 and spmpaddr0
		csr_write(0x1a0, 0x1f);
		csr_write(0x1b0, 0xffffffff);
	}

	return 0;
}

const struct platform_override nuclei_customsoc = {
	.match_table = nuclei_customsoc_match,
	.final_init = nuclei_customsoc_final_init,
};