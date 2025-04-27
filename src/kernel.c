#include "kernel.h"
#include "console.h"
#include "string.h"
#include "gdt.h"
#include "idt.h"
#include "keyboard.h"
#include "io_ports.h"
#include "filesystem.h"
#include "utils.h"
#include "info.h"

// Update the external declaration to match the correct function name
extern void draw_wave(void);

#define BRAND_QEMU  1
#define BRAND_VBOX  2

// Declare external color variables from console.c
extern uint8 g_fore_color, g_back_color;

// Helper function to get current path
static void get_current_path(char* path_buffer, uint32 size) {
    char temp[MAX_PATH];
    FileNode* node = fs_get_current_dir();
    uint32 path_len = 0;  // Change to uint32 to match size type

    // Handle root directory
    if (node == fs_path_to_node("/")) {
        strcpy(path_buffer, "/");
        return;
    }

    // Build path from current directory up to root
    while (node != fs_path_to_node("/")) {
        uint32 name_len = strlen(node->name);  // Change to uint32
        if (path_len + name_len + 1 >= size) break;
        
        // Shift existing path right
        memmove(temp + name_len + 1, temp, path_len);
        // Add new directory name
        memcpy(temp, node->name, name_len);
        temp[name_len] = '/';
        path_len += name_len + 1;
        
        node = node->parent;
    }

    // Add leading slash and null terminator
    temp[path_len] = '\0';
    strcpy(path_buffer, "/");
    strcat(path_buffer, temp);
}

void __cpuid(uint32 type, uint32 *eax, uint32 *ebx, uint32 *ecx, uint32 *edx) {
    asm volatile("cpuid"
                : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx)
                : "0"(type)); // put the type into eax
}

int cpuid_info(int print) {
    uint32 brand[12];
    uint32 eax, ebx, ecx, edx;
    uint32 type;

    memset(brand, 0, sizeof(brand));
    __cpuid(0x80000002, (uint32 *)brand+0x0, (uint32 *)brand+0x1, (uint32 *)brand+0x2, (uint32 *)brand+0x3);
    __cpuid(0x80000003, (uint32 *)brand+0x4, (uint32 *)brand+0x5, (uint32 *)brand+0x6, (uint32 *)brand+0x7);
    __cpuid(0x80000004, (uint32 *)brand+0x8, (uint32 *)brand+0x9, (uint32 *)brand+0xa, (uint32 *)brand+0xb);

    if (print) {
        printf("Brand: %s\n", (char*)brand);
        for(type = 0; type < 4; type++) {
            __cpuid(type, &eax, &ebx, &ecx, &edx);
            printf("type:0x%x, eax:0x%x, ebx:0x%x, ecx:0x%x, edx:0x%x\n", type, eax, ebx, ecx, edx);
        }
    }

    if (strstr((const char*)brand, "QEMU") != NULL)
        return BRAND_QEMU;

    return BRAND_VBOX;
}

BOOL is_echo(char *b) {
    if((b[0]=='e')&&(b[1]=='c')&&(b[2]=='h')&&(b[3]=='o'))
        if(b[4]==' '||b[4]=='\0')
            return TRUE;
    return FALSE;
}

void shutdown() {
    int brand = cpuid_info(0);
        // QEMU
    if (brand == BRAND_QEMU)
        outports(0x604, 0x2000);
    else
        // VirtualBox
        outports(0x4004, 0x3400);
}

// Custom delay function - busy wait
static void delay(int count) {
    for (int i = 0; i < count * 10000000; i++) {
        asm volatile("nop");
    }
}

void kmain() {
    char buffer[255];
    char command[32];
    char args[223];
    char prompt[MAX_PATH + 32];  // Extra space for "user@Smetana:" and "$"
    char current_path[MAX_PATH];

    gdt_init();
    idt_init();
    console_init(COLOR_WHITE, COLOR_BLACK);
    keyboard_init();
    fs_init();  // Initialize filesystem

    announce("Smetana Interactive Shell initialized\n");
    delay(2);
    announce("Teletype /dev/tty1 initialized\n");
    announce("Filesystem initialized\n");
    delay(1);

    while(1) {
        // Build prompt with current directory
        get_current_path(current_path, MAX_PATH);
        
        VGA_COLOR_TYPE orig_fore = g_fore_color;
        VGA_COLOR_TYPE orig_back = g_back_color;
        
        set_text_color(COLOR_BRIGHT_GREEN, COLOR_BLACK);
        printf("tty1@");
        printf("smetana");
        
        set_text_color(COLOR_WHITE, COLOR_BLACK);
        printf(":");
        
        set_text_color(COLOR_BRIGHT_BLUE, COLOR_BLACK);
        printf("%s", current_path);
        

        set_text_color(COLOR_WHITE, COLOR_BLACK);
        printf("$ ");
        
        // Calculate total prompt length for input bound
        strcpy(prompt, "tty1@smetana:");
        strcat(prompt, current_path);
        strcat(prompt, "$ ");

        memset(buffer, 0, sizeof(buffer));
        memset(command, 0, sizeof(command));
        memset(args, 0, sizeof(args));
        
        getstr_bound(buffer, strlen(prompt));
        
        // Restore original colors after input
        set_text_color(orig_fore, orig_back);

        if (strlen(buffer) == 0)
            continue;

        // Split input into command and arguments
        int i = 0;
        while (buffer[i] && buffer[i] != ' ' && i < 31) {
            command[i] = buffer[i];
            i++;
        }
        command[i] = '\0';
        
        if (buffer[i] == ' ') {
            i++;
            strcpy(args, buffer + i);
        }

        if(strcmp(command, "cpuid") == 0) {
            cpuid_info(1);
        } else if(strcmp(command, "help") == 0) {
            printf("SIS Smetana Interactive Shell\n");
            printf("These shell commands are defined internally. Type `help' to see this list.\n");
            printf("Type `help name' to find out more about the function `name'.\n");
            printf("Use `info bash' to find out more about the shell in general.\n");
            printf("\n");
            printf(" cpuid           - Display CPU information\n");
            printf(" echo <text>     - Display a line of text\n");
            printf(" clear           - Clear the Smetana screen\n");
            printf(" ls              - List directory contents\n");
            printf(" cd <dir>        - Change the current directory\n");
            printf(" pwd             - Print working directory\n");
            printf(" mkdir <dir>     - Create a new directory\n");
            printf(" color           - Show available colors\n");
            printf(" color <fg> <bg> - Set text color (e.g. color RED BLACK)\n");
            printf(" reset-color     - Reset text color to default\n");
            printf(" shutdown/exit   - Shutdown the system\n");
            printf(" uname           - Display system information\n");
        } else if(strcmp(command, "clear") == 0) {
            console_clear(g_fore_color, g_back_color);
        } else if(strcmp(command, "ls") == 0) {
            fs_ls(args[0] ? args : NULL);
        } else if(strcmp(command, "cd") == 0) {
            if (fs_cd(args) == NULL) {
                printf("cd: %s: No such directory\n", args);
            }
        } else if(strcmp(command, "pwd") == 0) {
            fs_print_working_directory();
        } else if(strcmp(command, "mkdir") == 0) {
            if (!args[0]) {
                printf("mkdir: missing operand\n");
            } else if (!fs_mkdir(args)) {
                printf("mkdir: cannot create directory '%s'\n", args);
            }
        } else if(strcmp(command, "echo") == 0) {
            printf("%s\n", args);
        } else if(strcmp(command, "waver") == 0) {
            printf("Warning, this will lock your system\n");
            printf("Press ESC to exit\n");
            printf(">draw_wave()\n");
            draw_wave();
        } else if(strcmp(command, "color") == 0) {
            print_available_colors();
        } else if(strncmp(command, "color ", 6) == 0) {
            char fg[20], bg[20];
            int pos = 6;
            int i = 0;
            
            // Foregroun
            while(buffer[pos] && buffer[pos] != ' ' && i < 19) {
                fg[i++] = buffer[pos++];
            }
            fg[i] = '\0';
            
            while(buffer[pos] == ' ') pos++;
            
            // Background
            i = 0;
            while(buffer[pos] && buffer[pos] != ' ' && i < 19) {
                bg[i++] = buffer[pos++];
            }
            bg[i] = '\0';

            // Convert color names to values
            VGA_COLOR_TYPE fore_color = COLOR_WHITE;
            VGA_COLOR_TYPE back_color = COLOR_BLACK;

            if(strcmp(fg, "BLACK") == 0) fore_color = COLOR_BLACK;
            else if(strcmp(fg, "BLUE") == 0) fore_color = COLOR_BLUE;
            else if(strcmp(fg, "GREEN") == 0) fore_color = COLOR_GREEN;
            else if(strcmp(fg, "CYAN") == 0) fore_color = COLOR_CYAN;
            else if(strcmp(fg, "RED") == 0) fore_color = COLOR_RED;
            else if(strcmp(fg, "MAGENTA") == 0) fore_color = COLOR_MAGENTA;
            else if(strcmp(fg, "BROWN") == 0) fore_color = COLOR_BROWN;
            else if(strcmp(fg, "GREY") == 0) fore_color = COLOR_GREY;
            else if(strcmp(fg, "DARK_GREY") == 0) fore_color = COLOR_DARK_GREY;
            else if(strcmp(fg, "BRIGHT_BLUE") == 0) fore_color = COLOR_BRIGHT_BLUE;
            else if(strcmp(fg, "BRIGHT_GREEN") == 0) fore_color = COLOR_BRIGHT_GREEN;
            else if(strcmp(fg, "BRIGHT_CYAN") == 0) fore_color = COLOR_BRIGHT_CYAN;
            else if(strcmp(fg, "BRIGHT_RED") == 0) fore_color = COLOR_BRIGHT_RED;
            else if(strcmp(fg, "BRIGHT_MAGENTA") == 0) fore_color = COLOR_BRIGHT_MAGENTA;
            else if(strcmp(fg, "YELLOW") == 0) fore_color = COLOR_YELLOW;
            else if(strcmp(fg, "WHITE") == 0) fore_color = COLOR_WHITE;

            if(strcmp(bg, "BLACK") == 0) back_color = COLOR_BLACK;
            else if(strcmp(bg, "BLUE") == 0) back_color = COLOR_BLUE;
            else if(strcmp(bg, "GREEN") == 0) back_color = COLOR_GREEN;
            else if(strcmp(bg, "CYAN") == 0) back_color = COLOR_CYAN;
            else if(strcmp(bg, "RED") == 0) back_color = COLOR_RED;
            else if(strcmp(bg, "MAGENTA") == 0) back_color = COLOR_MAGENTA;
            else if(strcmp(bg, "BROWN") == 0) back_color = COLOR_BROWN;
            else if(strcmp(bg, "GREY") == 0) back_color = COLOR_GREY;
            else if(strcmp(bg, "DARK_GREY") == 0) back_color = COLOR_DARK_GREY;
            else if(strcmp(bg, "BRIGHT_BLUE") == 0) back_color = COLOR_BRIGHT_BLUE;
            else if(strcmp(bg, "BRIGHT_GREEN") == 0) back_color = COLOR_BRIGHT_GREEN;
            else if(strcmp(bg, "BRIGHT_CYAN") == 0) back_color = COLOR_BRIGHT_CYAN;
            else if(strcmp(bg, "BRIGHT_RED") == 0) back_color = COLOR_BRIGHT_RED;
            else if(strcmp(bg, "BRIGHT_MAGENTA") == 0) back_color = COLOR_BRIGHT_MAGENTA;
            else if(strcmp(bg, "YELLOW") == 0) back_color = COLOR_YELLOW;
            else if(strcmp(bg, "WHITE") == 0) back_color = COLOR_WHITE;

            set_text_color(fore_color, back_color);
        } else if(strcmp(command, "reset-color") == 0) {
            reset_text_color();
        } else if (strcmp(command, "shutdown") == 0 || strcmp(command, "exit") == 0) {
            announce("Shutting down. Bye!\n");
            delay(15);
            shutdown();
        } else if(strcmp(command, "uname") == 0) {
            printf("%s\n", OS_FULL_NAME);
        } else {
            printf("from regular: %s: command not found\n", command);
        }
    }
}



