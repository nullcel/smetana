#include "keyboard.h"
#include "console.h"
#include "idt.h"
#include "io_ports.h"
#include "isr.h"
#include "types.h"
#include "string.h"

#define INPUT_BUFFER_SIZE 256  // Adjust as needed

static BOOL g_caps_lock = FALSE;
static BOOL g_shift_pressed = FALSE;
char g_ch = 0, g_scan_code = 0;

// Input buffer and position tracking
static char g_input_buffer[INPUT_BUFFER_SIZE];
static int g_input_pos = 0;

// see scan codes defined in keyboard.h for index
char g_scan_code_chars[128] = {
    0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0,
    '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' ',
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '-', 0, 0, 0, '+', 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0
};

static int get_scancode() {
    int i, scancode = 0;

    // get scancode until status is on(key pressed)
    for (i = 1000; i > 0; i++) {
        // Check if scan code is ready
        if ((inportb(KEYBOARD_STATUS_PORT) & 1) == 0) continue;
        // Get the scan code
        scancode = inportb(KEYBOARD_DATA_PORT);
        break;
    }
    if (i > 0)
        return scancode;
    return 0;
}

char alternate_chars(char ch) {
    switch(ch) {
        case '`': return '~';
        case '1': return '!';
        case '2': return '@';
        case '3': return '#';
        case '4': return '$';
        case '5': return '%';
        case '6': return '^';
        case '7': return '&';
        case '8': return '*';
        case '9': return '(';
        case '0': return ')';
        case '-': return '_';
        case '=': return '+';
        case '[': return '{';
        case ']': return '}';
        case '\\': return '|';
        case ';': return ':';
        case '\'': return '\"';
        case ',': return '<';
        case '.': return '>';
        case '/': return '?';
        default: return ch;
    }
}

void keyboard_handler(REGISTERS *r) {
    int scancode;

    g_ch = 0;
    scancode = get_scancode();
    g_scan_code = scancode;
    
    if (scancode & 0x80) {
        // key release
        switch(scancode & 0x7F) {
            case SCAN_CODE_KEY_LEFT_SHIFT:
                g_shift_pressed = FALSE;
                break;
        }
    } else {
        // key down
        switch(scancode) {
            case SCAN_CODE_KEY_CAPS_LOCK:
                g_caps_lock = !g_caps_lock;
                break;

            case SCAN_CODE_KEY_ENTER:
                g_ch = '\n';
                g_input_buffer[g_input_pos] = '\0'; // Null-terminate the string
                g_input_pos = 0; // Reset position for next input
                break;

            case SCAN_CODE_KEY_TAB:
                g_ch = '\t';
                break;

            case SCAN_CODE_KEY_LEFT_SHIFT:
                g_shift_pressed = TRUE;
                break;

            case SCAN_CODE_KEY_BACKSPACE:
                if (g_input_pos > 0) {
                    g_input_pos--;
                    g_input_buffer[g_input_pos] = '\0';
                    console_backspace();
                }
                break;

            default:
                g_ch = g_scan_code_chars[scancode];
                if (g_ch) {
                    // if caps is on, convert to upper
                    if (g_caps_lock) {
                        // if shift is pressed before
                        if (g_shift_pressed) {
                            // replace alternate chars
                            g_ch = alternate_chars(g_ch);
                        } else {
                            g_ch = upper(g_ch);
                        }
                    } else {
                        if (g_shift_pressed) {
                            if (isalpha(g_ch)) {
                                g_ch = upper(g_ch);
                            } else {
                                // replace alternate chars
                                g_ch = alternate_chars(g_ch);
                            }
                        }
                    }
                    
                    // Only add to buffer if there's space and it's a printable character
                    if (g_input_pos < INPUT_BUFFER_SIZE - 1 && g_ch >= ' ') {
                        g_input_buffer[g_input_pos] = g_ch;
                        g_input_pos++;
                    }
                }
                break;
        }
    }
}

void keyboard_init() {
    memset(g_input_buffer, 0, INPUT_BUFFER_SIZE);
    g_input_pos = 0;
    isr_register_interrupt_handler(IRQ_BASE + 1, keyboard_handler);
}

// a blocking character read
char kb_getchar() {
    char c;

    while(g_ch <= 0);
    c = g_ch;
    g_ch = 0;
    g_scan_code = 0;
    return c;
}

char kb_get_scancode() {
    char code;

    while(g_scan_code <= 0);
    code = g_scan_code;
    g_ch = 0;
    g_scan_code = 0;
    return code;
}

int kb_getkey() {
    char c = kb_getchar();
    if (c == 3) {  // Ctrl+C
        return 3;
    }
    return c;
}

// Get raw keyboard scan code without blocking
char kb_scan(void) {
    if ((inportb(KEYBOARD_STATUS_PORT) & 1) == 0) {
        return 0;
    }
    return inportb(KEYBOARD_DATA_PORT);
}

// Non-blocking character read
char keyboard_read() {
    char c = g_ch;
    g_ch = 0;
    return c;
}

// Get the current input buffer (for command processing)
const char* kb_get_input_buffer() {
    return g_input_buffer;
}