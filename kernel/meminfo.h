#ifndef MEMINFO_H
#define MEMINFO_H

#include <stdint.h>

typedef struct {
    uint64_t total_bytes; 
    uint64_t reserved_bytes;
    uint64_t free_bytes;
    int      mmap_available; 
} meminfo_t;

void meminfo_collect(uint32_t multiboot_ptr, meminfo_t *out);

#endif
