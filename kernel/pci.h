#ifndef PCI_H
#define PCI_H

#include <stdint.h>

typedef struct {
    int found;
    uint16_t vendor_id;
    uint16_t device_id;
    const char *vendor_name;
} gpu_info_t;

/* Scans the PCI bus (legacy 0xCF8/0xCFC config mechanism) for the first
 * display controller (class code 0x03) and fills out its IDs. */
void pci_find_gpu(gpu_info_t *out);

#endif
