/**
 * Copyright (C) 2019 Assured Information Security, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <asm/boxy.h>
#include <asm/desc.h>
#include <asm/irq_regs.h>
#include <asm/irq_vectors.h>
#include <linux/interrupt.h>

__visible void __irq_entry boxy_virq_handler(struct pt_regs *regs)
{
	uint64_t virq;

    struct pt_regs *old_regs = set_irq_regs(regs);
	irq_enter();

	virq = hypercall_virq_op__get_next_virq();
    if (virq == FAILURE) {
        pr_err("hypercall_virq_op__get_next_virq failed");
        BUG();
    }

	switch(virq) {
		case boxy_virq__vclock_event_handler:
			boxy_vclock_event_handler();
			break;

		default:
			pr_err("unknown virq");
			BUG();
	}

	irq_exit();
	set_irq_regs(old_regs);
}

void __init boxy_virq_init(void)
{
	uint64_t ret;

	ret = hypercall_virq_op__set_hypervisor_callback_vector(
		HYPERVISOR_CALLBACK_VECTOR);
    if (ret != SUCCESS) {
        pr_err("hypercall_virq_op__set_hypervisor_callback_vector failed");
        BUG();
    }

	alloc_intr_gate(HYPERVISOR_CALLBACK_VECTOR, boxy_virq_handler_sym);
}
