CC      = i686-elf-gcc
AS      = nasm
LD      = i686-elf-gcc

CFLAGS  = -std=gnu99 -ffreestanding -O2 -Wall -Wextra
LDFLAGS = -ffreestanding -O2 -nostdlib -T linker.ld

TARGET  = myos.bin

# all object files
OBJS = boot/boot.o \
       kernel/kernel.o \
       gdt/gdt_load.o \
       gdt/gdt.o \
       idt/idt_load.o \
       idt/idt.o \
       drivers/keyboard/keyboard.o \
       drivers/VGA/VGA.o \
       memory/pmm/pmm.o \
       memory/heap/heap.o \
       libk/itoa/itoa.o \
       libk/kprintf/kprintf.o

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

run: $(TARGET)
	qemu-system-i386 -kernel $(TARGET)

clean:
	del /Q $(OBJS) $(TARGET) 2>nul || true

.PHONY: all run clean