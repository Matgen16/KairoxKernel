#include <stddef.h>
#include "cpuid.h"

void cpu_get_vendor(char vendor[13])
{
    cpuid_regs_t r;
    cpuid(0, 0, &r);

    uint32_t *v = (uint32_t *)vendor;
    v[0] = r.ebx;
    v[1] = r.edx;
    v[2] = r.ecx;
    vendor[12] = '\0';
}

void cpu_get_brand(char brand[49])
{
    cpuid_regs_t r;
    cpuid(0x80000000, 0, &r);

    if (r.eax < 0x80000004) {
        brand[0] = '\0';
        return;
    }

    uint32_t *b = (uint32_t *)brand;

    cpuid(0x80000002, 0, &r);
    b[0] = r.eax; b[1] = r.ebx; b[2] = r.ecx; b[3] = r.edx;

    cpuid(0x80000003, 0, &r);
    b[4] = r.eax; b[5] = r.ebx; b[6] = r.ecx; b[7] = r.edx;

    cpuid(0x80000004, 0, &r);
    b[8] = r.eax; b[9] = r.ebx; b[10] = r.ecx; b[11] = r.edx;

    brand[48] = '\0';

    /* Brand strings are padded with leading spaces; trim them. */
    size_t start = 0;
    while (brand[start] == ' ') {
        start++;
    }
    if (start > 0) {
        size_t i = 0;
        while (brand[start + i] != '\0') {
            brand[i] = brand[start + i];
            i++;
        }
        brand[i] = '\0';
    }
}

int cpu_logical_count(void)
{
    cpuid_regs_t r;
    cpuid(1, 0, &r);

    int htt = (r.edx >> 28) & 1;
    if (!htt) {
        return 1;
    }

    int logical = (int)((r.ebx >> 16) & 0xFF);
    return logical > 0 ? logical : 1;
}

int cpu_core_count(void)
{
    char vendor[13];
    cpu_get_vendor(vendor);

    cpuid_regs_t r;
    cpuid(0, 0, &r);
    uint32_t max_leaf = r.eax;

    if (vendor[0] == 'G') { /* "GenuineIntel" */
        if (max_leaf >= 4) {
            cpuid(4, 0, &r);
            int cores = (int)((r.eax >> 26) & 0x3F) + 1;
            return cores > 0 ? cores : 1;
        }
    } else { /* AMD and most others expose this extended leaf */
        cpuid(0x80000000, 0, &r);
        uint32_t max_ext = r.eax;
        if (max_ext >= 0x80000008) {
            cpuid(0x80000008, 0, &r);
            int cores = (int)(r.ecx & 0xFF) + 1;
            return cores > 0 ? cores : 1;
        }
    }

    return 1;
}
