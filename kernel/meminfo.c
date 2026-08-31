#include "meminfo.h"
#include "multiboot.h"

extern uint8_t _kernel_start;
extern uint8_t _kernel_end;

void meminfo_collect(uint32_t multiboot_ptr, meminfo_t *out)
{
    out->total_bytes = 0;
    out->reserved_bytes = 0;
    out->free_bytes = 0;
    out->mmap_available = 0;

    if (multiboot_ptr != 0) {
        multiboot_info_t *mbi = (multiboot_info_t *)(uintptr_t)multiboot_ptr;

        if (mbi->flags & MULTIBOOT_FLAG_MMAP) {
            out->mmap_available = 1;

            uint8_t *p   = (uint8_t *)(uintptr_t)mbi->mmap_addr;
            uint8_t *end = p + mbi->mmap_length;

            while (p < end) {
                multiboot_mmap_entry_t *e = (multiboot_mmap_entry_t *)p;
                if (e->type == MULTIBOOT_MEMORY_AVAILABLE) {
                    out->total_bytes += e->len;
                }
                p += e->size + 4;
            }
        } else if (mbi->flags & MULTIBOOT_FLAG_MEM) {
            out->total_bytes = ((uint64_t)mbi->mem_lower + (uint64_t)mbi->mem_upper) * 1024ULL;
        }
    }

    uint64_t kernel_start = (uint64_t)(uintptr_t)&_kernel_start;
    uint64_t kernel_end   = (uint64_t)(uintptr_t)&_kernel_end;
    uint64_t kernel_size  = kernel_end - kernel_start;

    out->reserved_bytes = (1ULL * 1024 * 1024) + kernel_size;

    if (out->total_bytes > out->reserved_bytes) {
        out->free_bytes = out->total_bytes - out->reserved_bytes;
    } else {
        out->free_bytes = 0;
    }
}
