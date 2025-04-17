#include "types.h"
#include "io_ports.h"
#include "keyboard.h"
#include "vga.h"
#include "console.h"
#include "string.h"
#include "utils.h"

// Box dimensions (in pixels)
#define BOX_WIDTH  100
#define BOX_HEIGHT 80
#define BOX_X      110  // Center on 320x200 screen
#define BOX_Y      60
#define LINE_COLOR 15   // White in default VGA palette

int liner(void) {
    // Save text mode buffer
    uint16* text_save = (uint16*)malloc(VGA_WIDTH * VGA_HEIGHT * sizeof(uint16));
    memcpy(text_save, (void*)VGA_ADDRESS, VGA_WIDTH * VGA_HEIGHT * sizeof(uint16));
    
    // Clear keyboard buffer before switching modes
    while (kb_scan() != 0);
    
    // Switch to graphics mode and ensure screen is cleared
    vga_set_graphics_mode();
    
    // Draw the box outline
    vga_draw_rect(BOX_X, BOX_Y, BOX_WIDTH, 1, LINE_COLOR);                    // Top
    vga_draw_rect(BOX_X, BOX_Y + BOX_HEIGHT, BOX_WIDTH, 1, LINE_COLOR);      // Bottom
    vga_draw_rect(BOX_X, BOX_Y, 1, BOX_HEIGHT + 1, LINE_COLOR);              // Left
    vga_draw_rect(BOX_X + BOX_WIDTH - 1, BOX_Y, 1, BOX_HEIGHT + 1, LINE_COLOR); // Right
    
    // Wait for Q key press
    while (1) {
        char scan = kb_scan();
        if (scan != 0) {
            if (scan == SCAN_CODE_KEY_Q) {
                break;
            }
            while (kb_scan() != 0);  // Flush any other keys
        }
    }
    
    // Switch back to text mode
    vga_set_text_mode();
    
    // Restore text mode buffer
    memcpy((void*)VGA_ADDRESS, text_save, VGA_WIDTH * VGA_HEIGHT * sizeof(uint16));
    free(text_save);
    
    return 0;
}