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
 

#include <stdint.h>
#include <stddef.h>
#include <libkern/stivale2.h>
#include <libkern/panic.h>
#include <debug/log.h>
#include <arch/memory/gdt.h>
#include <arch/memory/pmm.h>
#include <arch/memory/vmm.h>
#include <arch/memory/kheap.h>
#include <arch/interrupts/IDT.h>
#include <arch/timer/pit.h>
#include <arch/cpu/smp.h>
#include <power/acpi/acpi.h>
#include <proc/thread.h>

#define PIT_FREQ_HZ 100


void* get_tag(struct stivale2_struct* stivale2_struct, uint64_t id) {
    struct stivale2_tag* curTag = (void*)stivale2_struct->tags;

    while (1) {
        if (!(curTag)) {
            // End.
            return NULL;
        }

        if (curTag->identifier == id) {
            return curTag;
        }

        curTag = (void*)curTag->next;
    }
}

static uint8_t stack[8192];
 
static struct stivale2_header_tag_terminal terminal_hdr_tag = {
    .tag = {
        .identifier = STIVALE2_HEADER_TAG_TERMINAL_ID,
        .next = 0
    },
    .flags = 0
};
 
static struct stivale2_header_tag_framebuffer framebuffer_hdr_tag = {
    .tag = {
        .identifier = STIVALE2_HEADER_TAG_FRAMEBUFFER_ID,
        .next = (uint64_t)&terminal_hdr_tag
    },
    .framebuffer_width  = 0,
    .framebuffer_height = 0,
    .framebuffer_bpp    = 0
};
 
__attribute__((section(".stivale2hdr"), used))
static struct stivale2_header stivale_hdr = {
    .entry_point = 0,
    .stack = (uintptr_t)stack + sizeof(stack),
    .flags = (1 << 1) | (1 << 2) | (1 << 3) | (1 << 4),
    .tags = (uintptr_t)&framebuffer_hdr_tag
};


static void init(struct stivale2_struct* ss) {
    struct stivale2_struct_tag_terminal* term_str_tag = get_tag(ss, STIVALE2_STRUCT_TAG_TERMINAL_ID);
    void* term_write_ptr = (void*)term_str_tag->term_write;

    if (term_str_tag == NULL) {
        while (1) {
            __asm__ __volatile__("cli; hlt");
        }
    }

    log_init(term_write_ptr);
    gdt_load();
    log(KINFO "Global Descriptor Table loaded.\n");
    idt_install();
    log(KINFO "Interrupt Descriptor Table loaded.\n");
    pmm_init(ss);
    log(KINFO "Finished setting up PMM.\n");
    vmm_init(ss);
    log(KINFO "Finished setting up VMM.\n");
    pit_init(PIT_FREQ_HZ);
    log(KINFO "PIT setup at %dHz\n", PIT_FREQ_HZ);
    acpi_init(ss);
    log(KINFO "ACPI related stuff has been setup.\n");
    setup_general_interrupts();
    log(KINFO "General interrupts have been setup.\n"); 
    __asm__ __volatile__("sti");
    cpu_wakeup_cores();
    log(KINFO "All CPU cores are now active!\n");
    kheap_init();
    log(KINFO "Heap created.\n");
    init_multithreading();
    
    fork();
    log("A");
}


void _start(struct stivale2_struct* stivale2_struct) { 
    init(stivale2_struct);

    while (1) {
        __asm__ __volatile__("hlt");
    }
}
