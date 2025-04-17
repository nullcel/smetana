#include "types.h"
#include "console.h"
#include "string.h"
#include "gdt.h"
#include "idt.h"
#include "keyboard.h"
#include "io_ports.h"
#include "filesystem.h"
#include "utils.h"

// Multiboot header constants
#define MULTIBOOT_HEADER_MAGIC 0x1BADB002
#define MULTIBOOT_HEADER_FLAGS 0x00000003  // Align modules on page boundaries and provide memory map
#define MULTIBOOT_HEADER_CHECKSUM -(MULTIBOOT_HEADER_MAGIC + MULTIBOOT_HEADER_FLAGS)

// Ensure multiboot header is properly aligned
__attribute__((section(".multiboot"), aligned(4)))
const struct {
    uint32 magic;
    uint32 flags;
    uint32 checksum;
} multiboot_header = {
    MULTIBOOT_HEADER_MAGIC,
    MULTIBOOT_HEADER_FLAGS,
    MULTIBOOT_HEADER_CHECKSUM
};

// Entry point for the kernel
extern void kmain(void);  // Declaration of kmain

void _start(void) {
    // Initialize essential kernel subsystems first
    gdt_init();
    idt_init();
    console_init(COLOR_WHITE, COLOR_BLACK);
    
    // Call main kernel function
    kmain();
    
    // Halt if kmain returns
    while(1) {
        asm volatile("hlt");
    }
}

void kmain() {
    // Print a message to the screen
    char *video_memory = (char *)0xB8000;
    const char *message = "Smetana OS started";

    for (int i = 0; message[i] != '\0'; i++) {
        video_memory[i * 2] = message[i];
        video_memory[i * 2 + 1] = 0x07; // Light grey on black
    }

    // Halt the system
    while (1) {
        __asm__("hlt");
    }
}