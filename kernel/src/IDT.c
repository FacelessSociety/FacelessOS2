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

#include <arch/interrupts/IDT.h>
#include <arch/interrupts/exceptions.h>

#define TRAP_GATE_FLAGS 0x8F
#define INT_GATE_FLAGS 0x8E
#define IDT_INT_GATE_USER 0xEE

__attribute__((section(".data"))) static struct IDTGateDescriptor idt[256];
__attribute__((section(".data"))) static struct IDTR idtr;


static void set_idt_desc(uint8_t vector, void* isr, int flags) { 
    uint64_t addr = (uint64_t)isr;
    idt[vector].isr_low = addr & 0xFFFF;
    idt[vector].cs = 0x28;
    idt[vector].ist = 0x0;
    idt[vector].reserved = 0x0;
    idt[vector].attr = 0xF;
    idt[vector].reserved1 = 0x0;
    idt[vector].dpl = 0;
    idt[vector].p = 1;
    idt[vector].isr_middle = (addr >> 16) & 0xFFFF;
    idt[vector].isr_high = (addr >> 32) & 0xFFFFFFFF;
    idt[vector].reserved2 = 0x0;
}


__attribute__((section(".data"))) static void(*exceptions[])(void) = {
    div0_handler,
    debug_exception,
    general_protection_fault,
    breakpoint_exception,
    overflow_exception,
    boundrange_exceeded_exception,
    invalid_opcode_exception,
    dev_not_avail_exception,
    double_fault,
    general_protection_fault,
    invalid_tss_exception,
    segment_not_present,
    stack_segment_fault,
    general_protection_fault,
    page_fault
};


static void setup_exceptions(void) {
    for (uint16_t vector = 0; vector <= 0xE; ++vector) {
        set_idt_desc(vector, exceptions[vector], TRAP_GATE_FLAGS);
    }
    
    set_idt_desc(0, div0_handler, TRAP_GATE_FLAGS);
}


void idt_install(void) {
    setup_exceptions();
    idtr.limit = sizeof(struct IDTGateDescriptor) * 255;
    idtr.base = (uint64_t)&idt;
    __asm__ __volatile__("lidt %0" :: "m" (idtr));
}
