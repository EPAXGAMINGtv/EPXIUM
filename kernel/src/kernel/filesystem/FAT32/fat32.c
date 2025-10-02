#include "fat32.h"                                              
#include <stdint.h>
#include <stddef.h>
#include "kernel/device/display/fprint.h"                                              

int my_toupper(int c) {
    if (c >= 'a' && c <= 'z') {
        return c - ('a' - 'A');
    }
    return c;
}

size_t my_strlen(const char *str) {
    size_t len = 0;
    while (str[len]) len++;                 
    return len;
}

void *my_memcpy(void *dest, const void *src, size_t n) {
    unsigned char *d = (unsigned char*)dest;
    const unsigned char *s = (const unsigned char*)src;
    for (size_t i = 0; i < n; i++) {
        d[i] = s[i];
    }
    return dest;
}

void *my_memset(void *ptr, int value, size_t num) {
    unsigned char *p = (unsigned char*)ptr;
    for (size_t i = 0; i < num; i++) {
        p[i] = (unsigned char)value;
    }   
    return ptr;
}

fat32_bootsector bootsector;  
uint8_t *fat32_img = "iso_root.img";    
uint32_t cluster_size = 0;   


int fat32_read(uint32_t cluster_number, uint8_t *buffer, size_t size) {
    uint32_t cluster_offset = bootsector.data_start_sector * bootsector.bytes_per_sector +
                              (cluster_number - 2) * cluster_size;

    
    for (size_t i = 0; i < size; i++) {
        buffer[i] = fat32_img[cluster_offset + i];
    }

    return 0;
}

uint32_t fat32_get_next_cluster(uint32_t cluster) {
    uint32_t fat_offset = bootsector.fat_start_sector * bootsector.bytes_per_sector + cluster * 4;
    uint32_t next_cluster = 0;

                                
    for (int i = 0; i < 4; i++) {
        next_cluster |= fat32_img[fat_offset + i] << (i * 8);
    }
 
    return next_cluster & 0x0FFFFFFF; 
}



uint32_t fat32_resolve_path(uint8_t *path, uint8_t *name_out, int *is_dir_out) {
    uint32_t current_cluster = bootsector.root_cluster;
    uint8_t part[12];
    uint8_t *p = path;
    uint8_t *q;

    while (1) {
        q = part;
        while (*p && *p != '/') {
            *q++ = *p++;
        }
        *q = '\0';

        if (*p == '/') p++;

        int this_is_dir = (*p != '\0');
        fat32_dir_entry entry;
        uint32_t entry_cluster;

        if (!fat32_find_entry(current_cluster, part, this_is_dir, &entry, &entry_cluster)) {
            return 0;
        }

        if (this_is_dir) {
            current_cluster = entry_cluster;
        } else {
            *is_dir_out = this_is_dir;
            my_memset(name_out, ' ', 11);

            int len = my_strlen((char*)part);
            int i;
            for (i = 0; i < len && i < 8 && part[i] != '.'; i++) {
                name_out[i] = my_toupper(part[i]);
            }

            if (part[i] == '.') {
                i++;
                for (int j = 0; j < 3 && part[i] && part[i] != '/'; j++, i++) {
                    name_out[8 + j] = my_toupper(part[i]);
                }
            }

            return current_cluster;
        }
    }
}

int fat32_find_entry(uint32_t dir_cluster, uint8_t *name8_3, int want_dir, fat32_dir_entry *out_entry, uint32_t *out_entry_cluster) {
    uint32_t cluster = dir_cluster;
    uint8_t buf[cluster_size];

    while (cluster < 0x0FFFFFF8) {
        fat32_read(cluster, buf, cluster_size);

        int entries_per_cluster = cluster_size / sizeof(fat32_dir_entry);
        fat32_dir_entry *entry = (fat32_dir_entry*)buf;

        for (int i = 0; i < entries_per_cluster; i++) {
            if (entry[i].name[0] == 0x00) {
                // Kein weiteres Verzeichnis/Datei gefunden
                return 0;
            }

            if ((entry[i].attributes & 0x0F) == 0x0F) {
                continue;
            }

            uint8_t match = 1;
            for (int j = 0; j < 11; j++) {
                if (entry[i].name[j] != name8_3[j]) {
                    match = 0;
                    break;
                }
            }

            if (match) {
                *out_entry = entry[i];
                uint32_t high = entry[i].first_cluster_high;
                uint32_t low  = entry[i].first_cluster_low;
                *out_entry_cluster = (high << 16) | low;

                if (want_dir && !(entry[i].attributes & 0x10)) {
                    return 0;  // Keine Datei, aber ein Verzeichnis gewünscht
                }
                if (!want_dir && (entry[i].attributes & 0x10)) {
                    return 0;  // Verzeichnis, aber eine Datei gewünscht
                }

                return 1;  // Treffer gefunden
            }
        }

        cluster = fat32_get_next_cluster(cluster);
    }

    return 0;  // Kein Treffer
}


int fat32_find_free_dir_slot(uint32_t dir_cluster, fat32_dir_entry *free_entry, uint32_t *free_entry_cluster, uint32_t *free_entry_index) {
    uint32_t cluster = dir_cluster;
    uint8_t buf[cluster_size];

    while (cluster < 0x0FFFFFF8) {
        fat32_read(cluster, buf, cluster_size);
        int entries_per_cluster = cluster_size / sizeof(fat32_dir_entry);
        fat32_dir_entry *entry = (fat32_dir_entry*)buf;

        for (int i = 0; i < entries_per_cluster; i++) {
            if (entry[i].name[0] == 0x00 || entry[i].name[0] == 0xE5) {
                *free_entry = entry[i];
                *free_entry_cluster = cluster;
                *free_entry_index = i;
                return 1;
            }
        }

        cluster = fat32_get_next_cluster(cluster);
    }

    return 0;
}

uint32_t fat32_allocate_cluster() {
    extern uint32_t max_cluster_count;
    for (uint32_t c = 2; c < max_cluster_count; c++) {
        uint32_t entry = fat32_read_fat_entry(c);
        if (entry == 0) {
            fat32_write_fat_entry(c, 0x0FFFFFFF);
            return c;
        }
    }

    return 0;
}

void fat32_write_fat_entry(uint32_t cluster, uint32_t value) {
    uint32_t fat_offset = bootsector.fat_start_sector * bootsector.bytes_per_sector +
                          cluster * 4;

    for (int i = 0; i < 4; i++) {
        fat32_img[fat_offset + i] = (value >> (i * 8)) & 0xFF;
    }

    if (bootsector.num_fats > 1) {
        uint32_t fat2_offset = fat_offset + bootsector.fat_size_32 * bootsector.bytes_per_sector;
        for (int i = 0; i < 4; i++) {
            fat32_img[fat2_offset + i] = (value >> (i * 8)) & 0xFF;
        }
    }
}

int fat32_write_cluster(uint32_t cluster_number, uint8_t *data, size_t data_size) {
    if (!fat32_img) return -1;

    uint32_t cluster_offset = bootsector.data_start_sector * bootsector.bytes_per_sector +
                              (cluster_number - 2) * cluster_size;

    for (size_t i = 0; i < data_size; i++) {
        fat32_img[cluster_offset + i] = data[i];
    }

    return 0;
}

int fat32_create_file(uint8_t *path) {
    uint8_t name8_3[11];
    int is_dir;
    uint32_t parent_cluster = fat32_resolve_path(path, name8_3, &is_dir);

    if (parent_cluster == 0) {
        return -1;  
    }

    if (is_dir) {
        return -2;  // It's a directory, not a file
    }

    fat32_dir_entry free_entry;
    uint32_t free_cluster;
    uint32_t free_index;

    if (!fat32_find_free_dir_slot(parent_cluster, &free_entry, &free_cluster, &free_index)) {
        return -3;  // No free slot in the directory
    }

    uint32_t new_cluster = fat32_allocate_cluster();
    if (new_cluster == 0) {
        return -4;  // No free clusters available
    }

    fat32_dir_entry new_entry;
    my_memset(&new_entry, 0, sizeof(new_entry));
    my_memcpy(new_entry.name, name8_3, 11);
    new_entry.attributes = 0x20;  // File attribute
    new_entry.first_cluster_high = (new_cluster >> 16) & 0xFFFF;
    new_entry.first_cluster_low = new_cluster & 0xFFFF;
    new_entry.file_size = 0;

    uint8_t buf[cluster_size];
    fat32_read(free_cluster, buf, cluster_size);
    fat32_dir_entry *entries = (fat32_dir_entry*)buf;
    entries[free_index] = new_entry;
    fat32_write_cluster(free_cluster, buf, cluster_size);

    return 0;
}

int fat32_make_dir(uint8_t *path) {
    uint8_t name8_3[11];
    int is_dir;
    uint32_t parent_cluster = fat32_resolve_path(path, name8_3, &is_dir);

    if (parent_cluster == 0) {
        return -1;  // Path resolution failed
    }

    if (!is_dir) {
        return -2;  // It's a file, not a directory
    }

    fat32_dir_entry free_entry;
    uint32_t free_cluster;
    uint32_t free_index;

    if (!fat32_find_free_dir_slot(parent_cluster, &free_entry, &free_cluster, &free_index)) {
        return -3;  // No free slot in the directory
    }

    uint32_t new_cluster = fat32_allocate_cluster();
    if (new_cluster == 0) {
        return -4;  // No free clusters available
    }

    fat32_dir_entry new_entry;
    my_memset(&new_entry, 0, sizeof(new_entry));
    my_memcpy(new_entry.name, name8_3, 11);
    new_entry.attributes = 0x10;  // Directory attribute
    new_entry.first_cluster_high = (new_cluster >> 16) & 0xFFFF;
    new_entry.first_cluster_low = new_cluster & 0xFFFF;
    new_entry.file_size = 0;

    uint8_t buf[cluster_size];
    fat32_read(free_cluster, buf, cluster_size);
    fat32_dir_entry *entries = (fat32_dir_entry*)buf;
    entries[free_index] = new_entry;
    fat32_write_cluster(free_cluster, buf, cluster_size);

    return 0;
}
