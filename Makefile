AS = nasm
LD = x86_64-elf-ld
ASFLAGS = -f elf64
LDFLAGS = -n --no-warn-rwx-segments -T linker.ld
CC = x86_64-elf-gcc
CFLAGS = -ffreestanding -fno-stack-protector -fno-pic -fno-pie -mno-red-zone -Wall -Wextra
GRUB_MKRESCUE = i686-elf-grub-mkrescue

all: build/os.iso

build/boot.o: boot/boot.asm
	mkdir -p build
	$(AS) $(ASFLAGS) $< -o $@

build/kernel.o: kernel/kernel.c kernel/cpuid.h kernel/meminfo.h kernel/pci.h kernel/pmm.h
	$(CC) $(CFLAGS) -c $< -o $@

build/tty.o: kernel/tty.c kernel/tty.h
	$(CC) $(CFLAGS) -c $< -o $@

build/gdt.o: kernel/gdt.c kernel/gdt.h
	$(CC) $(CFLAGS) -c $< -o $@

build/gdt_flush.o: kernel/gdt_flush.asm
	$(AS) $(ASFLAGS) $< -o $@

build/idt.o: kernel/idt.c kernel/idt.h
	$(CC) $(CFLAGS) -c $< -o $@

build/idt_flush.o: kernel/idt_flush.asm
	$(AS) $(ASFLAGS) $< -o $@

build/isr.o: kernel/isr.asm
	$(AS) $(ASFLAGS) $< -o $@

build/kprintf.o: kernel/kprintf.c kernel/kprintf.h
	$(CC) $(CFLAGS) -c $< -o $@

build/pic.o: kernel/pic.c kernel/pic.h
	$(CC) $(CFLAGS) -c $< -o $@

build/keyboard.o: kernel/keyboard.c kernel/keyboard.h
	$(CC) $(CFLAGS) -c $< -o $@

build/cpuid.o: kernel/cpuid.c kernel/cpuid.h
	$(CC) $(CFLAGS) -c $< -o $@

build/pci.o: kernel/pci.c kernel/pci.h kernel/io.h
	$(CC) $(CFLAGS) -c $< -o $@

build/meminfo.o: kernel/meminfo.c kernel/meminfo.h kernel/multiboot.h
	$(CC) $(CFLAGS) -c $< -o $@

build/pmm.o: kernel/pmm.c kernel/pmm.h
	$(CC) $(CFLAGS) -c $< -o $@

build/pit.o: kernel/pit.c kernel/pit.h
	$(CC) $(CFLAGS) -c $< -o $@

KOBJS = build/boot.o build/kernel.o build/tty.o build/gdt.o build/gdt_flush.o \
        build/idt.o build/idt_flush.o build/isr.o build/kprintf.o build/pic.o \
        build/keyboard.o build/cpuid.o build/pci.o build/meminfo.o build/pmm.o \
        build/pit.o

build/kernel.elf: $(KOBJS) linker.ld
	$(LD) $(LDFLAGS) $(KOBJS) -o build/kernel.elf

build/os.iso: build/kernel.elf grub.cfg
	mkdir -p build/isodir/boot/grub
	cp build/kernel.elf build/isodir/boot/kernel.elf
	cp grub.cfg build/isodir/boot/grub/grub.cfg
	$(GRUB_MKRESCUE) -o build/os.iso build/isodir -- -as mkisofs -b boot/grub/i386-pc/eltorito.img -no-emul-boot -boot-load-size 4 -boot-info-table
run: build/os.iso
	qemu-system-x86_64 -cdrom build/os.iso -no-reboot -no-shutdown -d int,cpu_reset,guest_errors -D build/qemu.log -display cocoa,zoom-to-fit=on -vga std

clean:
	rm -rf build

.PHONY: all clean run
