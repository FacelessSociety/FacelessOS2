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

#include <arch/pic/ioapic.h>
#include <power/acpi/acpi.h>
#include <debug/log.h>

// MMIO IO APIC regs.
#define IOREGSEL                        0x00
#define IOWIN                           0x10

// Standard IO APIC registers.
#define IOAPICID                        0x00
#define IOAPICVER                       0x01
#define IOAPICARB                       0x02
#define IOREDTBL                        0x10


static void ioapic_write(uint8_t reg, uint32_t val) {
    // Select the register with IOREGSEL.
    *(volatile uint32_t*)(io_apic_ptr + IOREGSEL) = reg;

    // Now send the data.
    *(volatile uint32_t*)(io_apic_ptr + IOWIN) = val;
}


static uint32_t ioapic_read(uint8_t reg) {
    *(volatile uint32_t*)(io_apic_ptr + IOREGSEL);
    return *(volatile uint32_t*)(io_apic_ptr + IOWIN);
}


void ioapic_set_entry(uint8_t entry, uint64_t data) {
    ioapic_write(IOREDTBL + entry * 2, (uint32_t)data);
    ioapic_write(IOREDTBL + entry * 2 + 1, (uint32_t)(data >> 32));
}


void ioapic_init(void) {
    // Fetch number of redirection entries I/O APIC supports.
    uint32_t in_data = ioapic_read(IOAPICVER);
    uint32_t count = ((in_data >> 16) & 0xFF) + 1;
    log(KINFO "I/O APIC pins: %d\n", count);

    // Disable all redirection entries.
    for (uint32_t i = 0; i < count; ++i) {
        ioapic_set_entry(i, 1 << 16);
    }
}
