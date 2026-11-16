CC = gcc
AS = nasm
LD = ld

KERNEL_SECTORS = 96

CFLAGS  = -m32 -ffreestanding -fno-pie -fno-pic -fno-stack-protector -fno-builtin \
          -nostdlib -O2 -Wall -Wextra -std=gnu11 -Ikernel/include
LDFLAGS = -m elf_i386 -nostdlib -z noexecstack

BUILD = build

ASM_SRCS = kernel/src/kernel_entry.asm \
           kernel/drivers/vga_putchar.asm \
           kernel/drivers/inb.asm kernel/drivers/outb.asm \
           kernel/drivers/isrstub.asm kernel/drivers/irqstub.asm
C_SRCS   = kernel/src/kernel.c \
           kernel/init/init.c \
           kernel/tty/tty.c \
           kernel/fs/fs.c kernel/fs/ramfs.c kernel/fs/fm.c kernel/fs/fm_glue.c \
           kernel/proc/process.c \
           kernel/lib/src/kstdio.c kernel/lib/src/kstring.c \
           kernel/drivers/vga.c kernel/drivers/serial.c kernel/drivers/keyboard.c \
           kernel/drivers/idt.c kernel/drivers/isr.c kernel/drivers/pic.c \
           kernel/drivers/clock.c \
           kernel/shell/kshell.c kernel/shell/interpreter.c kernel/shell/command.c \
           kernel/shell/commandlist.c kernel/shell/tokens.c

OBJS = $(addprefix $(BUILD)/,$(notdir $(ASM_SRCS:.asm=.o) $(C_SRCS:.c=.o)))
HDRS = $(wildcard kernel/include/*.h)

vpath %.asm $(sort $(dir $(ASM_SRCS)))
vpath %.c   $(sort $(dir $(C_SRCS)))

.PHONY: all clean run run-serial
all: $(BUILD)/os.img

$(BUILD):
	mkdir -p $(BUILD)

$(BUILD)/boot.bin: boot/boot.asm | $(BUILD)
	$(AS) -f bin -DKERNEL_SECTORS=$(KERNEL_SECTORS) $< -o $@

$(BUILD)/%.o: %.asm | $(BUILD)
	$(AS) -f elf32 $< -o $@

$(BUILD)/%.o: %.c $(HDRS) | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/kernel.elf: $(OBJS) linker.ld
	$(LD) $(LDFLAGS) -T linker.ld $(OBJS) -o $@

$(BUILD)/kernel.bin: $(BUILD)/kernel.elf
	objcopy -O binary $< $@
	@size=$$(stat -c%s $@); max=$$(( $(KERNEL_SECTORS) * 512 )); \
	if [ $$size -gt $$max ]; then \
	  echo "kernel.bin ($$size bytes) exceeds $$max bytes: raise KERNEL_SECTORS"; exit 1; fi

# boot sector + kernel, padded so the BIOS can always read KERNEL_SECTORS sectors
$(BUILD)/os.img: $(BUILD)/boot.bin $(BUILD)/kernel.bin
	dd if=/dev/zero of=$@ bs=512 count=$$(( $(KERNEL_SECTORS) + 1 )) 2>/dev/null
	dd if=$(BUILD)/boot.bin   of=$@ conv=notrunc 2>/dev/null
	dd if=$(BUILD)/kernel.bin of=$@ bs=512 seek=1 conv=notrunc 2>/dev/null

clean:
	rm -rf $(BUILD)

run: $(BUILD)/os.img
	qemu-system-i386 -drive format=raw,file=$(BUILD)/os.img

# same as run, but mirrors the console to your terminal
run-serial: $(BUILD)/os.img
	qemu-system-i386 -drive format=raw,file=$(BUILD)/os.img -serial stdio
