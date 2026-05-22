CC      = i686-elf-gcc
AS      = nasm
LD      = i686-elf-gcc

CFLAGS  = -std=gnu99 -ffreestanding -O2 -Wall -Wextra
LDFLAGS = -ffreestanding -O2 -nostdlib -T linker.ld

TARGET  = myos.elf
ISO     = myos.iso
ISO_DIR = isodir

LIMINE_DIR = limine

# all object files
OBJS = boot/guiboot.o \
       kernel/kernel.o \
       gdt/gdt_load.o \
       gdt/gdt.o \
       idt/idt_load.o \
       idt/idt.o \
       drivers/keyboard/keyboard.o \
       drivers/Serial/URAT.o \
	   drivers/Serial/libk/itoa/itoa.o \
       drivers/Serial/libk/kprintf/kprintf.o \
       memory/pmm/pmm.o \
       memory/heap/heap.o \
	   GUI/fb/framebuffer.o \
	   GUI/shapes/shapes.o

all: $(TARGET)

# link everything
$(TARGET): $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $(OBJS) -lgcc

# compile any .c file
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# assemble any .asm file
%.o: %.asm
	$(AS) -f elf32 $< -o $@

# -----------------------------------------------------------------------
# ISO target — builds a bootable ISO using Limine
# Requires: xorriso.exe and limine.exe both in PATH or in limine\ folder
# -----------------------------------------------------------------------
iso: $(TARGET)
	@echo [1/4] Setting up ISO directory structure...
	if not exist $(ISO_DIR)\boot\limine mkdir $(ISO_DIR)\boot\limine
	if not exist $(ISO_DIR)\EFI\BOOT    mkdir $(ISO_DIR)\EFI\BOOT

	@echo [2/4] Copying kernel and Limine files...
	copy /Y $(TARGET)                            $(ISO_DIR)\kernel.elf
	copy /Y $(LIMINE_DIR)\limine.cfg             $(ISO_DIR)\limine.cfg
	copy /Y $(LIMINE_DIR)\limine.sys             $(ISO_DIR)\boot\limine.sys
	copy /Y $(LIMINE_DIR)\limine-cd.bin          $(ISO_DIR)\boot\limine\limine-cd.bin
	copy /Y $(LIMINE_DIR)\limine-cd-efi.bin      $(ISO_DIR)\boot\limine\limine-cd-efi.bin
	copy /Y $(LIMINE_DIR)\BOOTX64.EFI            $(ISO_DIR)\EFI\BOOT\BOOTX64.EFI

	@echo [3/4] Creating ISO with xorriso...
	xorriso -as mkisofs \
		-b boot/limine/limine-cd.bin \
		-no-emul-boot -boot-load-size 4 -boot-info-table \
		--efi-boot boot/limine/limine-cd-efi.bin \
		-efi-boot-part --efi-boot-image \
		--protective-msdos-label \
		-o $(ISO) $(ISO_DIR)

	@echo [4/4] Installing Limine BIOS boot sectors...
	$(LIMINE_DIR)\limine-deploy.exe $(ISO)

	@echo Done! Run: make run-iso

# -----------------------------------------------------------------------
# Run targets
# -----------------------------------------------------------------------

# Quick run — no ISO needed, QEMU loads multiboot directly
# Use this during development for fast iteration
run: $(TARGET)
	qemu-system-i386 -kernel $(TARGET)

# Full ISO run — boots exactly like real hardware would
run-iso: $(ISO)
	qemu-system-i386 -cdrom $(ISO) -m 256M

# Run with debug output on serial port (very useful)
run-debug: $(TARGET)
	qemu-system-i386 -kernel $(TARGET) -serial file:log.txt

# -----------------------------------------------------------------------
# Clean
# -----------------------------------------------------------------------
clean:
	del /Q $(OBJS) $(TARGET) $(ISO) 2>nul || true
	if exist $(ISO_DIR) rmdir /S /Q $(ISO_DIR)

.PHONY: all iso run run-iso run-debug clean