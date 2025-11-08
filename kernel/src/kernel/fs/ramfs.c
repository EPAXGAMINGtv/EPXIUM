#include "ramfs.h"
#include "kernel/device/display/fprint.h"
#include "kernel/mem/kmalloc.h"
#include "kernel/mem/umalloc.h"
#include "kernel/kernel_lib/string.h"

static ramfs_node_t* ramfs_root = NULL;



static char* next_token(char** str) {
    if (!str || !*str || **str == '\0') return NULL;

    char* start = *str;

    
    while (*start == '/') start++;
    if (*start == '\0') {
        *str = start;
        return NULL;
    }

    char* end = start;
    while (*end && *end != '/') end++;

    if (*end == '/') {
        *end = '\0';           
        *str = end + 1;        
    } else {
        *str = end;            
    }

    return start;
}

static ramfs_node_t* ramfs_find_node(const char* path, int create_missing) {
    fprint("[DEBUG] ramfs_find_node: '"); fprint(path); fprint("'\n");

    if (!ramfs_root) {
        fprint("[DEBUG] ramfs_root is NULL!\n");
        return NULL;
    }
    fprint("[DEBUG] ramfs_root OK\n");

    if (!path || path[0] == '\0' || (path[0] == '/' && path[1] == '\0')) {
        fprint("[DEBUG] returning root\n");
        return ramfs_root;
    }

    char temp[256];
    strncpy(temp, path, sizeof(temp) - 1);
    temp[sizeof(temp) - 1] = '\0';
    fprint("[DEBUG] path copy: '"); fprint(temp); fprint("'\n");

    ramfs_node_t* cur = ramfs_root;
    char* p = temp;
    char* token;

    while ((token = next_token(&p))) {
        if (token[0] == '\0') continue;

        fprint("[DEBUG] token: '"); fprint(token); fprint("'\n");

        int found = 0;
        for (size_t i = 0; i < cur->child_count; i++) {
            fprint("[DEBUG]   compare with child["); 
            char buf[16]; itoa(i, buf, 10); fprint(buf); fprint("]: '");
            fprint(cur->children[i]->name); fprint("'\n");

            if (strcmp(cur->children[i]->name, token) == 0) {
                cur = cur->children[i];
                found = 1;
                fprint("[DEBUG]   FOUND!\n");
                break;
            }
        }

        if (!found) {
            fprint("[DEBUG]   NOT FOUND\n");
            if (!create_missing) return NULL;
            ramfs_node_t* new_node = kmalloc(sizeof(ramfs_node_t));
            if (!new_node) {
                fprint("[DEBUG] kmalloc failed for new node!\n");
                return NULL;
            }
            memset(new_node, 0, sizeof(ramfs_node_t));
            strncpy(new_node->name, token, RAMFS_MAX_NAME - 1);
            new_node->name[RAMFS_MAX_NAME - 1] = '\0';
            new_node->type = RAMFS_DIR;
            new_node->parent = cur;
            new_node->child_count = 0;

            if (cur->child_count >= RAMFS_MAX_CHILDREN) {
                fprint("[DEBUG] DIR FULL!\n");
                return NULL;
            }

            cur->children[cur->child_count++] = new_node;
            cur = new_node;
            fprint("[DEBUG]   CREATED new dir: '"); fprint(token); fprint("'\n");
        }
    }

    fprint("[DEBUG] returning node\n");
    return cur;
}
void ramfs_init(void) {
    ramfs_root = kmalloc(sizeof(ramfs_node_t));
    memset(ramfs_root, 0, sizeof(ramfs_node_t));
    ramfs_root->name[0] = '\0';
    ramfs_root->type = RAMFS_DIR;
    ramfs_root->child_count = 0;
    fprint("[RAMFS] Initialized\n");
}

ramfs_node_t* ramfs_mkdir(const char* path) {
    if (!path || !ramfs_root) return NULL;

    ramfs_node_t* node = ramfs_find_node(path, 1);
    if (!node) {
        fprint("[RAMFS] Failed to mkdir: "); fprint(path); fprint("\n");
        return NULL;
    }

    if (node->type != RAMFS_DIR) {
        fprint("[RAMFS] Path exists but not dir: "); fprint(path); fprint("\n");
        return NULL;
    }

    fprint("[RAMFS] mkdir: "); fprint(path); fprint("\n");
    return node;
}

ramfs_node_t* ramfs_create_file(const char* path, const char* data) {
    if (!path || !data || !ramfs_root) return NULL;

    char path_copy[256];
    strncpy(path_copy, path, sizeof(path_copy) - 1);
    path_copy[sizeof(path_copy) - 1] = '\0';

    char* last_slash = strrchr(path_copy, '/');
    if (!last_slash || last_slash == path_copy) return NULL;
    *last_slash = '\0';
    char* filename = last_slash + 1;
    if (filename[0] == '\0') return NULL;

    ramfs_node_t* dir = ramfs_find_node(path_copy, 1);
    if (!dir || dir->type != RAMFS_DIR) return NULL;

    for (size_t i = 0; i < dir->child_count; i++) {
        if (strcmp(dir->children[i]->name, filename) == 0) {
            fprint("[RAMFS] File exists: "); fprint(path); fprint("\n");
            return NULL;
        }
    }

    if (dir->child_count >= RAMFS_MAX_CHILDREN) {
        fprint("[RAMFS] ERR: Dir full\n");
        return NULL;
    }

    ramfs_node_t* file = kmalloc(sizeof(ramfs_node_t));
    memset(file, 0, sizeof(ramfs_node_t));
    strncpy(file->name, filename, RAMFS_MAX_NAME - 1);
    file->name[RAMFS_MAX_NAME - 1] = '\0';
    file->type = RAMFS_FILE;
    file->size = strlen(data);
    file->data = kmalloc(file->size + 1);
    strcpy(file->data, data);
    file->parent = dir;

    dir->children[dir->child_count++] = file;

    fprint("[RAMFS] Created file: "); fprint(path); fprint("\n");
    return file;
}

ramfs_node_t* ramfs_read_file(const char* path) {
    ramfs_node_t* node = ramfs_find_node(path, 0);
    if (!node || node->type != RAMFS_FILE) {
        fprint("[RAMFS] Not found or not file: "); fprint(path); fprint("\n");
        return NULL;
    }

    fprint("[RAMFS] Read: "); fprint(node->name); fprint(" => "); fprint(node->data); fprint("\n");
    return node;
}

void ramfs_list_dir(const char* path) {
    const char* dirpath = path && path[0] ? path : "/";
    ramfs_node_t* dir = ramfs_find_node(dirpath, 0);
    if (!dir || dir->type != RAMFS_DIR) {
        fprint("[RAMFS] Dir not found: "); fprint(dirpath); fprint("\n");
        return;
    }

    fprint("[RAMFS] ls "); fprint(dirpath); fprint("\n");
    for (size_t i = 0; i < dir->child_count; i++) {
        fprint("  ");
        fprint(dir->children[i]->name);
        fprint(dir->children[i]->type == RAMFS_DIR ? "/" : "");
        fprint("\n");
    }
}