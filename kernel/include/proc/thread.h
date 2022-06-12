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

#ifndef THREAD_H
#define THREAD_H

#include <stdint.h>

typedef uint16_t PID;

struct ThreadControlBlock {
    PID pid;
    void* rip;
    uint16_t flags;
    struct ThreadControlBlock* next;
    struct ThreadControlBlock* prev;
};


typedef enum {
    THREAD_KILLED = (1 << 0)
} THREAD_FLAG;


void threading_init(void);


// Used by IRQ0 handler.
uint8_t is_threading_setup(void);

/*
 * Context switches to the next
 * thread.
 *
 * @rip: Instruction pointer
 *    to update the CURRENTLY running
 *    thread with.
 *
 *    Should be NULL if you just want to 
 *    go to the next thread.
 *
 * Returns the PID of the thread that 
 * it switched to.
 *
 */

__attribute__((noreturn)) PID context_switch(void* rip);
PID get_pid(void);

#endif
