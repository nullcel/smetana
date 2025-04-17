#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include "types.h"

#define MAX_FILENAME 128
#define MAX_PATH 256
#define MAX_FILES 100
#define MAX_DIRS 50

typedef struct FileNode {
    char name[MAX_FILENAME];
    uint32 size;
    BOOL is_directory;
    struct FileNode* parent;
    struct FileNode* children[MAX_FILES];
    uint32 child_count;
} FileNode;

// Filesystem operations
void fs_init(void);
FileNode* fs_create_node(const char* name, BOOL is_directory);
BOOL fs_mkdir(const char* path);
FileNode* fs_cd(const char* path);
void fs_ls(const char* path);
FileNode* fs_get_current_dir(void);
void fs_print_working_directory(void);

// Path manipulation
char* fs_get_absolute_path(const char* relative_path);
FileNode* fs_path_to_node(const char* path);

#endif