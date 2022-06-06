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
#include <arch/memory/pmm.h>
#include <arch/memory/mem.h>
#include <libkern/tag.h>
#include <debug/log.h>

typedef uint64_t BITMAP;


static uint64_t highest_frame_top = 0;
static uint64_t max_frames = 0;                 // Max frames that can be allocated.
static uint64_t used_frames = 0;
static struct stivale2_struct_tag_memmap* gmmap;

// Returns 1 if usable memory.
static inline uint8_t is_usable(const struct stivale2_mmap_entry* ENTRY) {
    return ENTRY->type == STIVALE2_MMAP_USABLE && ENTRY->length > 0;
}


void* pmm_alloc(void) {
    for (uint64_t i = 0; i < gmmap->entries; ++i) {
        struct stivale2_mmap_entry* current_entry = &gmmap->memmap[i];
        if (is_usable(current_entry)) {
            // Take this spot.
            uint64_t ret = current_entry->base;
            current_entry->base += 0x1000;
            current_entry->length -= 0x1000;
            return (void*)ret;
        }
    }

    // No memory left!
    return NULL;
}



void* pmm_allocz(void) {
    uint64_t* alloc = pmm_alloc();

    for (uint32_t i = 0; i < 512; ++i) {
        alloc[i] = 0;
    }

    return alloc;
}


void pmm_init(struct stivale2_struct* ss) {
    struct stivale2_struct_tag_memmap* mmap = get_tag(ss, STIVALE2_STRUCT_TAG_MEMMAP_ID);
    gmmap = mmap;

    struct stivale2_mmap_entry* current_entry;
    uint64_t top = 0;                               // Current biggest page.

    log(DBG_YELLOW "\n\n<mmap_start>\n");
    for (uint64_t i = 0; i < mmap->entries; ++i) {
        current_entry = &mmap->memmap[i];
        log(DBG_CYAN "\t%x-%x | %x ", 
                current_entry->base, 
                current_entry->base + current_entry->length, 
                current_entry->length);

        if (current_entry->type != STIVALE2_MMAP_USABLE) {
            // Log the status of the memory and continue.
            log(DBG_RED "[Reserved]\n");
            continue;
        }

        // If we got to this point, the memory is usable.
        log(KINFO "[Usable]\n");

        top = current_entry->base + current_entry->length;

        if (top > highest_frame_top) {
            highest_frame_top = top;
        }

        used_frames = ((highest_frame_top * 1024) / PAGE_SIZE);
        max_frames = used_frames;
    }


    log(DBG_YELLOW "</mmap_end>\n\n");
}
