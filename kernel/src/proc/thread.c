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

#include <proc/thread.h>
#include <libkern/asm.h>
#include <libkern/panic.h>
#include <arch/memory/kheap.h>


static struct ThreadControlBlock* root_thread = NULL;
static PID running_pid = 0;
static PID next_pid = 0;


static struct ThreadControlBlock* get_thread(PID pid) {
    struct ThreadControlBlock* tmp = root_thread;

    while (tmp != root_thread) {
        if (tmp->pid == pid) {
            return tmp;
        }

        tmp = tmp->next;
    }

    return root_thread;
}


PID get_pid(void) {
    return running_pid;
}


uint8_t is_threading_setup(void) {
    return root_thread != NULL;
}


__attribute__((noreturn)) PID context_switch(void* rip) {
    // If the root thread has
    // not been setup just return 0.
    CLI;

    // Get current thread.
    struct ThreadControlBlock* current = get_thread(get_pid());

    /* 
     *  If the running thread counter goes too high
     *  since we are using a circular linked list it will
     *  wrap back down to 0.
     */
    running_pid = current->pid;

    /*
     * Ensure the thread is not a killed thread.
     * If it is, we will just recursively context switch
     * again.
     */
    if (current->flags & THREAD_KILLED) {
        running_pid++;
        context_switch(NULL);
    }

    /*
     * If we didn't pass NULL as RIP then
     * then we will update the current threads
     * instruction pointer.
     */
    if (rip != NULL) {
        current->rip = rip;
    }

    /*
     * Now we just switch threads and 
     * jump to the instruction pointer.
     */

    running_pid++;
    struct ThreadControlBlock* next = get_thread(running_pid);
    running_pid = next->pid;
    __asm__ __volatile__(
            "mov %0, %%r9; \
            sti;           \
            jmp *%%r9" :: "r" (next->rip));

    while (1);
}


void threading_init(void) {
    CLI;
    // Allocate memory for root thread control block.
    root_thread = kmalloc(sizeof(struct ThreadControlBlock));

    if (root_thread == NULL)
        panic("Failed to setup threading: kmalloc() returned null.");

    // The thread control block list is a circular linked list.
    root_thread->prev = root_thread->next = root_thread;

    // Setup flags and PID.
    root_thread->flags = 0;
    root_thread->pid = next_pid++;
    root_thread->rip = __builtin_extract_return_addr(__builtin_return_address(0));
    STI;
}
