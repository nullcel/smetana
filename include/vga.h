#ifndef VGA_H
#define VGA_H

#include "types.h"

#define VGA_ADDRESS        0xB8000
#define VGA_TOTAL_ITEMS    2200

#define VGA_WIDTH     80
#define VGA_HEIGHT    24

typedef enum {
    COLOR_BLACK,
    COLOR_BLUE,
    COLOR_GREEN,
    COLOR_CYAN,
    COLOR_RED,
    COLOR_MAGENTA,
    COLOR_BROWN,
    COLOR_GREY,
    COLOR_DARK_GREY,
    COLOR_BRIGHT_BLUE,
    COLOR_BRIGHT_GREEN,
    COLOR_BRIGHT_CYAN,
    COLOR_BRIGHT_RED,
    COLOR_BRIGHT_MAGENTA,
    COLOR_YELLOW,
    COLOR_WHITE,
} VGA_COLOR_TYPE;

/**
 * 16 bit video buffer elements(register ax)
 * 8 bits(ah) higher : 
 * lower 4 bits - forec olor
 * higher 4 bits - back color

 * 8 bits(al) lower :
 * 8 bits : ASCII character to print
 * 
 * returns complete item with fore & back color to be placed at VGA address
*/
uint16 vga_item_entry(uint8 ch, VGA_COLOR_TYPE fore_color, VGA_COLOR_TYPE back_color);

/**
 * set cursor position to given (x, y)
 * by writing to CRT controller registers
 */
void vga_set_cursor_pos(uint8 x, uint8 y);

/**
 * disable blinking top-left cursor
 * by writing to CRT controller registers
 */
void vga_disable_cursor();

// Graphics mode definitions
#define VGA_GRAPHICS_MODE 0x13    // 320x200 256 colors
#define VGA_GRAPHICS_WIDTH 320
#define VGA_GRAPHICS_HEIGHT 200
#define VGA_GRAPHICS_ADDRESS 0xA0000

// Graphics functions
void vga_set_graphics_mode(void);
void vga_set_text_mode(void);
void vga_draw_pixel(uint16 x, uint16 y, uint8 color);
void vga_draw_rect(uint16 x, uint16 y, uint16 width, uint16 height, uint8 color);

#endif
