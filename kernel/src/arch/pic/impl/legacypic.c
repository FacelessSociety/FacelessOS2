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


#include <arch/pic/legacypic.h>
#include <arch/io/io.h>


#define EOI 0x20
#define ICW1_ICW4	0x01		
#define ICW1_SINGLE	0x02	
#define ICW1_INTERVAL4	0x04	
#define ICW1_LEVEL	0x08		
#define ICW1_INIT	0x10		
 
#define ICW4_8086	0x01		
#define ICW4_AUTO	0x02		
#define ICW4_BUF_SLAVE	0x08
#define ICW4_BUF_MASTER	0x0C
#define ICW4_SFNM	0x10



typedef enum {
    CT_MASTER_COMMAND = 0x20,
    CT_MASTER_DATA    = 0x21,
    CT_SLAVE_COMMAND  = 0xA0,
    CT_SLAVE_DATA     = 0xA1
} CHIP_TYPE;

// legacypic_set_mask() and legacypic_get_mask()
// is a little abstraction to make things simple.

void legacypic_set_mask(uint16_t mask) {
    outportb(CT_MASTER_DATA, mask & 0xFF);
    outportb(CT_SLAVE_DATA, mask >> 8);
}

uint16_t legacypic_get_mask(void) {
    uint8_t lo = inportb(CT_MASTER_DATA);
    uint8_t hi = inportb(CT_SLAVE_DATA);
    return (uint16_t)(hi << 16 | lo);
}


void legacypic_send_eoi(uint8_t irq) {
    if (irq >= 8)
        outportb(CT_SLAVE_COMMAND, EOI);

    outportb(CT_MASTER_COMMAND, EOI);
}


void legacypic_init(void) {
    // Send init to PICs.
    outportb(CT_MASTER_COMMAND, ICW1_INIT | ICW1_ICW4);
    io_wait();
    outportb(CT_SLAVE_COMMAND, ICW1_INIT | ICW1_ICW4);
    io_wait();

    // Give them their new offsets.
    outportb(CT_MASTER_DATA, 0x20);
    io_wait();
    outportb(CT_SLAVE_DATA, 0x28);
    io_wait();

    // Give some information of it's enviorement.
    outportb(CT_MASTER_DATA, 4);
    io_wait();
    outportb(CT_SLAVE_DATA, 2);
    io_wait();

    outportb(CT_MASTER_DATA, ICW4_8086);
    io_wait();
    outportb(CT_SLAVE_DATA, ICW4_8086);
    io_wait();

    // Mask em.
    outportb(CT_MASTER_DATA, 0xFF);
    io_wait();
    outportb(CT_SLAVE_DATA, 0xFF);
}
