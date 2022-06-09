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
global gdt_load

GDT:
    null: equ $ - GDT
        dw 0                         ; Limit (low).
        dw 0                         ; Base (low).
        db 0                         ; Base (middle)
        db 0                         ; Access.
        db 1                         ; Granularity.
        db 0                         ; Base (high).
    code16:
        dw 0xFFFF
        dw 0
        db 0
        db 0b10011010
        db 0b00000000
        db 0
    data16:
        dw 0xFFFF
        dw 0
        db 0
        db 0b10010010
        db 0b00000000
        db 0
    code32:
        dw 0xFFFF
        dw 0
        db 0
        db 0b10011010
        db 0b11001111
        db 0
    data32:
        dw 0xFFFF
        dw 0
        db 0
        db 0b10010010
        db 0b11001111
        db 0
    code64:
        dw 0
        dw 0
        db 0
        db 0b10011010
        db 0b00100000
        db 0
    data64:
        dw 0
        dw 0
        db 0
        db 0b10010010
        db 0b00000000
        db 0
    gdt_ptr:
        dw $ - GDT - 1
        dq GDT


gdt_load:
    cli
    lgdt [gdt_ptr]
    mov rax, 0x30
    mov ds, rax
    mov es, rax
    mov fs, rax
    mov gs, rax
    mov ss, rax
    pop rdi
    mov rax, 0x28
    push rax
    push rdi
    retfq
