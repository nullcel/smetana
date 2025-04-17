#include "vga.h"
#include "io_ports.h"
#include "types.h"

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
uint16 vga_item_entry(uint8 ch, VGA_COLOR_TYPE fore_color, VGA_COLOR_TYPE back_color) {
    uint16 ax = 0;
    uint8 ah = 0, al = 0;

    ah = back_color;
    ah <<= 4;
    ah |= fore_color;
    ax = ah;
    ax <<= 8;
    al = ch;
    ax |= al;

    return ax;
}

/**
 * set cursor position to given (x, y)
 * by writing to CRT controller registers
 */
void vga_set_cursor_pos(uint8 x, uint8 y) {
    // The screen is 80 characters wide...
    uint16 cursorLocation = y * VGA_WIDTH + x;
    outportb(0x3D4, 14);
    outportb(0x3D5, cursorLocation >> 8);
    outportb(0x3D4, 15);
    outportb(0x3D5, cursorLocation);
}

/**
 * disable blinking top-left cursor
 * by writing to CRT controller registers
 */
void vga_disable_cursor() {
    outportb(0x3D4, 10);
    outportb(0x3D5, 32);
}

void vga_set_graphics_mode(void) {
    // Set VGA mode 13h (320x200, 256 colors)
    outportb(0x3C2, 0x63);
    
    // Sequencer registers
    outportb(0x3C4, 0x00); outportb(0x3C5, 0x03);
    outportb(0x3C4, 0x01); outportb(0x3C5, 0x01);
    outportb(0x3C4, 0x02); outportb(0x3C5, 0x0F);
    outportb(0x3C4, 0x03); outportb(0x3C5, 0x00);
    outportb(0x3C4, 0x04); outportb(0x3C5, 0x0E);

    // CRT Controller registers
    outportb(0x3D4, 0x00); outportb(0x3D5, 0x5F);
    outportb(0x3D4, 0x01); outportb(0x3D5, 0x4F);
    outportb(0x3D4, 0x02); outportb(0x3D5, 0x50);
    outportb(0x3D4, 0x03); outportb(0x3D5, 0x82);
    outportb(0x3D4, 0x04); outportb(0x3D5, 0x54);
    outportb(0x3D4, 0x05); outportb(0x3D5, 0x80);
    outportb(0x3D4, 0x06); outportb(0x3D5, 0xBF);
    outportb(0x3D4, 0x07); outportb(0x3D5, 0x1F);
    outportb(0x3D4, 0x08); outportb(0x3D5, 0x00);
    outportb(0x3D4, 0x09); outportb(0x3D5, 0x41);
    outportb(0x3D4, 0x10); outportb(0x3D5, 0x9C);
    outportb(0x3D4, 0x11); outportb(0x3D5, 0x8E);
    outportb(0x3D4, 0x12); outportb(0x3D5, 0x8F);
    outportb(0x3D4, 0x13); outportb(0x3D5, 0x28);
    outportb(0x3D4, 0x14); outportb(0x3D5, 0x00);
    outportb(0x3D4, 0x15); outportb(0x3D5, 0x96);
    outportb(0x3D4, 0x16); outportb(0x3D5, 0xB9);
    outportb(0x3D4, 0x17); outportb(0x3D5, 0xA3);

    // Graphics Controller registers
    outportb(0x3CE, 0x00); outportb(0x3CF, 0x00);
    outportb(0x3CE, 0x01); outportb(0x3CF, 0x00);
    outportb(0x3CE, 0x02); outportb(0x3CF, 0x00);
    outportb(0x3CE, 0x03); outportb(0x3CF, 0x00);
    outportb(0x3CE, 0x04); outportb(0x3CF, 0x00);
    outportb(0x3CE, 0x05); outportb(0x3CF, 0x40);
    outportb(0x3CE, 0x06); outportb(0x3CF, 0x05);
    outportb(0x3CE, 0x07); outportb(0x3CF, 0x0F);
    outportb(0x3CE, 0x08); outportb(0x3CF, 0xFF);

    // Set up initial palette (make color 15 white)
    outportb(0x3C8, 15);      // Select palette index 15
    outportb(0x3C9, 63);      // Maximum red
    outportb(0x3C9, 63);      // Maximum green  
    outportb(0x3C9, 63);      // Maximum blue

    // Clear the screen buffer
    uint8* vga = (uint8*)VGA_GRAPHICS_ADDRESS;
    for(int i = 0; i < VGA_GRAPHICS_WIDTH * VGA_GRAPHICS_HEIGHT; i++) {
        vga[i] = 0;  // Set to black
    }
}

void vga_set_text_mode(void) {
    // Return to text mode 03h (80x25 text)
    outportb(0x3C2, 0x67);  // Miscellaneous output 
    
    // Sequencer registers
    outportb(0x3C4, 0x00); outportb(0x3C5, 0x03);
    outportb(0x3C4, 0x01); outportb(0x3C5, 0x00);
    outportb(0x3C4, 0x02); outportb(0x3C5, 0x03);
    outportb(0x3C4, 0x03); outportb(0x3C5, 0x00);
    outportb(0x3C4, 0x04); outportb(0x3C5, 0x02);

    // CRT Controller registers
    outportb(0x3D4, 0x00); outportb(0x3D5, 0x5F);
    outportb(0x3D4, 0x01); outportb(0x3D5, 0x4F);
    outportb(0x3D4, 0x02); outportb(0x3D5, 0x50);
    outportb(0x3D4, 0x03); outportb(0x3D5, 0x82);
    outportb(0x3D4, 0x04); outportb(0x3D5, 0x55);
    outportb(0x3D4, 0x05); outportb(0x3D5, 0x81);
    outportb(0x3D4, 0x06); outportb(0x3D5, 0xBF);
    outportb(0x3D4, 0x07); outportb(0x3D5, 0x1F);
    outportb(0x3D4, 0x08); outportb(0x3D5, 0x00);
    outportb(0x3D4, 0x09); outportb(0x3D5, 0x4F);
    outportb(0x3D4, 0x10); outportb(0x3D5, 0x0E);
    outportb(0x3D4, 0x11); outportb(0x3D5, 0x0F);
    outportb(0x3D4, 0x12); outportb(0x3D5, 0x00);
    outportb(0x3D4, 0x13); outportb(0x3D5, 0x00);
    outportb(0x3D4, 0x14); outportb(0x3D5, 0x00);
    outportb(0x3D4, 0x15); outportb(0x3D5, 0x00);
    outportb(0x3D4, 0x16); outportb(0x3D5, 0x00);
    outportb(0x3D4, 0x17); outportb(0x3D5, 0x00);

    // Graphics Controller registers
    outportb(0x3CE, 0x00); outportb(0x3CF, 0x00);
    outportb(0x3CE, 0x01); outportb(0x3CF, 0x00);
    outportb(0x3CE, 0x02); outportb(0x3CF, 0x00);
    outportb(0x3CE, 0x03); outportb(0x3CF, 0x00);
    outportb(0x3CE, 0x04); outportb(0x3CF, 0x00);
    outportb(0x3CE, 0x05); outportb(0x3CF, 0x10);
    outportb(0x3CE, 0x06); outportb(0x3CF, 0x0E);
    outportb(0x3CE, 0x07); outportb(0x3CF, 0x00);
    outportb(0x3CE, 0x08); outportb(0x3CF, 0xFF);

    // Attribute Controller registers 
    outportb(0x3C0, 0x10); outportb(0x3C0, 0x0C);
    outportb(0x3C0, 0x11); outportb(0x3C0, 0x00);
    outportb(0x3C0, 0x12); outportb(0x3C0, 0x0F);
    outportb(0x3C0, 0x13); outportb(0x3C0, 0x08);
    outportb(0x3C0, 0x14); outportb(0x3C0, 0x00);
    
    // Enable the screen (very important!)
    outportb(0x3C0, 0x20);  // Re-enable video display

    // Don't clear the text buffer - let the existing content show
}

void vga_draw_pixel(uint16 x, uint16 y, uint8 color) {
    if (x >= VGA_GRAPHICS_WIDTH || y >= VGA_GRAPHICS_HEIGHT) return;
    uint8* vga = (uint8*)VGA_GRAPHICS_ADDRESS;
    uint32 offset = y * VGA_GRAPHICS_WIDTH + x;
    if(offset < VGA_GRAPHICS_WIDTH * VGA_GRAPHICS_HEIGHT) {
        vga[offset] = color;
    }
}

void vga_draw_rect(uint16 x, uint16 y, uint16 width, uint16 height, uint8 color) {
    for(uint16 py = y; py < y + height; py++) {
        for(uint16 px = x; px < x + width; px++) {
            vga_draw_pixel(px, py, color);
        }
    }
}

