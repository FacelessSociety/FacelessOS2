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

acpi_madt_t* madt = NULL;
static acpi_rsdt_t* rsdt = NULL;
static int rsdt_entries = 0;
static uint8_t using_madt = 1;
static uint8_t n_cores = 0;


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
        log(KINFO "MADT not found! Using legacy mode..\n");
        using_madt = 0;
    } else {
        log(KINFO "MADT found!\n");
    }

    uint32_t cores = cpu_detect_cores();

    if (cores > 1) {
        log(KINFO "%d CPU cores detected.\n", cores);
    } else {
        log(KINFO "%d CPU core detected.\n", cores);
    }
}
