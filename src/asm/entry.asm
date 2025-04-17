; constants for multiboot header
MBALIGN     equ  1<<0
MEMINFO     equ  1<<1
FLAGS       equ  MBALIGN | MEMINFO
MAGIC       equ  0x1BADB002
CHECKSUM    equ -(MAGIC + FLAGS)

; set multiboot section
section .multiboot
    align 4
    dd MAGIC
    dd FLAGS
    dd CHECKSUM

section .data
    align 4096

; initial stack
section .initial_stack, nobits
    align 4

stack_bottom:
    ; 1 MB of uninitialized data for stack
    resb 104856
stack_top:

; kernel entry, main text section
section .text
    global _start

; Early FPU initialization
init_fpu:
    fninit          ; Initialize FPU
    mov eax, cr0
    and al, 0xF3    ; Clear EM and TS flags (bits 2 and 3)
    or al, 0x22     ; Set MP and NE flags (bits 1 and 5)
    mov cr0, eax
    ret

; define _start, aligned by linker.ld script
_start:
    mov esp, stack_top
    call init_fpu   ; Initialize FPU before anything else
    extern kmain
    push ebx
    call kmain
loop:
    jmp loop
