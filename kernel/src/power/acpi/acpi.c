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

#include <power/acpi/acpi.h>
#include <libkern/tag.h>
#include <libkern/panic.h>
#include <libkern/string.h>
#include <debug/log.h>
#include <arch/cpu/smp.h>
#include <arch/pic/lapic.h>

#define MAX_NEEDED_CORES 16


acpi_madt_t* madt = NULL;
static acpi_rsdt_t* rsdt = NULL;
static int rsdt_entries = 0;
static uint8_t n_cores = 0;
uint8_t* local_apic_addr = NULL;

__attribute__((section(".data"))) uint8_t acpi_cpuids[MAX_NEEDED_CORES];
void* io_apic_ptr = 0;


static uint8_t rsdt_is_valid(void) {
    if (rsdt == NULL) return 0;

    uint8_t sum = 0;

    for (int i = 0; i < rsdt->header.length; ++i) {
        sum += ((char*)&rsdt->header)[i];
    }

    return sum % 0x100 == 0;
}


static acpi_madt_t* get_madt(void) {
    for (int i = 0; i < rsdt_entries; ++i) {
        acpi_madt_t* madt = (acpi_madt_t*)(uint64_t)rsdt->tables[i];

        if (strncmp(madt->header.signature, "APIC", 4)) {
            return madt;
        }
    }

    // MADT not found.
    return NULL;
}



static void parse_madt(void) {
    uint8_t* p = (uint8_t*)(madt + 1);
    uint8_t* end = (uint8_t*)(madt + madt->header.length);

    // Index for the acpi_cpuids array.
    uint8_t cpuid_idx = 0;
    local_apic_addr = (uint8_t*)(uint64_t)madt->lapic_addr;

    while (p < end) {
        apic_header_t* header = (apic_header_t*)p;
        if (header->length == 0) break;
        switch (header->type) {
            case APIC_TYPE_LOCAL_APIC:
                // We don't need any more cores.
                if (cpuid_idx > MAX_NEEDED_CORES) break;

                local_apic_t* lapic = (local_apic_t*)p;
                // Print out some information.
                log(DBG_PURPLE "\n\n---- CPU core found ----\n");
                log(DBG_YELLOW "<core_info>\n");
                log(DBG_CYAN "    Processor ID: %d\n", lapic->processor_id);
                log(DBG_CYAN "    APIC ID: %d\n", lapic->apic_id);
                log(DBG_CYAN "    Flags: %x\n", lapic->flags);
                log(DBG_YELLOW "</core_info>\n\n");
                // Put cpuid into array.
                acpi_cpuids[cpuid_idx++] = lapic->processor_id;
                break;
            case APIC_TYPE_IO_APIC:
                // Print some info.
                io_apic_t* io_apic = (io_apic_t*)p;
                log(DBG_PURPLE "\n\n---- I/O APIC found ----\n");
                log(DBG_YELLOW "<apic_info>\n");
                log(DBG_CYAN "    I/O APIC ID: %d\n", io_apic->io_apic_id);
                log(DBG_CYAN "    I/O APIC address: %x\n", io_apic->io_apic_addr);
                log(DBG_CYAN "    Global system interrupt base: %d\n", io_apic->global_system_interrupt_base);
                log(DBG_YELLOW "</apic_info>\n\n");
                io_apic_ptr = (void*)(uint64_t)io_apic->io_apic_addr;
                break;
            case APIC_TYPE_INTERRUPT_OVERRIDE:
                // Print some info.
                apic_interrupt_override_t* aio = (apic_interrupt_override_t*)p;
                log(DBG_PURPLE "\n\n---- Interrupt Override found ----\n");
                log(DBG_YELLOW "<apic_info>\n");
                log(DBG_CYAN "    Bus: %d\n", aio->bus);
                log(DBG_CYAN "    Source: %d\n", aio->source);
                log(DBG_CYAN "    Interrupt: %x\n", aio->interrupt);
                log(DBG_CYAN "    Flags: %x\n", aio->flags);
                log(DBG_YELLOW "</apic_info>\n\n");
                break;
        }

        p += header->length;
    }
}


void acpi_init(struct stivale2_struct* ss) {
    struct stivale2_struct_tag_rsdp* rsdp_tg = get_tag(ss, STIVALE2_STRUCT_TAG_RSDP_ID);
    acpi_rsdp_t* rsdp = (acpi_rsdp_t*)rsdp_tg->rsdp;
    rsdt = (acpi_rsdt_t*)(uint64_t)rsdp->rsdtaddr;

    // Check if RSDT is valid.
    if (!(rsdt_is_valid())) {
        panic("RSDT checksum invalid!");
    } else {
        log(KINFO "RSDT checkum is valid.\n");
    }

    rsdt_entries = (rsdt->header.length - sizeof(rsdt->header)) / 4;

    // Locate MADT.
    if ((madt = get_madt()) == NULL) {
        panic("Could not locate MADT!");
    } else {
        log(KINFO "MADT found!\n");
    }

    parse_madt();
    init_lapic();
    log(KINFO "LAPIC on BSP all setup.\n");

    uint32_t cores = cpu_detect_cores();

    if (cores > 1) {
        log(KINFO "%d CPU cores detected.\n", cores);
    } else {
        log(KINFO "%d CPU core detected.\n", cores);
    }
}
