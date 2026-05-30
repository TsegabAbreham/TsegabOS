CC      = i686-elf-gcc
AS      = nasm
LD      = i686-elf-gcc

# -------------------------------------------------------
# FLAGS
# -------------------------------------------------------
CFLAGS  = -std=gnu99 -ffreestanding -O2 -Wall -Wextra \
          -Iexternal/external/lvgl \
          -DLV_CONF_INCLUDE_SIMPLE

LDFLAGS = -ffreestanding -O2 -nostdlib -T linker.ld

TARGET  = myos.elf
ISO     = myos.iso
ISO_DIR = isodir

LIMINE_DIR = limine


# -------------------------------------------------------
# CORE OS OBJECTS
# -------------------------------------------------------
OBJS = \
	boot/boot.o \
	kernel/kernel.o \
	gdt/gdt_load.o \
	gdt/gdt.o \
	idt/idt_load.o \
	idt/idt.o \
	drivers/keyboard/keyboard.o \
	drivers/Serial/URAT.o \
	drivers/mouse/mouse.o \
	drivers/pit/pit.o \
	drivers/Serial/libk/itoa/itoa.o \
	drivers/Serial/libk/kprintf/kprintf.o \
	drivers/Serial/libk/tools/tools.o \
	drivers/ATA/ata.o \
	memory/pmm/pmm.o \
	memory/heap/heap.o \
	memory/paging/paging.o \
	memory/paging/paging_asm.o \
	GUI/fb/framebuffer.o \
	GUI/bitmap/bitmap.o \
	GUI/UI/UI.o \
	GUI/lvgl_manager/init_lvgl.o

# ---------------------------------------------------------
# Bitmap image OBJECTS
# ---------------------------------------------------------
BMP_OBJS = \
	GUI/bitmap/bitmaps/cursor.o


# -------------------------------------------------------
# FIXED LVGL PATH (IMPORTANT)
# -------------------------------------------------------
LVGL_SRC = $(wildcard external/external/lvgl/src/*.c) \
           $(wildcard external/external/lvgl/src/*/*.c) \
           $(wildcard external/external/lvgl/src/*/*/*.c) \
           $(wildcard external/external/lvgl/src/*/*/*/*.c) \
           $(wildcard external/external/lvgl/src/*/*/*/*/*.c) \
           $(wildcard external/external/lvgl/src/*/*/*/*/*/*.c) \
           $(wildcard external/external/lvgl/src/*/*/*/*/*/*/*.c) \
           $(wildcard external/external/lvgl/*.c)

LVGL_OBJS = $(LVGL_SRC:.c=.o)

OBJS += $(LVGL_OBJS)
OBJS += $(BMP_OBJS)


# -------------------------------------------------------
# BUILD RULES
# -------------------------------------------------------
all: $(TARGET)


$(TARGET): $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $(OBJS) -lgcc


%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.asm
	$(AS) -f elf32 $< -o $@


# -------------------------------------------------------
# ISO BUILD
# -------------------------------------------------------
iso: $(TARGET)
	@echo Building ISO...

	if not exist $(ISO_DIR)\boot\limine mkdir $(ISO_DIR)\boot\limine
	if not exist $(ISO_DIR)\EFI\BOOT mkdir $(ISO_DIR)\EFI\BOOT

	copy /Y $(TARGET) $(ISO_DIR)\kernel.elf
	copy /Y $(LIMINE_DIR)\limine.cfg $(ISO_DIR)\limine.cfg
	copy /Y $(LIMINE_DIR)\limine.sys $(ISO_DIR)\boot\limine.sys
	copy /Y $(LIMINE_DIR)\limine-cd.bin $(ISO_DIR)\boot\limine\limine-cd.bin
	copy /Y $(LIMINE_DIR)\limine-cd-efi.bin $(ISO_DIR)\boot\limine\limine-cd-efi.bin
	copy /Y $(LIMINE_DIR)\BOOTX64.EFI $(ISO_DIR)\EFI\BOOT\BOOTX64.EFI

	xorriso -as mkisofs \
		-b boot/limine/limine-cd.bin \
		-no-emul-boot -boot-load-size 4 -boot-info-table \
		--efi-boot boot/limine/limine-cd-efi.bin \
		-efi-boot-part --efi-boot-image \
		--protective-msdos-label \
		-o $(ISO) $(ISO_DIR)

	$(LIMINE_DIR)\limine-deploy.exe $(ISO)


# -------------------------------------------------------
# RUN
# -------------------------------------------------------
run: $(TARGET)
	qemu-system-i386 -kernel $(TARGET)

run-iso: $(ISO)
	qemu-system-i386 -cdrom $(ISO) -m 256M -hda disk.img

run-debug:
	qemu-system-i386 -kernel $(TARGET) -serial file:log.txt


# -------------------------------------------------------
# CLEAN
# -------------------------------------------------------
clean:
	del /Q $(subst /,\,$(OBJS)) $(TARGET) $(ISO) 2>nul || true
	if exist $(ISO_DIR) rmdir /S /Q $(ISO_DIR)

.PHONY: all iso run run-iso run-debug clean