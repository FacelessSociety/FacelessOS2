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


#ifndef VMM_H
#define VMM_H

#include <libkern/stivale2.h>
#include <libkern/tag.h>
#include <stdint.h>

#define PAGE_BIT_P_PRESENT (1 << 0)
#define PAGE_BIT_RW_WRITABLE (1 << 1)
#define PAGE_BIT_US_USER (1 << 2)
#define PAGE_XD_NX (1 << 63)
#define PAGE_ADDR_MASK 0x000ffffffffff000
#define PAGE_BIT_A_ACCESSED (1 << 5)
#define PAGE_BIT_D_DIRTY (1 << 6)
#define GB 0x40000000UL

void map_page(void* logical, unsigned int flags);
void map_page_from(void* _pml4, void* logical, unsigned int flags);
void* copy_pml4(void* _pml4);
void vmm_init(struct stivale2_struct* ss);
void* mkpml4(void);
void load_pml4(void* pml4);

#endif
