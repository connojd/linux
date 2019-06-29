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
#include <asm/apic.h>
#include <linux/init.h>

/**
 * Quirk Notes
 *
 * Like Jailhouse, we require an x2apic, and currently, if we don't set the
 * code below, the kernel will crash as it attempts to read an x2apic
 * register before the apic variable is set. The code below ensures that
 * we end up with symmetric IO mode with a physical x2apic while also setting
 * the apic variable so that the kernel doesn't segfault.
 *
 * If you enable ACPI, this bug will go away as ACPI happens to call the
 * default_acpi_madt_oem_check() function which sets the apic variable in the
 * kernel before the init_apic_mappings() function is called. The crash
 * occurs here:
 * https://elixir.bootlin.com/linux/latest/source/arch/x86/kernel/apic/apic.c#L1969
 *
 * Since we are calling the default_acpi_madt_oem_check() function manually,
 * we need to ensure that the apic is configured properly for Boxy guests
 * which includes a physical x2apic and symmetric IO mode.
 */

void __init boxy_apic_quirk(unsigned int early)
{
	x2apic_phys = 1;
	smp_found_config = 1;

	default_acpi_madt_oem_check("", "");
}
