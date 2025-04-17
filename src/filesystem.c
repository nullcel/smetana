#include "filesystem.h"
#include "string.h"
#include "console.h"

static FileNode* root_node = NULL;
static FileNode* current_dir = NULL;

void fs_init(void) {
    root_node = fs_create_node("/", TRUE);
    current_dir = root_node;
}

FileNode* fs_create_node(const char* name, BOOL is_directory) {
    FileNode* node = (FileNode*)malloc(sizeof(FileNode));
    if (!node) return NULL;

    strncpy(node->name, name, MAX_FILENAME);
    node->size = 0;
    node->is_directory = is_directory;
    node->parent = NULL;
    node->child_count = 0;
    memset(node->children, 0, sizeof(node->children));
    
    return node;
}

BOOL fs_mkdir(const char* path) {
    if (!path || strlen(path) == 0) return FALSE;
    
    FileNode* parent = current_dir;
    FileNode* new_dir = fs_create_node(path, TRUE);
    if (!new_dir) return FALSE;

    new_dir->parent = parent;
    if (parent->child_count >= MAX_FILES) {
        // Free the node if we can't add it
        free(new_dir);
        return FALSE;
    }

    parent->children[parent->child_count++] = new_dir;
    return TRUE;
}

FileNode* fs_cd(const char* path) {
    if (!path) return current_dir;
    
    if (strcmp(path, ".") == 0) {
        return current_dir;
    }
    
    if (strcmp(path, "..") == 0) {
        if (current_dir->parent) {
            current_dir = current_dir->parent;
        }
        return current_dir;
    }

    if (strcmp(path, "/") == 0) {
        current_dir = root_node;
        return current_dir;
    }

    // Look for directory in current directory
    for (uint32 i = 0; i < current_dir->child_count; i++) {
        if (strcmp(current_dir->children[i]->name, path) == 0) {
            if (current_dir->children[i]->is_directory) {
                current_dir = current_dir->children[i];
                return current_dir;
            }
            return NULL; // Found but not a directory
        }
    }
    
    return NULL; // Not found
}

void fs_ls(const char* path) {
    FileNode* dir = path ? fs_path_to_node(path) : current_dir;
    if (!dir) {
        printf("Directory not found\n");
        return;
    }

    for (uint32 i = 0; i < dir->child_count; i++) {
        FileNode* node = dir->children[i];
        if (node->is_directory) {
            printf("[DIR]  %s\n", node->name);
        } else {
            printf("       %s\n", node->name);
        }
    }
}

FileNode* fs_get_current_dir(void) {
    return current_dir;
}

void fs_print_working_directory(void) {
    char path[MAX_PATH];
    FileNode* node = current_dir;
    int path_len = 0;

    // Handle root directory
    if (node == root_node) {
        printf("/\n");
        return;
    }

    // Build path from current directory up to root
    while (node != root_node) {
        int name_len = strlen(node->name);
        if (path_len + name_len + 1 >= MAX_PATH) break;
        
        // Shift existing path right
        memmove(path + name_len + 1, path, path_len);
        // Add new directory name
        memcpy(path, node->name, name_len);
        path[name_len] = '/';
        path_len += name_len + 1;
        
        node = node->parent;
    }

    // Add null terminator
    path[path_len] = '\0';
    printf("/%s\n", path);
}

FileNode* fs_path_to_node(const char* path) {
    if (!path || strlen(path) == 0) return current_dir;
    
    if (strcmp(path, "/") == 0) return root_node;
    
    // Start from current directory
    FileNode* current = current_dir;
    char path_copy[MAX_PATH];
    strncpy(path_copy, path, MAX_PATH);
    
    char* token = strtok(path_copy, "/");
    while (token) {
        if (strcmp(token, ".") == 0) {
            // Stay in current directory
        } else if (strcmp(token, "..") == 0) {
            if (current->parent) {
                current = current->parent;
            }
        } else {
            // Look for directory in current directory
            BOOL found = FALSE;
            for (uint32 i = 0; i < current->child_count; i++) {
                if (strcmp(current->children[i]->name, token) == 0) {
                    current = current->children[i];
                    found = TRUE;
                    break;
                }
            }
            if (!found) return NULL;
        }
        token = strtok(NULL, "/");
    }
    
    return current;
}