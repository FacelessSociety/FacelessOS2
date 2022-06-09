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


#include <libkern/panic.h>
#include <debug/log.h>


void panic(const char* panic_msg) {
    log("\033[H\033[2J\033[3J");
    log("\033[91m** KERNEL PANIC **\n");
    log("\033[91m%s\n", panic_msg);
    __asm__ __volatile__("cli; hlt");
}


void cpu_panic(uint32_t vector_fired) {
    const char* fault_type = "Unknown Fault";

    log("\033[H\033[2J\033[3J");
    log("\033[91m** KERNEL PANIC **\n");

    // Set fault type.
    switch (vector_fired) {
        case 0x0:
            fault_type = "Divide Error";
            break;
        case 0x1:
            fault_type = "Debug Exception";
            break;
        case 0x3:
            fault_type = "Breakpoint Exception";
            break;
        case 0x4:
            fault_type = "Overflow Exception";
            break;
        case 0x5:
            fault_type = "Boundrange Exceeded";
            break;
        case 0x6:
            fault_type = "Invalid Opcode";
            break;
        case 0x7:
            fault_type = "Device Not Avl";
            break;
        case 0x8:
            fault_type = "Double Fault";
            break;
        case 0xA:
            fault_type = "Invalid TSS";
            break;
        case 0xB:
            fault_type = "Segment Not Present";
            break;
        case 0xC:
            fault_type = "Stack Segment Fault";
            break;
        case 0xD:
            fault_type = "General Protection Fault";
            break;
        case 0xE:
            fault_type = "Page Fault";
            break;

    }

    log("\033[91mCaught: %s\n", fault_type);
}
