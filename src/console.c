#include "console.h"
#include "string.h"
#include "types.h"
#include "vga.h"
#include "keyboard.h"

// Add this function declaration
static uint16 *g_vga_buffer;
//index for video buffer array
static uint32 g_vga_index;
// cursor positions
static uint8 cursor_pos_x = 0, cursor_pos_y = 0;
//fore & back color values
uint8 g_fore_color = COLOR_WHITE, g_back_color = COLOR_BLACK;
static uint16 g_temp_pages[MAXIMUM_PAGES][VGA_TOTAL_ITEMS];
uint32 g_current_temp_page = 0;

// clear video buffer array
void console_clear(VGA_COLOR_TYPE fore_color, VGA_COLOR_TYPE back_color) {
    uint32 i;

    for (i = 0; i < VGA_TOTAL_ITEMS; i++) {
        g_vga_buffer[i] = vga_item_entry(NULL, fore_color, back_color);
    }
    g_vga_index = 0;
    cursor_pos_x = 0;
    cursor_pos_y = 0;
    vga_set_cursor_pos(cursor_pos_x, cursor_pos_y);
}

//initialize console
void console_init(VGA_COLOR_TYPE fore_color, VGA_COLOR_TYPE back_color) {
    g_vga_buffer = (uint16 *)VGA_ADDRESS;
    g_fore_color = fore_color;
    g_back_color = back_color;
    cursor_pos_x = 0;
    cursor_pos_y = 0;
    console_clear(fore_color, back_color);
}

void console_scroll(int type) {
    uint32 i;
    if (type == SCROLL_UP) {
        // scroll up
        if (g_current_temp_page > 0)
            g_current_temp_page--;
        g_current_temp_page %= MAXIMUM_PAGES;
        for (i = 0; i < VGA_TOTAL_ITEMS; i++) {
            g_vga_buffer[i] = g_temp_pages[g_current_temp_page][i];
        }
    } else {
        // scroll down
        g_current_temp_page++;
        g_current_temp_page %= MAXIMUM_PAGES;
        for (i = 0; i < VGA_TOTAL_ITEMS; i++) {
            g_vga_buffer[i] = g_temp_pages[g_current_temp_page][i];
        }
    }
}

/*
increase vga_index by width of vga width
*/
static void console_newline() {
    uint32 i;

    if (cursor_pos_y >= VGA_HEIGHT - 1) {
        // Scroll content up by one line
        for (i = 0; i < VGA_TOTAL_ITEMS - VGA_WIDTH; i++) {
            g_vga_buffer[i] = g_vga_buffer[i + VGA_WIDTH];
        }
        // Clear the new line
        for (i = VGA_TOTAL_ITEMS - VGA_WIDTH; i < VGA_TOTAL_ITEMS; i++) {
            g_vga_buffer[i] = vga_item_entry(0, g_fore_color, g_back_color);
        }
        cursor_pos_y = VGA_HEIGHT - 1;  // Keep cursor on the last line
    } else {
        cursor_pos_y++;
    }
    cursor_pos_x = 0;
    g_vga_index = (cursor_pos_y * VGA_WIDTH) + cursor_pos_x;
    vga_set_cursor_pos(cursor_pos_x, cursor_pos_y);
}


//assign ascii character to video buffer
void console_putchar(char ch) {
    if (ch == ' ') {
        g_vga_buffer[g_vga_index++] = vga_item_entry(' ', g_fore_color, g_back_color);
        cursor_pos_x++;
        if (cursor_pos_x >= VGA_WIDTH) {
            cursor_pos_x = 0;
            cursor_pos_y++;
            if (cursor_pos_y >= VGA_HEIGHT) {
                console_newline();
            }
        }
        vga_set_cursor_pos(cursor_pos_x, cursor_pos_y);
    }
    else if (ch == '\t') {
        for(int i = 0; i < 4; i++) {
            g_vga_buffer[g_vga_index++] = vga_item_entry(' ', g_fore_color, g_back_color);
            cursor_pos_x++;
            if (cursor_pos_x >= VGA_WIDTH) {
                cursor_pos_x = 0;
                cursor_pos_y++;
                if (cursor_pos_y >= VGA_HEIGHT) {
                    console_newline();
                }
            }
            vga_set_cursor_pos(cursor_pos_x, cursor_pos_y);
        }
    } 
    else if (ch == '\n') {
        console_newline();
    } 
    else {
        if (ch > 0) {
            g_vga_buffer[g_vga_index++] = vga_item_entry(ch, g_fore_color, g_back_color);
            cursor_pos_x++;
            if (cursor_pos_x >= VGA_WIDTH) {
                cursor_pos_x = 0;
                cursor_pos_y++;
                if (cursor_pos_y >= VGA_HEIGHT) {
                    console_newline();
                }
            }
            vga_set_cursor_pos(cursor_pos_x, cursor_pos_y);
        }
    }
}

// revert back the printed character and add 0 to it
void console_ungetchar() {
    if(g_vga_index > 0) {
        g_vga_buffer[g_vga_index--] = vga_item_entry(0, g_fore_color, g_back_color);
        if(cursor_pos_x > 0) {
            cursor_pos_x--;
            vga_set_cursor_pos(cursor_pos_x, cursor_pos_y);
        } else {
            cursor_pos_x = VGA_WIDTH;
            if (cursor_pos_y > 0) {
                cursor_pos_y--;
                vga_set_cursor_pos(cursor_pos_x, cursor_pos_y);
            } else {
                cursor_pos_y = 0;
                vga_set_cursor_pos(cursor_pos_x, cursor_pos_y);
            }
        }
    }

    // set last printed character to 0
    g_vga_buffer[g_vga_index] = vga_item_entry(0, g_fore_color, g_back_color);
}

// revert back the printed character until n characters
void console_ungetchar_bound(uint8 n) {
    if(((g_vga_index % VGA_WIDTH) > n) && (n > 0)) {
        g_vga_buffer[g_vga_index--] = vga_item_entry(0, g_fore_color, g_back_color);
        if(cursor_pos_x >= n) {
            cursor_pos_x--;
            vga_set_cursor_pos(cursor_pos_x, cursor_pos_y);
        } else {
            cursor_pos_x = VGA_WIDTH;
            if (cursor_pos_y > 0) {
                cursor_pos_y--;
                vga_set_cursor_pos(cursor_pos_x, cursor_pos_y);
            } else {
                cursor_pos_y = 0;
                vga_set_cursor_pos(cursor_pos_x, cursor_pos_y);
            }
        }
    }

    // set last printed character to 0
    g_vga_buffer[g_vga_index] = vga_item_entry(0, g_fore_color, g_back_color);
}

void console_gotoxy(uint16 x, uint16 y) {
    g_vga_index = (80 * y) + x;
    cursor_pos_x = x;
    cursor_pos_y = y;
    vga_set_cursor_pos(cursor_pos_x, cursor_pos_y);
}

//print string by calling print_char
void console_putstr(const char *str) {
    uint32 index = 0;
    while (str[index]) {
        if (str[index] == '\n')
            console_newline();
        else
            console_putchar(str[index]);
        index++;
    }
}

void printf(const char *format, ...) {
    char **arg = (char **)&format;
    int c;
    char buf[32];

    arg++;

    memset(buf, 0, sizeof(buf));
    while ((c = *format++) != 0) {
        if (c != '%')
            console_putchar(c);
        else {
            char *p, *p2;
            int pad0 = 0, pad = 0;

            c = *format++;
            if (c == '0') {
                pad0 = 1;
                c = *format++;
            }

            if (c >= '0' && c <= '9') {
                pad = c - '0';
                c = *format++;
            }

            switch (c) {
                case 'd':
                case 'u':
                case 'x':
                    itoa(buf, c, *((int *)arg++));
                    p = buf;
                    goto string;
                    break;

                case 's':
                    p = *arg++;
                    if (!p)
                        p = "(null)";

                string:
                    for (p2 = p; *p2; p2++)
                        ;
                    for (; p2 < p + pad; p2++)
                        console_putchar(pad0 ? '0' : ' ');
                    while (*p)
                        console_putchar(*p++);
                    break;

                default:
                    console_putchar(*((int *)arg++));
                    break;
            }
        }
    }
}

// read string from console, but no backing
void getstr(char *buffer) {
    if (!buffer) return;
    while(1) {
        char ch = kb_getchar();
        if (ch == '\n') {
            printf("\n");
            return ;
        } else {
            *buffer++ = ch;
            printf("%c", ch);
        }
    }
}

// read string from console, and erase or go back util bound occurs
void getstr_bound(char *buffer, uint8 bound) {
    if (!buffer) return;
    while(1) {
        char ch = kb_getchar();
        if (ch == '\n') {
            printf("\n");
            return ;
        } else if(ch == '\b') {
            console_ungetchar_bound(bound);
            buffer--;
            *buffer = '\0';
        } else {
            *buffer++ = ch;
            printf("%c", ch);
        }
    }
}

void set_text_color(VGA_COLOR_TYPE fore_color, VGA_COLOR_TYPE back_color) {
    g_fore_color = fore_color;
    g_back_color = back_color;
}

void reset_text_color(void) {
    g_fore_color = COLOR_WHITE;
    g_back_color = COLOR_BLACK;
}

void print_available_colors(void) {
    VGA_COLOR_TYPE original_fore = g_fore_color;
    VGA_COLOR_TYPE original_back = g_back_color;

    printf("Available colors:\n");
    set_text_color(COLOR_BLACK, COLOR_WHITE);          printf("BLACK   "); reset_text_color(); printf("\n");
    set_text_color(COLOR_BLUE, COLOR_WHITE);           printf("BLUE    "); reset_text_color(); printf("\n");
    set_text_color(COLOR_GREEN, COLOR_BLACK);          printf("GREEN   "); reset_text_color(); printf("\n");
    set_text_color(COLOR_CYAN, COLOR_BLACK);           printf("CYAN    "); reset_text_color(); printf("\n");
    set_text_color(COLOR_RED, COLOR_BLACK);            printf("RED     "); reset_text_color(); printf("\n");
    set_text_color(COLOR_MAGENTA, COLOR_BLACK);        printf("MAGENTA "); reset_text_color(); printf("\n");
    set_text_color(COLOR_BROWN, COLOR_BLACK);          printf("BROWN   "); reset_text_color(); printf("\n");
    set_text_color(COLOR_GREY, COLOR_BLACK);           printf("GREY    "); reset_text_color(); printf("\n");
    set_text_color(COLOR_DARK_GREY, COLOR_WHITE);      printf("DARK_GREY"); reset_text_color(); printf("\n");
    set_text_color(COLOR_BRIGHT_BLUE, COLOR_BLACK);    printf("BRIGHT_BLUE"); reset_text_color(); printf("\n");
    set_text_color(COLOR_BRIGHT_GREEN, COLOR_BLACK);   printf("BRIGHT_GREEN"); reset_text_color(); printf("\n");
    set_text_color(COLOR_BRIGHT_CYAN, COLOR_BLACK);    printf("BRIGHT_CYAN"); reset_text_color(); printf("\n");
    set_text_color(COLOR_BRIGHT_RED, COLOR_BLACK);     printf("BRIGHT_RED"); reset_text_color(); printf("\n");
    set_text_color(COLOR_BRIGHT_MAGENTA, COLOR_BLACK); printf("BRIGHT_MAGENTA"); reset_text_color(); printf("\n");
    set_text_color(COLOR_YELLOW, COLOR_BLACK);         printf("YELLOW"); reset_text_color(); printf("\n");
    set_text_color(COLOR_WHITE, COLOR_BLACK);          printf("WHITE"); reset_text_color(); printf("\n");

    // Restore original colors
    g_fore_color = original_fore;
    g_back_color = original_back;
}

void announce(const char *format, ...) {
    VGA_COLOR_TYPE original_fore = g_fore_color;
    VGA_COLOR_TYPE original_back = g_back_color;
    
    // Print the prefix in grey
    set_text_color(COLOR_GREY, COLOR_BLACK);
    printf("[ ");
    set_text_color(COLOR_GREEN, COLOR_BLACK);
    printf("os");
    set_text_color(COLOR_GREY, COLOR_BLACK);
    printf(" ] ");

    
    // Restore original colors for the message
    set_text_color(original_fore, original_back);
    
    // Handle the variadic arguments similar to printf
    char **arg = (char **)&format;
    int c;
    char buf[32];

    arg++;
    memset(buf, 0, sizeof(buf));
    
    while ((c = *format++) != 0) {
        if (c != '%')
            console_putchar(c);
        else {
            char *p, *p2;
            int pad0 = 0, pad = 0;

            c = *format++;
            if (c == '0') {
                pad0 = 1;
                c = *format++;
            }

            if (c >= '0' && c <= '9') {
                pad = c - '0';
                c = *format++;
            }

            switch (c) {
                case 'd':
                case 'u':
                case 'x':
                    itoa(buf, c, *((int *)arg++));
                    p = buf;
                    goto string;
                    break;

                case 's':
                    p = *arg++;
                    if (!p)
                        p = "(null)";

                string:
                    for (p2 = p; *p2; p2++)
                        ;
                    for (; p2 < p + pad; p2++)
                        console_putchar(pad0 ? '0' : ' ');
                    while (*p)
                        console_putchar(*p++);
                    break;

                default:
                    console_putchar(*((int *)arg++));
                    break;
            }
        }
    }
}
