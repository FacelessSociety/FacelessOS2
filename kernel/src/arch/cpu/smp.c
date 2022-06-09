/*
 *  MIT License
 *
 *  Copyright (c) 2022 FacelessSociety, Ian Marco Moffett
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in all
 *  copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *  SOFTWARE.
 */

#include <arch/cpu/smp.h>
#include <arch/pic/lapic.h>
#include <arch/timer/pit.h>
#include <power/acpi/acpi.h>
#include <debug/log.h>

static uint8_t ncores = 0;


uint8_t cpu_detect_cores(void) {
    if (ncores > 0) return ncores;

    uint8_t* p = (uint8_t*)(madt + 1);
    uint8_t* end = (uint8_t*)(madt + madt->header.length / 4);

    while (p < end) {
        apic_header_t* header = (apic_header_t*)p;
        if (header->length == 0) break;
        
        if (header->type == APIC_TYPE_LOCAL_APIC) {
            ++ncores;
        };

        p += header->length;
    }

    return ncores;
}


void cpu_wakeup_cores(void) {
    log("Waking up all cores.\n");
    log("WAKE UP CORES!!!!!!!\n");

    // Send INIT to all cores except BSP.
    for (uint32_t i = 0; i < ncores; ++i) {
        uint32_t apic_id = acpi_cpuids[i];

        if (apic_id != lapic_get_id()) {
            lapic_send_init(apic_id);
        }

        pit_sleep(50);

        // Send startup to all cores except BSP.

        for (uint32_t i = 0; i < ncores; ++i) {
            uint32_t apic_id = acpi_cpuids[i];

            if (apic_id != lapic_get_id()) {
                lapic_send_startup(apic_id, 0x8);
            }
        }

        // Wait for all cores to be active.

        pit_sleep(50);
    }
}
