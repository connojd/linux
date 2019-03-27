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
#include <asm/i8259.h>

static uint32_t __init boxy_detect(void)
{
	uint32_t eax;
	uint32_t ignore[3];

	if (!boot_cpu_has(X86_FEATURE_HYPERVISOR))
		return 0;

	cpuid(CPUID_BAREFLANK_SYN, &eax, &ignore[0], &ignore[1], &ignore[2]);

    /**
     * TODO:
     *
     * We need to add a Boxy specific CPUID leaf at 0x40000000 that acks like
     * VMWare and HyperV so that we play nice with nested virtualization.
     * More importantly, right now we are acking with Bareflank and not
     * Boxy, so this code could end up detecting someone elses hypervisor.
     */

	/**
	 * TODO:
	 *
	 * We need to implement versioning to ensure that we are using a guest
	 * that actually knows how to talk to the hypervisor.
	 */

	if (eax == CPUID_BAREFLANK_ACK)
		return 1;

	return 0;
}

static void __init boxy_init_platform(void)
{
    pv_info.name = "Boxy Hypervisor";

	boxy_virq_init();
    boxy_vclock_init();

	x86_init.resources.probe_roms 	 = x86_init_noop;
	x86_init.mpparse.find_smp_config = x86_init_noop;
	x86_init.mpparse.get_smp_config	 = boxy_apic_quirk;
	x86_init.irqs.pre_vector_init 	 = x86_init_noop;
	x86_init.oem.arch_setup 	 	 = x86_init_noop;
	x86_init.oem.banner 			 = x86_init_noop;

	x86_platform.legacy.rtc			 = 0;
	x86_platform.legacy.warm_reset	 = 0;
	x86_platform.legacy.i8042		 = X86_LEGACY_I8042_PLATFORM_ABSENT;

	legacy_pic = &null_legacy_pic;
}

static bool __init boxy_x2apic_available(void)
{ return true; }

const __initconst struct hypervisor_x86 x86_hyper_boxy = {
	.name = "Boxy Hypervisor",
	.detect = boxy_detect,
	.type = X86_HYPER_BOXY,
	.init.init_platform	= boxy_init_platform,
    .init.x2apic_available = boxy_x2apic_available,
};
