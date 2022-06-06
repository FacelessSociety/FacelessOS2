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


#ifndef IDT_H
#define IDT_H

#include <stdint.h>


struct __attribute__((packed)) IDTGateDescriptor { 
    uint16_t isr_low;       // Low 16 bytes of ISR address/
    uint16_t cs;            // Code segment ISR is in.
    uint8_t ist : 3;
    uint8_t reserved : 5;
    uint8_t attr : 4;
    uint8_t reserved1 : 1;
    uint8_t dpl : 2;        // Descriptor privilege level.
    uint8_t p : 1;          // Present.
    uint16_t isr_middle;    // Middle 16 bits of ISR.
    uint32_t isr_high;      // High 32 bits of ISR.
    uint32_t reserved2; 
};


struct __attribute__((packed)) IDTR {
    uint16_t limit;
    uint64_t base;
};


void idt_install(void);


#endif
