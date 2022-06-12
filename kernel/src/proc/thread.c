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
#include <arch/memory/kheap.h>
#include <debug/log.h>

static struct ThreadControlBlock {
    void* rip;
    PID pid;
    uint8_t killed;
    struct ThreadControlBlock* next;
} *root_thread = NULL;

PID next_pid = 0;
PID running_thread = 0;
static uint8_t lock = 0;


static struct ThreadControlBlock* get_thread(PID pid) {
    struct ThreadControlBlock* tmp = root_thread;

    while (tmp) {
        if (tmp->pid == pid)
            break;

        tmp = tmp->next;
    }

    if (tmp == NULL) {
        tmp = root_thread;
        running_thread = 0;
    }

    return tmp;
}


// Forces a thread to "wait".
void mutex_lock(uint8_t* lock) {
    while (__atomic_test_and_set(lock, __ATOMIC_ACQUIRE));
}

// Allows waiting threads to continue.
void mutex_unlock(uint8_t* lock) {
    __atomic_clear(lock, __ATOMIC_RELEASE);
}


void spinlock(uint8_t* lock) {
    *lock = 1;
    while (*lock);
}


void spinunlock(uint8_t* lock) {
    *lock = 0;
}


int fork(void) {
    __asm__ __volatile__("cli");
    struct ThreadControlBlock* parent_thread = get_thread(running_thread);

    struct ThreadControlBlock* child_thread = kmalloc(sizeof(struct ThreadControlBlock));
    child_thread->pid = next_pid++;
    child_thread->next = NULL;
    child_thread->rip = __builtin_extract_return_addr(__builtin_return_address(0));

    struct ThreadControlBlock* tmp = root_thread;

    while (tmp->next)
        tmp = tmp->next;

    tmp->next = child_thread;

    /*
    if (get_thread(running_thread) == parent_thread) {

    }
    */

    __asm__ __volatile__("sti");
    return child_thread->pid;
}


/*
 * Kills a thread.
 *
 * @pid is the PID of the thread
 * that should be killed.
 *
 */

void kill(PID pid) {
    mutex_lock(&lock);
    struct ThreadControlBlock* tmp = root_thread;

    while (tmp) {
        if (tmp->next) {
            if (tmp->next->pid == pid) {
                tmp->killed = 1;
                break;
            }
        }

        tmp = tmp->next;
    }
}


void thread_switch(void* ret_rip) {
    if (root_thread == NULL) return;
    
    if (get_thread(running_thread)->killed) {
        ++running_thread;

        uint8_t lock = 0;
        spinlock(&lock);
    }


    /*
     * Doing a mutex unlock in case lock was set
     * by kill().
     *
     */
    mutex_unlock(&lock);

    get_thread(running_thread++)->rip = ret_rip;

    __asm__ __volatile__(
            "sti; \
            jmp *%0" :: "r" (get_thread(running_thread)->rip));
}


void init_multithreading(void) {
    // Allocate memory for a thread control block.
    root_thread = kmalloc(sizeof(struct ThreadControlBlock));

    // Setup the root thread.
    root_thread->pid = next_pid++;
    root_thread->next = NULL;
}
