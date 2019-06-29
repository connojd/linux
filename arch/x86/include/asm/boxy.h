/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _ASM_X86_BOXY_H
#define _ASM_X86_BOXY_H

#include <linux/init.h>
#include <linux/types.h>

#include <asm/cpufeature.h>
#include <asm/hypervisor.h>
#include <asm/paravirt.h>
#include <asm/msr.h>
#include <asm/processor.h>

#define SUCCESS 0
#define FAILURE 0xFFFFFFFFFFFFFFFF
#define SUSPEND 0xFFFFFFFFFFFFFFFE

#define status_t int64_t

/* -------------------------------------------------------------------------- */
/* VMCall Prototypes                                                          */
/* -------------------------------------------------------------------------- */

uint64_t asm_vmcall(uint64_t r1, uint64_t r2, uint64_t r3, uint64_t r4);
uint64_t asm_vmcall1(void *r1);
uint64_t asm_vmcall2(void *r1, void *r2);
uint64_t asm_vmcall3(void *r1, void *r2, void *r3);
uint64_t asm_vmcall4(void *r1, void *r2, void *r3, void *r4);

/* -------------------------------------------------------------------------- */
/* CPUID                                                                      */
/* -------------------------------------------------------------------------- */

#define CPUID_BAREFLANK_SYN 0x4BF00000
#define CPUID_BAREFLANK_ACK 0x4BF00001

/* -------------------------------------------------------------------------- */
/* Virtual IRQs                                                               */
/* -------------------------------------------------------------------------- */

void boxy_virq_init(void);
void boxy_virq_handler_sym(void);

#define boxy_virq__vclock_event_handler 0xBF00000000000201

#define hypercall_enum_virq_op__set_hypervisor_callback_vector 0xBF10000000000100
#define hypercall_enum_virq_op__get_next_virq 0xBF10000000000101

static inline status_t
hypercall_virq_op__set_hypervisor_callback_vector(uint64_t vector)
{
    return asm_vmcall(
        hypercall_enum_virq_op__set_hypervisor_callback_vector, vector, 0, 0);
}

static inline status_t
hypercall_virq_op__get_next_virq(void)
{
    return asm_vmcall(
        hypercall_enum_virq_op__get_next_virq, 0, 0, 0);
}

/* -------------------------------------------------------------------------- */
/* Virtual Clock                                                              */
/* -------------------------------------------------------------------------- */

void boxy_vclock_init(void);
void boxy_vclock_event_handler(void);

#define hypercall_enum_vclock_op__get_tsc_freq_khz 0xBF11000000000100
#define hypercall_enum_vclock_op__set_next_event 0xBF11000000000102
#define hypercall_enum_vclock_op__reset_host_wallclock 0xBF11000000000103
#define hypercall_enum_vclock_op__set_host_wallclock_rtc 0xBF11000000000104
#define hypercall_enum_vclock_op__set_host_wallclock_tsc 0xBF11000000000105
#define hypercall_enum_vclock_op__set_guest_wallclock_rtc 0xBF11000000000106
#define hypercall_enum_vclock_op__set_guest_wallclock_tsc 0xBF11000000000107
#define hypercall_enum_vclock_op__get_guest_wallclock 0xBF11000000000108

static inline status_t
hypercall_vclock_op__get_tsc_freq_khz(void)
{
    return asm_vmcall(
        hypercall_enum_vclock_op__get_tsc_freq_khz, 0, 0, 0);
}

static inline status_t
hypercall_vclock_op__set_next_event(uint64_t tsc_delta)
{
    return asm_vmcall(
        hypercall_enum_vclock_op__set_next_event, tsc_delta, 0, 0);
}

static inline status_t
hypercall_vclock_op__reset_host_wallclock(void)
{
    return asm_vmcall(
        hypercall_enum_vclock_op__reset_host_wallclock, 0, 0, 0
    );
}

static inline status_t
hypercall_vclock_op__set_guest_wallclock_rtc(void)
{
    return asm_vmcall(
        hypercall_enum_vclock_op__set_guest_wallclock_rtc, 0, 0, 0
    );
}

static inline status_t
hypercall_vclock_op__set_guest_wallclock_tsc(void)
{
    return asm_vmcall(
        hypercall_enum_vclock_op__set_guest_wallclock_tsc, 0, 0, 0
    );
}

static inline status_t
hypercall_vclock_op__get_guest_wallclock(
    int64_t *sec, long *nsec, uint64_t *tsc)
{
    uint64_t op = hypercall_enum_vclock_op__get_guest_wallclock;

    if (sec == 0 || nsec == 0 || tsc == 0) {
        return FAILURE;
    }

    return asm_vmcall4(
        &op, sec, nsec, tsc);
}

/* -------------------------------------------------------------------------- */
/* Quirks                                                                     */
/* -------------------------------------------------------------------------- */

void boxy_apic_quirk(unsigned int early);

#endif
