/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2019 Western Digital Corporation or its affiliates.
 *
 * Authors:
 *   Anup Patel <anup.patel@wdc.com>
 */

#include <sbi/sbi_ecall.h>
#include <sbi/sbi_ecall_interface.h>
#include <sbi/sbi_error.h>
#include <sbi/sbi_trap.h>

/* Note(DD): used for debug penglai enclave */
#if 0
#include <sbi/sbi_console.h>
#include <sbi/riscv_asm.h>
#include <sbi/riscv_encoding.h>
#endif
u16 sbi_ecall_version_major(void)
{
	return SBI_ECALL_VERSION_MAJOR;
}

u16 sbi_ecall_version_minor(void)
{
	return SBI_ECALL_VERSION_MINOR;
}

static unsigned long ecall_impid = SBI_OPENSBI_IMPID;

unsigned long sbi_ecall_get_impid(void)
{
	return ecall_impid;
}

void sbi_ecall_set_impid(unsigned long impid)
{
	ecall_impid = impid;
}

static SBI_LIST_HEAD(ecall_exts_list);

struct sbi_ecall_extension *sbi_ecall_find_extension(unsigned long extid)
{
	struct sbi_ecall_extension *t, *ret = NULL;

	sbi_list_for_each_entry(t, &ecall_exts_list, head) {
		if (t->extid_start <= extid && extid <= t->extid_end) {
			ret = t;
			break;
		}
	}

	return ret;
}

int sbi_ecall_register_extension(struct sbi_ecall_extension *ext)
{
	struct sbi_ecall_extension *t;

	if (!ext || (ext->extid_end < ext->extid_start) || !ext->handle)
		return SBI_EINVAL;

	sbi_list_for_each_entry(t, &ecall_exts_list, head) {
		unsigned long start = t->extid_start;
		unsigned long end = t->extid_end;
		if (end < ext->extid_start || ext->extid_end < start)
			/* no overlap */;
		else
			return SBI_EINVAL;
	}

	SBI_INIT_LIST_HEAD(&ext->head);
	sbi_list_add_tail(&ext->head, &ecall_exts_list);

	return 0;
}

void sbi_ecall_unregister_extension(struct sbi_ecall_extension *ext)
{
	bool found = FALSE;
	struct sbi_ecall_extension *t;

	if (!ext)
		return;

	sbi_list_for_each_entry(t, &ecall_exts_list, head) {
		if (t == ext) {
			found = TRUE;
			break;
		}
	}

	if (found)
		sbi_list_del_init(&ext->head);
}

/* Note(DD): used to debug penglai enclave */
#if 0
static inline int valid_pte(unsigned long pte){
	return (pte & 0x1);
}
static inline int is_leaf_pt_page(unsigned long pte){
	return ((pte & 0xf) != 0x1);
}

/*
 * level: 0 (root_pt_page), 1 (2nd pt page), 2 (last pt page), 3 (the leaf pte)
 * */
void dd_debug_dump_pt(unsigned long pte, int level)
{
	int i;
	unsigned long * pt_page;

	/* Return if we are @ last pt page */
	if (level==3){ //It's always a valid entry checked by prior func
		sbi_printf("      0x%x\n", pte);
		return;
	}

	for (i=0;i<level;i++)
		sbi_printf("  ");
	sbi_printf("0x%x\n", pte);

	/* Return if we are leaf page */
	if (level!=0 && is_leaf_pt_page(pte))
		return;

	/* Remove the attribute bits and re-align the addr */
	if (level!=0) {// root is handled specially
		pt_page = (unsigned long*) (((unsigned long)pte >> 10ul) << 12ul);
	}
	else {
	pt_page = (unsigned long*) pte;
	}


	for (i=0; i<512; i++){
		if (valid_pte(pt_page[i])){
			dd_debug_dump_pt(pt_page[i], level+1);
		}
	}
}

/*
 * addr: the virtual addr of the target content
 * root_pt: the page table
 * lines: how much lines (8B each) to dump
 * */
void dd_debug_dump_page(unsigned long addr, unsigned long *root_pt, int lines)
{
	/* Get the page first */
	unsigned long vpn_2 = (addr & 0x7ffffffffful) >> 30;
	unsigned long vpn_1 = (addr & 0x3ffffffful) >> 21;
	unsigned long vpn_0 = (addr & 0x1ffffful) >> 12;

	//here, we assume there is no huge page for enclaves now!
	unsigned long pte = root_pt[vpn_2];
	unsigned long * page;
	unsigned long offset;
	int i;

	sbi_printf("[Penglai Monitor@%s] Dump contents from 0x%x using PT:0x%x, lines:%d\n",
			__func__, addr, (unsigned long)root_pt, lines);

	if (!valid_pte(pte) || is_leaf_pt_page(pte)){
		sbi_printf("[Penglai monitor@%s] error! pte1(0x%x) is huge!\n", __func__,
				pte);
		return;
	}

	root_pt = (unsigned long*) (((unsigned long)pte >> 10ul) << 12ul);
	pte = root_pt[vpn_1];
	if (!valid_pte(pte) || is_leaf_pt_page(pte)){
		sbi_printf("[Penglai monitor@%s] error! pte2(0x%x) is huge!\n", __func__,
				pte);
		return;
	}

	root_pt = (unsigned long*) (((unsigned long)pte >> 10ul) << 12ul);
	pte = root_pt[vpn_0];
	if (!valid_pte(pte)){
		sbi_printf("[Penglai monitor@%s] error! pte3(0x%x) is not valid!\n", __func__,
				pte);
		return;
	}

	page = (unsigned long*) (((unsigned long)pte >> 10ul) << 12ul);
	offset = addr & 0xfff;

	/* Now read the content from the page */
	for (i=0; i<lines; i++){
		sbi_printf("\t0x%lx\n", page[offset/8 + i]);
	}

}
#endif

int sbi_ecall_handler(struct sbi_trap_regs *regs)
{
	int ret = 0;
	struct sbi_ecall_extension *ext;
	unsigned long extension_id = regs->a7;
	unsigned long func_id = regs->a6;
	struct sbi_trap_info trap = {0};
	unsigned long out_val = 0;
	bool is_0_1_spec = 0;
	unsigned long args[6];

	args[0] = regs->a0;
	args[1] = regs->a1;
	args[2] = regs->a2;
	args[3] = regs->a3;
	args[4] = regs->a4;
	args[5] = regs->a5;

	if (extension_id == SBI_EXT_BASE && func_id > 80){
		/* FIXME(DD): hacking, when extension id is base, put regs into last args
		 * currently this reg will not be used by any base functions
		 * */
		args[5] = (unsigned long) regs;
		regs->mepc += 4;
	}

	ext = sbi_ecall_find_extension(extension_id);
	if (ext && ext->handle) {
		ret = ext->handle(extension_id, func_id,
				  args, &out_val, &trap);
		if (extension_id >= SBI_EXT_0_1_SET_TIMER &&
		    extension_id <= SBI_EXT_0_1_SHUTDOWN)
			is_0_1_spec = 1;
	} else {
		ret = SBI_ENOTSUPP;
	}

	if (ret == SBI_ETRAP) {
		trap.epc = regs->mepc;
		sbi_trap_redirect(regs, &trap);
	} else if (extension_id == SBI_EXT_BASE && func_id > 80){
		/* Do nothing as the regs mepc and a0 is set by the SM */
		regs->a0 = ret;
		if (!is_0_1_spec)
			regs->a1 = out_val;
		/* Note(DD): for debug */
		#if 0
		sbi_printf("[Penglai Monitor@%s] mepc:0x%x, a0:0x%x, mstatus:0x%x\n",
				__func__, regs->mepc, regs->a0, regs->mstatus);
		/* Dump PT here */
		if (func_id == 97) {//run
			sbi_printf("[Penglai Monitor@%s] Dump pt @ 0x%x\n",
					__func__, csr_read(CSR_SATP));
				dd_debug_dump_pt(((csr_read(CSR_SATP) & 0xffffffffffful)<<12ul), 0);
		}

		dd_debug_dump_page(regs->mepc, (unsigned long*)((csr_read(CSR_SATP) & 0xffffffffffful)<<12ul), 8);
		dd_debug_dump_page(regs->sp, (unsigned long*)((csr_read(CSR_SATP) & 0xffffffffffful)<<12ul), 2);
		#endif
	} else {
		/*
		 * This function should return non-zero value only in case of
		 * fatal error. However, there is no good way to distinguish
		 * between a fatal and non-fatal errors yet. That's why we treat
		 * every return value except ETRAP as non-fatal and just return
		 * accordingly for now. Once fatal errors are defined, that
		 * case should be handled differently.
		 */
		regs->mepc += 4;
		regs->a0 = ret;
		if (!is_0_1_spec)
			regs->a1 = out_val;
	}

	return 0;
}

int sbi_ecall_init(void)
{
	int ret;

	/* The order of below registrations is performance optimized */
	ret = sbi_ecall_register_extension(&ecall_time);
	if (ret)
		return ret;
	ret = sbi_ecall_register_extension(&ecall_rfence);
	if (ret)
		return ret;
	ret = sbi_ecall_register_extension(&ecall_ipi);
	if (ret)
		return ret;
	ret = sbi_ecall_register_extension(&ecall_base);
	if (ret)
		return ret;
	ret = sbi_ecall_register_extension(&ecall_hsm);
	if (ret)
		return ret;
	ret = sbi_ecall_register_extension(&ecall_legacy);
	if (ret)
		return ret;
	ret = sbi_ecall_register_extension(&ecall_vendor);
	if (ret)
		return ret;

	return 0;
}
