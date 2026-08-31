#include <stdint.h>
#include "gdt.h"
#include "idt.h"
#include "tty.h"
#include "kprintf.h"
#include "pic.h"
#include "keyboard.h"
#include "cpuid.h"
#include "meminfo.h"
#include "pci.h"
#include "pit.h"
#include "pmm.h"

static void print_mb(uint64_t bytes)
{
    kprintf("%u MB", (unsigned int)(bytes / (1024 * 1024)));
}

static void print_section(const char *title)
{
    tty_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    kprintf("== %s ==\n", title);
    tty_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
}

void kernel_main(uint32_t multiboot_ptr)
{
    gdt_init();
    idt_init();
    tty_init();
    pic_remap(0x20, 0x28);

    tty_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    tty_print("OK\n\n");
    tty_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);

    print_section("cpu info");

    char vendor[13];
    char brand[49];
    cpu_get_vendor(vendor);
    cpu_get_brand(brand);

    int logical = cpu_logical_count();
    int cores   = cpu_core_count();
    if (cores > logical) {
        cores = logical;
    }
    int threads_per_core = (cores > 0) ? (logical / cores) : logical;

    kprintf("%s\n", vendor);
    if (brand[0] != '\0') {
        kprintf("%s\n", brand);
    }

    print_section("mem info");

    meminfo_t mem;
    meminfo_collect(multiboot_ptr, &mem);

    if (mem.total_bytes > 0) {
        kprintf("Total:   ");
        print_mb(mem.total_bytes);
        kprintf("\n");

        kprintf("Free:    ");
        print_mb(mem.free_bytes);
        kprintf("\n\n");
    } else {
        kprintf("no memory map provided by bootloader.\n\n");
    }

    print_section("physical memory manager");

    pmm_init(multiboot_ptr);

    uint64_t total_frames = pmm_total_frames();
    uint64_t free_frames  = pmm_free_frames();
    uint64_t used_frames  = pmm_used_frames();

    kprintf("Frames:  %u total, %u used, %u free\n",
            (unsigned int)total_frames, (unsigned int)used_frames, (unsigned int)free_frames);
    kprintf("Managed: ");
    print_mb(total_frames * PMM_FRAME_SIZE);
    kprintf(" (");
    print_mb(free_frames * PMM_FRAME_SIZE);
    kprintf(" free)\n\n");


    print_section("gpu info");

    gpu_info_t gpu;
    pci_find_gpu(&gpu);

    if (gpu.found) {
        kprintf("%s\n", gpu.vendor_name);
        kprintf("Device:  %04x:%04x\n", gpu.vendor_id, gpu.device_id);
    } else {
        kprintf("no display controller found on pci bus.\n\n");
    }

    keyboard_drain();
    keyboard_init();

    tty_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    kprintf("\n");
    tty_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);

    __asm__ volatile ("sti");

    for (;;) {
        __asm__ volatile ("hlt");
    }
}