#ifndef CPUID_H
#define CPUID_H

#include <stdint.h>

typedef struct {
    uint32_t eax, ebx, ecx, edx;
} cpuid_regs_t;

static inline void cpuid(uint32_t leaf, uint32_t subleaf, cpuid_regs_t *out)
{
    __asm__ volatile (
        "cpuid"
        : "=a"(out->eax), "=b"(out->ebx), "=c"(out->ecx), "=d"(out->edx)
        : "a"(leaf), "c"(subleaf)
    );
}

void cpu_get_vendor(char vendor[13]);

void cpu_get_brand(char brand[49]);

int cpu_logical_count(void);

int cpu_core_count(void);

#endif
