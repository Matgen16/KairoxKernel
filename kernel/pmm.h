#ifndef PMM_H
#define PMM_H

#include <stdint.h>

#define PMM_FRAME_SIZE 4096u

void pmm_init(uint32_t multiboot_ptr);

void *pmm_alloc_frame(void);

void pmm_free_frame(void *phys_addr);

uint64_t pmm_total_frames(void);
uint64_t pmm_free_frames(void);
uint64_t pmm_used_frames(void);

#endif