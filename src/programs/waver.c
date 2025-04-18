#include "types.h"
#include "io_ports.h"
#include "keyboard.h"
#include "vga.h"
#include "console.h"
#include "string.h"
#include "utils.h"

#define PI 3.14159265359
#define AMPLITUDE 30.0
#define WAVE_SPEED 0.5
#define INITIAL_DENSITY 3  // Initial number of pixels per wave cycle

// Function to calculate sine without using floating point
sint32 sin_fixed(sint32 angle) {
    static const sint32 sin_table[] = {
        0, 17, 34, 50, 64, 77, 87, 95, 98, 98, 95, 87, 77, 64, 50, 34,
        17, 0, -17, -34, -50, -64, -77, -87, -95, -98, -98, -95, -87, -77, -64, -50,
        -34, -17
    };
    
    angle = angle % 360;
    if (angle < 0) angle += 360;
    return sin_table[(angle * 34) / 360];
}

void draw_wave() {
    static sint32 time = 0;
    static sint32 wave_density = INITIAL_DENSITY;
    uint16 x;
    sint16 y;
    uint8 color = COLOR_BLUE;

    vga_set_graphics_mode();

    while (1) {
        // Clear screen by drawing black pixels
        for (y = 0; y < VGA_GRAPHICS_HEIGHT; y++) {
            for (x = 0; x < VGA_GRAPHICS_WIDTH; x++) {
                vga_draw_pixel(x, (uint16)y, COLOR_BLACK);
            }
        }

        // Draw the wave
        for (x = 0; x < VGA_GRAPHICS_WIDTH; x++) {
            sint32 angle = (x * wave_density + time) % 360;
            y = (sint16)((VGA_GRAPHICS_HEIGHT / 2) + (sin_fixed(angle) * AMPLITUDE) / 100);
            
            if (y >= 0 && y < (sint16)VGA_GRAPHICS_HEIGHT) {
                vga_draw_pixel(x, (uint16)y, color);
            }
        }

        time = (time + 1) % 360;
        char key = keyboard_read();
        
        // Press + to increase density, - to decrease density
        if (key == '+' && wave_density < 20) {
            wave_density++;
        } else if (key == '-' && wave_density > 1) {
            wave_density--;
        } else if (key == 27) { // ESC
            break;
        }

        for (int i = 0; i < 100000; i++) { }
    }

    // Return to text mode
    vga_set_text_mode();
}

