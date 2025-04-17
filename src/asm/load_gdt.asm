section .text
    global load_gdt

load_gdt:
    mov eax, [esp + 4]  ; get gdt pointer
    lgdt [eax]          ; load gdt

    mov ax, 0x10    ; kernel data segment
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    cli             ; clear interrupts
    
    ; Initialize FPU
    fninit         ; Initialize FPU
    mov eax, cr0
    and al, 0xF3   ; Clear EM and TS flags (bits 2 and 3)
    or al, 0x22    ; Set MP and NE flags (bits 1 and 5)
    mov cr0, eax
    
    ; Enter protected mode
    mov	eax, cr0    
    or eax, 1
    mov	cr0, eax

    jmp 0x08:far_jump   ; jump to far with code data segment
far_jump:
    ret

