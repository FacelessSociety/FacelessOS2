;;
;;  MIT License
;;
;;  Copyright (c) 2022 FacelessSociety, Ian Marco Moffett
;;
;;  Permission is hereby granted, free of charge, to any person obtaining a copy
;;  of this software and associated documentation files (the "Software"), to deal
;;  in the Software without restriction, including without limitation the rights
;;  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
;;  copies of the Software, and to permit persons to whom the Software is
;;  furnished to do so, subject to the following conditions:
;;
;;  The above copyright notice and this permission notice shall be included in all
;;  copies or substantial portions of the Software.
;;
;;  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
;;  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
;;  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
;;  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
;;  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
;;  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
;;  SOFTWARE.
;;

bits 64

global div0_handler
global debug_exception
global breakpoint_exception
global overflow_exception
global boundrange_exceeded_exception
global invalid_opcode_exception
global dev_not_avail_exception
global double_fault
global invalid_tss_exception
global segment_not_present
global stack_segment_fault
global general_protection_fault
global page_fault

extern panic


%macro kpanic 1
    cli

    mov rdi, panic_msg1
    call panic

    hlt
%endmacro



div0_handler:
    kpanic 0x0

debug_exception:
    kpanic 0x1

breakpoint_exception:
    kpanic 0x3

overflow_exception:
    kpanic 0x4

boundrange_exceeded_exception:
    kpanic 0x5

invalid_opcode_exception:
    kpanic 0x6

dev_not_avail_exception:
    kpanic 0x7

double_fault:
    kpanic 0x8

invalid_tss_exception:
    kpanic 0xA

segment_not_present:
    kpanic 0xB

stack_segment_fault:
    kpanic 0xC

general_protection_fault:
    kpanic 0xD

page_fault:
    kpanic 0xE


panic_msg1: db "An exception vector fired.", 0xA, 0x0
