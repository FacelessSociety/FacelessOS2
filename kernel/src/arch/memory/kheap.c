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

#include <arch/memory/kheap.h>
#include <arch/memory/vmm.h>
#include <libkern/panic.h>

#define KHEAP_START 0x500
#define KHEAP_LIMIT 0x500+GB
#define KHEAP_END 0x500+(KHEAP_LIMIT+1)
#define KHEAP_MAGIC 0xCA75101               // CATS LOL


struct __attribute__((packed)) Block {
    uint8_t is_free;
    struct Block* next;
    struct Block* prev;
    size_t size;
    uint32_t magic;
    char data_start[1];                         // First element of data would be data_start + 1.
};


static struct Block* kheap = (struct Block*)KHEAP_START;
static size_t mem_allocated = 0;


void kheap_init(void) {
    map_page((void*)KHEAP_START, PAGE_BIT_P_PRESENT | PAGE_BIT_RW_WRITABLE);
    kheap[0].next = NULL;
    kheap[0].is_free = 1;
    kheap[0].size = 0;
    kheap[0].magic = KHEAP_MAGIC;
}


static struct Block* first_fit(size_t size) {
    map_page((void*)KHEAP_START + mem_allocated, PAGE_BIT_P_PRESENT | PAGE_BIT_RW_WRITABLE);

    // No memory is left!
    if (mem_allocated + size >= KHEAP_LIMIT) {
        return NULL;
    }

    struct Block* current = &kheap[0];
    struct Block* prev = NULL;

    while (current != NULL) {
        if (!(current->is_free)) {
            prev = current;
            current = current->next;
        } else {
            return current;
        }
    }

    // Nope, nobody is getting memory, almost full.
    if ((uint64_t)(prev + sizeof(struct Block)) > KHEAP_END)
        return NULL;

    prev->next = (struct Block*)(KHEAP_START + mem_allocated + size);
    prev->next->magic = KHEAP_MAGIC;
    prev->size = size;
    return prev->next;
}


void* kmalloc(size_t sz) {
    struct Block* hole = first_fit(sz);
    if (hole == NULL) return NULL;

    hole->is_free = 0;
    mem_allocated += sz;
    return hole->data_start;
}


void kfree(void* block) {
    struct Block* blk = block - sizeof(struct Block) + 1;
    if (blk->magic != KHEAP_MAGIC) {
        panic("Invalid free or kernel heap corruption detected..\n");
    }

    blk->is_free = 1;
    mem_allocated -= blk->size;
}
