#include "pci.h"
#include "io.h"

#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA    0xCFC

static uint32_t pci_config_read32(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset)
{
    uint32_t address = (uint32_t)(
        (1u << 31) |
        ((uint32_t)bus  << 16) |
        ((uint32_t)slot << 11) |
        ((uint32_t)func << 8)  |
        (offset & 0xFC)
    );

    outl(PCI_CONFIG_ADDRESS, address);
    return inl(PCI_CONFIG_DATA);
}

static const char *pci_vendor_name(uint16_t vendor_id)
{
    switch (vendor_id) {
        case 0x8086: return "Intel";
        case 0x10DE: return "NVIDIA";
        case 0x1002: return "AMD/ATI";
        case 0x1234: return "QEMU/Bochs (Standard VGA)";
        case 0x15AD: return "VMware SVGA";
        case 0x1AF4: return "Virtio GPU (Red Hat/QEMU)";
        case 0x80EE: return "VirtualBox VBoxVGA";
        default:     return "Unknown vendor";
    }
}

void pci_find_gpu(gpu_info_t *out)
{
    out->found = 0;
    out->vendor_id = 0;
    out->device_id = 0;
    out->vendor_name = "None";

    for (uint32_t bus = 0; bus < 1; bus++) {
        for (uint32_t slot = 0; slot < 32; slot++) {
            for (uint32_t func = 0; func < 8; func++) {
                uint32_t id_reg = pci_config_read32((uint8_t)bus, (uint8_t)slot, (uint8_t)func, 0x00);
                uint16_t vendor_id = id_reg & 0xFFFF;

                if (vendor_id == 0xFFFF) {
                    if (func == 0) {
                        break; /* nothing at this slot at all */
                    }
                    continue;
                }

                uint32_t class_reg  = pci_config_read32((uint8_t)bus, (uint8_t)slot, (uint8_t)func, 0x08);
                uint8_t  class_code = (class_reg >> 24) & 0xFF;

                if (class_code == 0x03) { /* display controller */
                    out->found = 1;
                    out->vendor_id = vendor_id;
                    out->device_id = (id_reg >> 16) & 0xFFFF;
                    out->vendor_name = pci_vendor_name(vendor_id);
                    return;
                }

                if (func == 0) {
                    uint32_t header_reg  = pci_config_read32((uint8_t)bus, (uint8_t)slot, (uint8_t)func, 0x0C);
                    uint8_t  header_type = (header_reg >> 16) & 0xFF;
                    if (!(header_type & 0x80)) {
                        break; /* single-function device, skip other funcs */
                    }
                }
            }
        }
    }
}
