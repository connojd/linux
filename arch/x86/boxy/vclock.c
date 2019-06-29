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
#include <linux/time64.h>
#include <linux/clocksource.h>
#include <linux/clockchips.h>

static uint64_t g_tsc_offset = 0;
static uint64_t g_tsc_freq_khz = 0;

/******************************************************************************/
/* helpers                                                                    */
/******************************************************************************/

static uint64_t mul_div(uint64_t x, uint64_t n, uint64_t d)
{ return ((x / d) * n) + (((x % d) * n) / d); }

static uint64_t tsc_to_nsec(uint64_t tsc)
{ return mul_div(tsc, 1000000, g_tsc_freq_khz); }

/******************************************************************************/
/* clock source                                                               */
/******************************************************************************/

static u64 boxy_clocksource_read(struct clocksource *cs)
{ return rdtsc_ordered() - g_tsc_offset; }

static struct clocksource boxy_clocksource = {
	.name	= "boxy-clocksource",
	.read	= boxy_clocksource_read,
	.rating	= 500,
	.mask	= CLOCKSOURCE_MASK(64),
	.flags	= CLOCK_SOURCE_IS_CONTINUOUS | CLOCK_SOURCE_VALID_FOR_HRES,
    .archdata.vclock_mode = VCLOCK_TSC
};

/******************************************************************************/
/* clock event                                                                */
/******************************************************************************/

static int boxy_set_next_event(
    unsigned long delta, struct clock_event_device *evt)
{
    if (hypercall_vclock_op__set_next_event(delta) != SUCCESS) {
        pr_err("hypercall_vclock_op__set_next_event failed");
        BUG();
    }

	return 0;
}

static struct clock_event_device boxy_clock_event_device = {
	.name			    = "boxy-clock-event-device",
	.features		    = CLOCK_EVT_FEAT_ONESHOT,
	.mult			    = 1,
	.shift			    = 0,
	.rating			    = 500,
	.set_next_event		= boxy_set_next_event,
};

void boxy_vclock_event_handler(void)
{ boxy_clock_event_device.event_handler(&boxy_clock_event_device); }

static void __init boxy_setup_percpu_clockev(void)
{
    boxy_clock_event_device.cpumask = cpumask_of(smp_processor_id());

    clockevents_config_and_register(
        &boxy_clock_event_device, g_tsc_freq_khz * 1000, 0, ~0UL);
}

/******************************************************************************/
/* pv_ops                                                                     */
/******************************************************************************/

static u64 boxy_sched_clock(void)
{ return tsc_to_nsec(boxy_clocksource_read(0)); }

static u64 boxy_steal_clock(int cpu)
{
    /**
     * Note:
     *
     * For now we do not support the steal clock. Timekeeping seems to work
     * fine without it, and implementing this would require not only an
     * additional VMCall on every sched_clock() call, but it would also
     * require the hypervisor to perform time keeping on every exit and
     * entry to account for the time that the VM is actually executing.
     */

    return 0;
}

/******************************************************************************/
/* x86_platform_ops                                                           */
/******************************************************************************/

static unsigned long tsc_freq_khz(void)
{ return g_tsc_freq_khz; }

/******************************************************************************/
/* init functions                                                             */
/******************************************************************************/

static void wallclock_init(void)
{
    if (hypercall_vclock_op__reset_host_wallclock() != SUCCESS) {
        pr_err("hypercall_vclock_op__reset_host_wallclock failed");
        BUG();
    }

    if (hypercall_vclock_op__set_guest_wallclock_rtc() != SUCCESS) {
        pr_err("hypercall_vclock_op__set_guest_wallclock_rtc failed");
        BUG();
    }

    if (hypercall_vclock_op__set_guest_wallclock_tsc() != SUCCESS) {
        pr_err("hypercall_vclock_op__set_guest_wallclock_tsc failed");
        BUG();
    }
}

void __init read_persistent_wall_and_boot_offset(
    struct timespec64 *wall_time, struct timespec64 *boot_offset)
{
    uint64_t ret, tsc;
    struct timespec64 wallclock;

    ret = hypercall_vclock_op__get_guest_wallclock(
        &wallclock.tv_sec, &wallclock.tv_nsec, &tsc);
    if (ret != SUCCESS) {
        pr_err("hypercall_vclock_op__get_wallclock failed");
        BUG();
    }

    *wall_time = wallclock;
	*boot_offset = ns_to_timespec64(tsc_to_nsec(tsc - g_tsc_offset));
}

void __init boxy_vclock_init(void)
{
    g_tsc_freq_khz = hypercall_vclock_op__get_tsc_freq_khz();
    if (g_tsc_freq_khz == FAILURE) {
        pr_err("hypercall_vclock_op__get_tsc_freq_khz failed");
        BUG();
    }

    pv_ops.time.sched_clock = boxy_sched_clock;
    pv_ops.time.steal_clock = boxy_steal_clock;

    x86_init.timers.setup_percpu_clockev = boxy_setup_percpu_clockev;
    x86_init.timers.timer_init = x86_init_noop;
    x86_init.timers.wallclock_init = wallclock_init;

    x86_platform.calibrate_tsc = tsc_freq_khz;
    x86_platform.calibrate_cpu = tsc_freq_khz;

    g_tsc_offset = rdtsc_ordered();
    clocksource_register_khz(&boxy_clocksource, g_tsc_freq_khz);

	setup_force_cpu_cap(X86_FEATURE_NONSTOP_TSC);
	setup_force_cpu_cap(X86_FEATURE_CONSTANT_TSC);
	setup_force_cpu_cap(X86_FEATURE_TSC_RELIABLE);
	setup_force_cpu_cap(X86_FEATURE_TSC_KNOWN_FREQ);
}
