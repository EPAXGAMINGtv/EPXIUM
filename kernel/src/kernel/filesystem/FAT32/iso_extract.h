#ifndef ISO_EXTRACT_H
#define ISO_EXTRACT_H

#include <stdint.h>

#define SECTOR_SIZE 2048

// Struktur für ISO9660-Verzeichniseintrag
typedef struct {
    uint8_t length;
    uint8_t ext_attr_length;
    uint32_t data_size;
    uint32_t data_location;  // Byte Offset im ISO-Image
    char name[255];
} iso_directory_entry;

// Selbst definierte Funktionen
int my_strcmp(const char *s1, const char *s2);
void *my_memcpy(void *dest, const void *src, size_t n);
int my_fseek(uint32_t offset);
size_t my_fread(uint8_t *buffer, size_t size);

int extract_file_from_iso(const char *iso_filename, const char *file_name, uint8_t **out_data, size_t *out_size);

#endif 
