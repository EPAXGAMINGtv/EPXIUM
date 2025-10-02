#ifndef FAT32_H
#define FAT32_H

#include <stdint.h>
#include <stddef.h>

// FAT32 Boot Sector Structure
typedef struct {
    uint8_t  jump[3];                // Boot Code Jump
    char     oem[8];                 // OEM Name (e.g., "MSWIN4.1")
    uint16_t bytes_per_sector;       // Bytes per sector (typically 512)
    uint8_t  sectors_per_cluster;    // Sectors per cluster
    uint16_t reserved_sectors;       // Reserved sectors (FAT region)
    uint8_t  num_fats;               // Number of FATs (typically 2)
    uint16_t root_entries;           // Number of entries in root directory
    uint16_t total_sectors_small;    // Total number of sectors (small)
    uint8_t  media_type;             // Media type (e.g., 0xF8 for hard drive)
    uint16_t fat_size;               // Size of a single FAT (in sectors)
    uint16_t sectors_per_track;      // Sectors per track (for heads and tracks)
    uint16_t num_heads;              // Number of heads
    uint32_t hidden_sectors;         // Number of hidden sectors
    uint32_t total_sectors_large;    // Total large number of sectors
    uint32_t fat_start_sector;       // Start sector of FAT
    uint32_t data_start_sector;      // Start sector of data area
    uint32_t fat_size_32;            // Size of FAT for FAT32 (in sectors)
    uint16_t ext_flags;              // Extended flags
    uint16_t fs_version;             // Filesystem version
    uint32_t root_cluster;           // Root directory cluster
    uint16_t fs_info;                // FS info sector
    uint16_t backup_bootsector;      // Backup boot sector
    uint8_t  reserved[12];           // Reserved bytes
    uint8_t  drive_number;           // Drive number
    uint8_t  reserved1;              // Reserved
    uint8_t  boot_signature;         // Boot signature
    uint32_t volume_id;              // Volume ID
    char     volume_label[11];       // Volume label
    char     fs_type[8];             // Filesystem type (e.g., "FAT32   ")
} fat32_bootsector;

// FAT32 Directory Entry Structure
typedef struct {
    uint8_t  name[11];              // 8.3 Name of the directory entry
    uint8_t  attributes;            // Attributes (e.g., hidden, read-only)
    uint8_t  reserved;              // Reserved
    uint8_t  creation_time_ms;      // Milliseconds of creation time
    uint16_t creation_time;        // Creation time
    uint16_t creation_date;        // Creation date
    uint16_t last_access_date;     // Last access date
    uint16_t first_cluster_high;   // High bits of first cluster
    uint16_t modified_time;        // Modification time
    uint16_t modified_date;        // Modification date
    uint16_t first_cluster_low;    // Low bits of first cluster
    uint32_t file_size;            // File size
} fat32_dir_entry;

// External global variables defined in fat32.c
extern fat32_bootsector bootsector;  // FAT32 Boot sector
extern uint8_t *fat32_img;           // Pointer to the FAT32 image
extern uint32_t cluster_size;        // Cluster size (in bytes)

// Function declarations for FAT32 operations
int fat32_open(uint8_t *image_buffer);
void fat32_close();
int fat32_get_file(uint8_t *filename11, uint8_t *buffer, size_t buffer_size);
int fat32_read(uint32_t cluster_number, uint8_t *buffer, size_t buffer_size);
int fat32_write_cluster(uint32_t cluster_number, uint8_t *data, size_t data_size);
uint32_t fat32_get_next_cluster(uint32_t cluster);
void fat32_write_fat_entry(uint32_t cluster, uint32_t value);
uint32_t fat32_allocate_cluster();
int fat32_create_file(uint8_t *path);
int fat32_make_dir(uint8_t *path);
int fat32_delete_file(uint8_t *path);
uint32_t fat32_resolve_path(uint8_t *path, uint8_t *name_out11, int *is_dir_out);
int fat32_find_entry(uint32_t dir_cluster, uint8_t *name11, int want_dir, fat32_dir_entry *out_entry, uint32_t *out_cluster);
uint32_t fat32_read_fat_entry(uint32_t cluster);
int fat32_find_free_dir_slot(uint32_t dir_cluster, fat32_dir_entry *free_entry, uint32_t *free_entry_cluster, uint32_t *free_entry_index);

#endif // FAT32_H
