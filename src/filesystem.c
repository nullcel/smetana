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
    if (!name) return NULL;

    FileNode* node = (FileNode*)malloc(sizeof(FileNode));
    if (!node) return NULL;

    strncpy(node->name, name, MAX_FILENAME - 1);
    node->name[MAX_FILENAME - 1] = '\0'; // nulltermination
    node->size = 0;
    node->is_directory = is_directory;
    node->parent = NULL;
    node->child_count = 0;
    memset(node->children, 0, sizeof(node->children));

    return node;
}

BOOL fs_mkdir(const char* path) {
    if (!path || !*path) return FALSE;

    if (current_dir->child_count >= MAX_FILES) return FALSE;

    FileNode* new_dir = fs_create_node(path, TRUE);
    if (!new_dir) return FALSE;

    new_dir->parent = current_dir;
    current_dir->children[current_dir->child_count++] = new_dir;
    return TRUE;
}

FileNode* fs_cd(const char* path) {
    if (!path || strcmp(path, ".") == 0) return current_dir;

    if (strcmp(path, "..") == 0) {
        if (current_dir->parent) current_dir = current_dir->parent;
        return current_dir;
    }

    if (strcmp(path, "/") == 0) {
        current_dir = root_node;
        return current_dir;
    }

    for (uint32 i = 0; i < current_dir->child_count; i++) {
        FileNode* child = current_dir->children[i];
        if (strcmp(child->name, path) == 0 && child->is_directory) {
            current_dir = child;
            return current_dir;
        }
    }

    return NULL;
}

void fs_ls(const char* path) {
    FileNode* dir = path ? fs_path_to_node(path) : current_dir;
    if (!dir) {
        printf("Directory not found\n");
        return;
    }

    for (uint32 i = 0; i < dir->child_count; i++) {
        FileNode* node = dir->children[i];
        printf("%s  %s\n", node->is_directory ? "[DIR]" : "     ", node->name);
    }
}

FileNode* fs_get_current_dir(void) {
    return current_dir;
}

void fs_print_working_directory(void) {
    if (current_dir == root_node) {
        printf("/\n");
        return;
    }

    char path[MAX_PATH] = {0};
    int path_len = 0;
    FileNode* node = current_dir;

    while (node != root_node) {
        int name_len = strlen(node->name);
        if (path_len + name_len + 1 >= MAX_PATH) break;

        memmove(path + name_len + 1, path, path_len);
        memcpy(path, node->name, name_len);
        path[name_len] = '/';
        path_len += name_len + 1;

        node = node->parent;
    }

    path[path_len] = '\0';
    printf("/%s\n", path);
}

FileNode* fs_path_to_node(const char* path) {
    if (!path || !*path) return current_dir;
    if (strcmp(path, "/") == 0) return root_node;

    FileNode* current = current_dir;
    char path_copy[MAX_PATH];
    strncpy(path_copy, path, MAX_PATH - 1);
    path_copy[MAX_PATH - 1] = '\0';

    char* token = strtok(path_copy, "/");
    while (token) {
        if (strcmp(token, "..") == 0) {
            if (current->parent) current = current->parent;
        } else if (strcmp(token, ".") != 0) {
            BOOL found = FALSE;
            for (uint32 i = 0; i < current->child_count; i++) {
                FileNode* child = current->children[i];
                if (strcmp(child->name, token) == 0) {
                    current = child;
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
