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

#include <arch/memory/vmm.h>
#include <arch/memory/pmm.h>
#include <arch/memory/kheap.h>
#include <libkern/string.h>
#include <debug/log.h>

#define PAGE_SIZE 0x1000

__attribute__((aligned(0x1000))) struct MappingTable {
    uint64_t entries[512];
} *pml4 = NULL;


void map_page(void* logical, unsigned int flags) {
    uint64_t addr = (uint64_t)logical;
    flags |= PAGE_BIT_P_PRESENT;            // Make sure it is present.

    // Get indices from logical address.
    int pml4_idx = (addr >> 39) & 0x1FF;
    int pdpt_idx = (addr >> 30) & 0x1FF;
    int pd_idx = (addr >> 21) & 0x1FF;
    int pt_idx = (addr >> 12) & 0x1FF;

    if (!(pml4->entries[pml4_idx] & PAGE_BIT_P_PRESENT)) {
        // We did not define a PDPT for this entry in PML4.
        uint64_t pdpt = (uint64_t)pmm_allocz();
        memzero((void*)pdpt, PAGE_SIZE);
        pml4->entries[pml4_idx] = (pdpt & PAGE_ADDR_MASK) | flags;
        map_page((void*)pdpt, flags);
    }

    struct MappingTable* pdpt = (struct MappingTable*)(pml4->entries[pml4_idx] & PAGE_ADDR_MASK);

    if (!(pdpt->entries[pdpt_idx] & PAGE_BIT_P_PRESENT)) {
        // We did not define PDT for this PDPT entry, so allocate a page for the PDT.
        uint64_t pdt = (uint64_t)pmm_allocz();
        memzero((void*)pdt, PAGE_SIZE);
        pdpt->entries[pdpt_idx] = (pdt & PAGE_ADDR_MASK) | flags;
        map_page((void*)pdt, flags);
    }


    struct MappingTable* pdt = (struct MappingTable*)(pdpt->entries[pdpt_idx] & PAGE_ADDR_MASK);

    if (!(pdt->entries[pd_idx] & PAGE_BIT_P_PRESENT)) {
        // We did not define a PT for this PDT entry, so allocate a page for the PT.
        uint64_t pt = (uint64_t)pmm_allocz();
        memzero((void*)pt, PAGE_SIZE);
        pdt->entries[pd_idx] = (pt & PAGE_ADDR_MASK) | flags;
        map_page((void*)pt, flags);
    }

    struct MappingTable* pt = (struct MappingTable*)(pdt->entries[pd_idx] & PAGE_ADDR_MASK);

    if (!(pt->entries[pt_idx] & PAGE_BIT_P_PRESENT)) {
        pt->entries[pt_idx] = (addr & PAGE_ADDR_MASK) | flags;
    }
}


uint8_t unmap_page(void* logical) {
    uint64_t addr = (uint64_t)logical;
    if (addr % 0x1000 != 0) return 0;           // Return 0 (false) if address is not page aligned.

    // Get indices from logical address.
    int pml4_idx = (addr >> 39) & 0x1FF;
    int pdpt_idx = (addr >> 30) & 0x1FF;
    int pd_idx = (addr >> 21) & 0x1FF;
    int pt_idx = (addr >> 12) & 0x1FF;

    struct MappingTable* pdpt = (struct MappingTable*)(pml4->entries[pml4_idx] & PAGE_ADDR_MASK);
    struct MappingTable* pdt = (struct MappingTable*)(pdpt->entries[pdpt_idx] & PAGE_ADDR_MASK);
    struct MappingTable* pt = (struct MappingTable*)(pdt->entries[pd_idx] & PAGE_ADDR_MASK);

    if (pt->entries[pt_idx] & PAGE_BIT_P_PRESENT) {
        pt->entries[pt_idx] = ((addr & PAGE_ADDR_MASK) & ~(PAGE_BIT_P_PRESENT));        // Just unset the PRESENT bit.
        __asm__ __volatile__("invlpg (%0)" : : "r" (logical));
    }

    return 1;
}


void* mkpml4(void) {
    struct MappingTable* alloc = kmalloc(sizeof(struct MappingTable));

    strncpy((void*)alloc, (void*)pml4, PAGE_SIZE);

    __asm__ __volatile__("mov %%cr3, %0" : "=r" (alloc)); 
    return alloc;
}


void load_pml4(void* pml4) {
    __asm__ __volatile__("mov %0, %%cr3" :: "r" (pml4));
}


void vmm_init(struct stivale2_struct* ss) {
    pml4 = pmm_allocz();

    // Copy CR3 into our PML4.
    __asm__ __volatile__("mov %%cr3, %0" : "=r" (pml4)); 

    // Unmap the NULL address.
    unmap_page(0x0);

    // Put the PML4 back into CR3.
    __asm__ __volatile__("mov %0, %%cr3" :: "r" (pml4));
}
