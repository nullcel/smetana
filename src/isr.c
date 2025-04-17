#include "isr.h"
#include "idt.h"
#include "8259_pic.h"
#include "console.h"

// For both exceptions and irq interrupt
ISR g_interrupt_handlers[NO_INTERRUPT_HANDLERS];

// for more details, see Intel manual -> Interrupt & Exception Handling
char *exception_messages[32] = {
    "Division By Zero",
    "Debug",
    "Non Maskable Interrupt",
    "Breakpoint",
    "Overflow",
    "BOUND Range Exceeded",
    "Invalid Opcode",
    "Device Not Available (No Math Coprocessor)",
    "Double Fault",
    "Coprocessor Segment Overrun",
    "Invalid TSS",
    "Segment Not Present",
    "Stack-Segment Fault",
    "General Protection",
    "Page Fault",
    "Unknown Interrupt (intel reserved)",
    "x87 FPU Floating-Point Error (Math Fault)",
    "Alignment Check",
    "Machine Check",
    "SIMD Floating-Point Exception",
    "Virtualization Exception",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved"
};

// Custom delay function - busy wait
static void delay(int count) {
    for (int i = 0; i < count * 10000000; i++) {
        asm volatile("nop");
    }
}

/**
 * register given handler to interrupt handlers at given num
 */
void isr_register_interrupt_handler(int num, ISR handler) {
    announce("IRQ %d registered\n", num);
    if (num < NO_INTERRUPT_HANDLERS)
        g_interrupt_handlers[num] = handler;

    delay(1);
    set_text_color(COLOR_GREY, COLOR_BLACK);
    printf("[ ");
    set_text_color(COLOR_YELLOW, COLOR_BLACK);
    printf("loading...");
    set_text_color(COLOR_GREY, COLOR_BLACK);
    printf(" ] ");
    set_text_color(COLOR_WHITE, COLOR_BLACK);
    printf("ISR module\n");
    delay(8);  // 5 units of delay
        
    // Restore original colors for the message
    set_text_color(COLOR_WHITE, COLOR_BLACK);
}

/*
 * turn off current interrupt
*/
void isr_end_interrupt(int num) {
    pic8259_eoi(num);
}

/**
 * invoke isr routine and send eoi to pic,
 * being called in irq.asm
 */
void isr_irq_handler(REGISTERS *reg) {
    if (g_interrupt_handlers[reg->int_no] != NULL) {
        ISR handler = g_interrupt_handlers[reg->int_no];
        handler(reg);
    }
    pic8259_eoi(reg->int_no);
}

static void print_registers(REGISTERS *reg) {
    printf("REGISTERS:\n");
    printf("err_code=%d\n", reg->err_code);
    printf("eax=0x%x, ebx=0x%x, ecx=0x%x, edx=0x%x\n", reg->eax, reg->ebx, reg->ecx, reg->edx);
    printf("edi=0x%x, esi=0x%x, ebp=0x%x, esp=0x%x\n", reg->edi, reg->esi, reg->ebp, reg->esp);
    printf("eip=0x%x, cs=0x%x, ss=0x%x, eflags=0x%x, useresp=0x%x\n", reg->eip, reg->ss, reg->eflags, reg->useresp);
}

static void handle_fpu_exception(REGISTERS *reg) {
    // Reset FPU state
    asm volatile("fnclex");  // Clear FPU exceptions
    asm volatile("finit");   // Reinitialize FPU
    
    // Re-enable FPU
    uint32 cr0;
    asm volatile("mov %%cr0, %0" : "=r"(cr0));
    cr0 &= ~(1 << 2);  // Clear EM flag
    cr0 &= ~(1 << 3);  // Clear TS flag
    asm volatile("mov %0, %%cr0" :: "r"(cr0));
}

/**
 * invoke exception routine,
 * being called in exception.asm
 */
void isr_exception_handler(REGISTERS reg) {
    if (reg.int_no < 32) {
        // Special handling for FPU exceptions
        if (reg.int_no == 7 || reg.int_no == 16) {  // Device not available or FPU error
            handle_fpu_exception(&reg);
            return;  // Return after handling FPU exception
        }
        
        printf("EXCEPTION: %s\n", exception_messages[reg.int_no]);
        print_registers(&reg);
        for (;;)
            ;
    }
    if (g_interrupt_handlers[reg.int_no] != NULL) {
        ISR handler = g_interrupt_handlers[reg.int_no];
        handler(&reg);
    }
}