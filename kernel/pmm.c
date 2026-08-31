#include "pmm.h"
#include "multiboot.h"

#define PMM_MAX_MANAGED_BYTES  (4ULL * 1024 * 1024 * 1024)
#define PMM_MAX_MANAGED_FRAMES (PMM_MAX_MANAGED_BYTES / PMM_FRAME_SIZE)
#define PMM_BITMAP_SIZE        (PMM_MAX_MANAGED_FRAMES / 8)

static uint8_t bitmap[PMM_BITMAP_SIZE];

static uint64_t total_frames = 0;
static uint64_t free_frame_count = 0;

extern uint8_t _kernel_start;
extern uint8_t _kernel_end;

static inline uint64_t addr_to_frame(uint64_t addr)
{
    return addr / PMM_FRAME_SIZE;
}

static inline int frame_in_range(uint64_t frame)
{
    return frame < PMM_MAX_MANAGED_FRAMES;
}

static inline int bitmap_test(uint64_t frame)
{
    return (bitmap[frame / 8] >> (frame % 8)) & 1;
}

static inline void bitmap_set(uint64_t frame)
{
    bitmap[frame / 8] |= (uint8_t)(1u << (frame % 8));
}

static inline void bitmap_clear(uint64_t frame)
{
    bitmap[frame / 8] &= (uint8_t)~(1u << (frame % 8));
}

static void mark_region_free(uint64_t start, uint64_t end)
{
    uint64_t first = addr_to_frame(start);
    uint64_t last  = addr_to_frame(end + PMM_FRAME_SIZE - 1);
    if (last >= PMM_MAX_MANAGED_FRAMES) {
        last = PMM_MAX_MANAGED_FRAMES - 1;
    }

    for (uint64_t frame = first; frame <= last && frame_in_range(frame); frame++) {
        if ((frame % 8) == 0 && (last - frame) >= 7 && bitmap[frame / 8] == 0xFF) {
            bitmap[frame / 8] = 0x00;
            free_frame_count += 8;
            total_frames += 8;
            frame += 7;
            continue;
        }

        if (bitmap_test(frame)) {
            bitmap_clear(frame);
            free_frame_count++;
            total_frames++;
        }
    }
}
static void mark_region_used(uint64_t start, uint64_t end)
{
    uint64_t first = addr_to_frame(start);
    uint64_t last  = addr_to_frame(end + PMM_FRAME_SIZE - 1);

    for (uint64_t frame = first; frame < last && frame_in_range(frame); frame++) {
        if (!bitmap_test(frame)) {
            bitmap_set(frame);
            free_frame_count--;
        }
    }
}

void pmm_init(uint32_t multiboot_ptr)
{
    for (uint64_t i = 0; i < PMM_BITMAP_SIZE; i++) {
        bitmap[i] = 0xFF;
    }
    total_frames = 0;
    free_frame_count = 0;

    if (multiboot_ptr != 0) {
        multiboot_info_t *mbi = (multiboot_info_t *)(uintptr_t)multiboot_ptr;

        if (mbi->flags & MULTIBOOT_FLAG_MMAP) {
            uint8_t *p   = (uint8_t *)(uintptr_t)mbi->mmap_addr;
            uint8_t *end = p + mbi->mmap_length;

            while (p < end) {
                multiboot_mmap_entry_t *e = (multiboot_mmap_entry_t *)p;
                if (e->type == MULTIBOOT_MEMORY_AVAILABLE && e->len > 0) {
                    mark_region_free(e->addr, e->addr + e->len);
                }
                p += e->size + 4;
            }
        } else if (mbi->flags & MULTIBOOT_FLAG_MEM) {
            uint64_t total_bytes = ((uint64_t)mbi->mem_lower + (uint64_t)mbi->mem_upper) * 1024ULL;
            mark_region_free(0, total_bytes);
        }
    }

    mark_region_used(0x00000000, 0x00100000);
    uint64_t kstart = (uint64_t)(uintptr_t)&_kernel_start;
    uint64_t kend   = (uint64_t)(uintptr_t)&_kernel_end;
    mark_region_used(kstart, kend);
}

void *pmm_alloc_frame(void)
{
    for (uint64_t frame = 0; frame < PMM_MAX_MANAGED_FRAMES; frame++) {
        if (!bitmap_test(frame)) {
            bitmap_set(frame);
            free_frame_count--;
            return (void *)(uintptr_t)(frame * PMM_FRAME_SIZE);
        }
    }
    return (void *)0;
}

void pmm_free_frame(void *phys_addr)
{
    uint64_t frame = addr_to_frame((uint64_t)(uintptr_t)phys_addr);
    if (!frame_in_range(frame)) {
        return;
    }
    if (bitmap_test(frame)) {
        bitmap_clear(frame);
        free_frame_count++;
    }
}

uint64_t pmm_total_frames(void)
{
    return total_frames;
}

uint64_t pmm_free_frames(void)
{
    return free_frame_count;
}

uint64_t pmm_used_frames(void)
{
    return total_frames - free_frame_count;
}