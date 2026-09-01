# =============================================================================
# KiOS Makefile
# =============================================================================

# Compiler und Tools
ASM = nasm
CC = gcc
LD = ld
OBJCOPY = objcopy
QEMU = qemu-system-x86_64

# Verzeichnisse
BOOT_DIR = src/bootloader
KERNEL_DIR = src/kernel
BUILD_DIR = build

# Compiler Flags
CFLAGS = -ffreestanding \
         -fno-pie \
         -fno-stack-protector \
         -mno-red-zone \
         -mgeneral-regs-only \
         -m64 \
         -Wall \
         -Wextra \
         -O2 \
         -I$(KERNEL_DIR) \
         -I$(KERNEL_DIR)/arch \
         -I$(KERNEL_DIR)/drivers \
         -I$(KERNEL_DIR)/fs \
         -I$(KERNEL_DIR)/proc \
         -I$(KERNEL_DIR)/mm \
         -I$(KERNEL_DIR)/lib

# Linker Flags
LDFLAGS = -n \
          -nostdlib \
          -T $(KERNEL_DIR)/linker.ld

# Output
OS_IMAGE = $(BUILD_DIR)/kios.bin

# =============================================================================
# Dateien
# =============================================================================

# Bootloader
STAGE1_SRC = $(BOOT_DIR)/stage1.asm
STAGE1_BIN = $(BUILD_DIR)/stage1.bin

STAGE2_SRC = $(BOOT_DIR)/stage2.asm
STAGE2_BIN = $(BUILD_DIR)/stage2.bin

# Kernel Entry
KERNEL_ENTRY_SRC = $(KERNEL_DIR)/arch/entry.asm
KERNEL_ENTRY_OBJ = $(BUILD_DIR)/entry.o

# IDT Assembly
IDT_ASM_SRC = $(KERNEL_DIR)/arch/idt_asm.asm
IDT_ASM_OBJ = $(BUILD_DIR)/idt_asm.o

# Syscall Assembly
SYSCALL_ASM_SRC = $(KERNEL_DIR)/proc/syscall_asm.asm
SYSCALL_ASM_OBJ = $(BUILD_DIR)/syscall_asm.o

# Task Restore Assembly
TASK_ASM_SRC = $(KERNEL_DIR)/proc/task_asm.asm
TASK_ASM_OBJ = $(BUILD_DIR)/task_asm.o

# Kernel C Sources
KERNEL_C_SRCS = \
    $(KERNEL_DIR)/main.c \
    $(KERNEL_DIR)/shell.c \
    $(KERNEL_DIR)/commands.c \
    $(KERNEL_DIR)/arch/gdt.c \
    $(KERNEL_DIR)/arch/idt.c \
    $(KERNEL_DIR)/arch/isr.c \
    $(KERNEL_DIR)/arch/tss.c \
    $(KERNEL_DIR)/drivers/vga.c \
    $(KERNEL_DIR)/drivers/keyboard_irq.c \
    $(KERNEL_DIR)/drivers/pic.c \
    $(KERNEL_DIR)/drivers/pit.c \
    $(KERNEL_DIR)/fs/vfs.c \
    $(KERNEL_DIR)/fs/devfs.c \
    $(KERNEL_DIR)/proc/task.c \
    $(KERNEL_DIR)/proc/syscall.c \
    $(KERNEL_DIR)/mm/pmm.c \
    $(KERNEL_DIR)/mm/vmm.c \
    $(KERNEL_DIR)/mm/heap.c

KERNEL_C_OBJS = \
    $(BUILD_DIR)/main.o \
    $(BUILD_DIR)/shell.o \
    $(BUILD_DIR)/commands.o \
    $(BUILD_DIR)/arch/gdt.o \
    $(BUILD_DIR)/arch/idt.o \
    $(BUILD_DIR)/arch/isr.o \
    $(BUILD_DIR)/arch/tss.o \
    $(BUILD_DIR)/drivers/vga.o \
    $(BUILD_DIR)/drivers/keyboard_irq.o \
    $(BUILD_DIR)/drivers/pic.o \
    $(BUILD_DIR)/drivers/pit.o \
    $(BUILD_DIR)/fs/vfs.o \
    $(BUILD_DIR)/fs/devfs.o \
    $(BUILD_DIR)/proc/task.o \
    $(BUILD_DIR)/proc/syscall.o \
    $(BUILD_DIR)/mm/pmm.o \
    $(BUILD_DIR)/mm/vmm.o \
    $(BUILD_DIR)/mm/heap.o

# Command-Module automatisch finden
COMMANDS_SRCS = $(wildcard $(KERNEL_DIR)/commands/*.c)
COMMANDS_OBJS = $(patsubst $(KERNEL_DIR)/commands/%.c,$(BUILD_DIR)/commands/%.o,$(COMMANDS_SRCS))

# Alle Kernel Object Files
KERNEL_OBJS = $(KERNEL_ENTRY_OBJ) $(IDT_ASM_OBJ) $(SYSCALL_ASM_OBJ) $(TASK_ASM_OBJ) $(KERNEL_C_OBJS) $(COMMANDS_OBJS)

KERNEL_ELF = $(BUILD_DIR)/kernel.elf
KERNEL_BIN = $(BUILD_DIR)/kernel.bin

# =============================================================================
# Targets
# =============================================================================

.PHONY: all clean run run-debug run-serial debug

all: $(OS_IMAGE)
	@echo ""
	@echo "========================================="
	@echo "  KiOS built successfully!"
	@echo "  Run with: make run"
	@echo "========================================="

# OS Image
$(OS_IMAGE): $(STAGE1_BIN) $(STAGE2_BIN) $(KERNEL_BIN)
	@echo ">>> Creating OS image..."
	cp $(STAGE1_BIN) $(OS_IMAGE)
	cat $(STAGE2_BIN) >> $(OS_IMAGE)
	@SIZE=$$(stat -c%s $(OS_IMAGE)); \
	SECTOR_34=$$((34 * 512)); \
	if [ $$SIZE -lt $$SECTOR_34 ]; then \
		PADDING=$$(($$SECTOR_34 - $$SIZE)); \
		echo "  Adding $$PADDING bytes padding to reach sector 34"; \
		dd if=/dev/zero bs=1 count=$$PADDING >> $(OS_IMAGE) 2>/dev/null; \
	fi
	cat $(KERNEL_BIN) >> $(OS_IMAGE)
	@echo ">>> Image size:"
	@ls -lh $(OS_IMAGE)

# Stage 1
$(STAGE1_BIN): $(STAGE1_SRC) | $(BUILD_DIR)
	@echo ">>> Assembling Stage 1..."
	$(ASM) -f bin $< -o $@

# Stage 2
$(STAGE2_BIN): $(STAGE2_SRC) $(KERNEL_BIN) | $(BUILD_DIR)
	$(eval KERNEL_SECTORS := $(shell stat -c%s $(KERNEL_BIN) 2>/dev/null | awk '{print int(($$1 + 511) / 512)}'))
	@echo ">>> Assembling Stage 2 (Kernel: $(KERNEL_SECTORS) sectors)..."
	$(ASM) -f bin $< -D_KERNEL_SECTORS=$(KERNEL_SECTORS) -o $@

# Assembly
$(KERNEL_ENTRY_OBJ): $(KERNEL_ENTRY_SRC) | $(BUILD_DIR)
	@echo ">>> Assembling Kernel Entry..."
	$(ASM) -f elf64 $< -o $@

$(IDT_ASM_OBJ): $(IDT_ASM_SRC) | $(BUILD_DIR)
	@echo ">>> Assembling IDT stubs..."
	$(ASM) -f elf64 $< -o $@

$(SYSCALL_ASM_OBJ): $(SYSCALL_ASM_SRC) | $(BUILD_DIR)
	@echo ">>> Assembling syscall entry..."
	$(ASM) -f elf64 $< -o $@

$(TASK_ASM_OBJ): $(TASK_ASM_SRC) | $(BUILD_DIR)
	@echo ">>> Assembling task restore..."
	$(ASM) -f elf64 $< -o $@

# C - root
$(BUILD_DIR)/main.o: $(KERNEL_DIR)/main.c | $(BUILD_DIR)
	@echo ">>> Compiling main.c..."
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/shell.o: $(KERNEL_DIR)/shell.c | $(BUILD_DIR)
	@echo ">>> Compiling shell.c..."
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/commands.o: $(KERNEL_DIR)/commands.c | $(BUILD_DIR)
	@echo ">>> Compiling commands.c..."
	$(CC) $(CFLAGS) -c $< -o $@

# C - arch/
$(BUILD_DIR)/arch/gdt.o: $(KERNEL_DIR)/arch/gdt.c | $(BUILD_DIR)/arch
	@echo ">>> Compiling arch/gdt.c..."
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/arch/idt.o: $(KERNEL_DIR)/arch/idt.c | $(BUILD_DIR)/arch
	@echo ">>> Compiling arch/idt.c..."
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/arch/isr.o: $(KERNEL_DIR)/arch/isr.c | $(BUILD_DIR)/arch
	@echo ">>> Compiling arch/isr.c..."
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/arch/tss.o: $(KERNEL_DIR)/arch/tss.c | $(BUILD_DIR)/arch
	@echo ">>> Compiling arch/tss.c..."
	$(CC) $(CFLAGS) -c $< -o $@

# C - drivers/
$(BUILD_DIR)/drivers/vga.o: $(KERNEL_DIR)/drivers/vga.c | $(BUILD_DIR)/drivers
	@echo ">>> Compiling drivers/vga.c..."
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/drivers/keyboard_irq.o: $(KERNEL_DIR)/drivers/keyboard_irq.c | $(BUILD_DIR)/drivers
	@echo ">>> Compiling drivers/keyboard_irq.c..."
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/drivers/pic.o: $(KERNEL_DIR)/drivers/pic.c | $(BUILD_DIR)/drivers
	@echo ">>> Compiling drivers/pic.c..."
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/drivers/pit.o: $(KERNEL_DIR)/drivers/pit.c | $(BUILD_DIR)/drivers
	@echo ">>> Compiling drivers/pit.c..."
	$(CC) $(CFLAGS) -c $< -o $@

# C - fs/
$(BUILD_DIR)/fs/vfs.o: $(KERNEL_DIR)/fs/vfs.c | $(BUILD_DIR)/fs
	@echo ">>> Compiling fs/vfs.c..."
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/fs/devfs.o: $(KERNEL_DIR)/fs/devfs.c | $(BUILD_DIR)/fs
	@echo ">>> Compiling fs/devfs.c..."
	$(CC) $(CFLAGS) -c $< -o $@

# C - proc/
$(BUILD_DIR)/proc/task.o: $(KERNEL_DIR)/proc/task.c | $(BUILD_DIR)/proc
	@echo ">>> Compiling proc/task.c..."
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/proc/syscall.o: $(KERNEL_DIR)/proc/syscall.c | $(BUILD_DIR)/proc
	@echo ">>> Compiling proc/syscall.c..."
	$(CC) $(CFLAGS) -c $< -o $@

# C - mm/
$(BUILD_DIR)/mm/pmm.o: $(KERNEL_DIR)/mm/pmm.c | $(BUILD_DIR)/mm
	@echo ">>> Compiling mm/pmm.c..."
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/mm/vmm.o: $(KERNEL_DIR)/mm/vmm.c | $(BUILD_DIR)/mm
	@echo ">>> Compiling mm/vmm.c..."
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/mm/heap.o: $(KERNEL_DIR)/mm/heap.c | $(BUILD_DIR)/mm
	@echo ">>> Compiling mm/heap.c..."
	$(CC) $(CFLAGS) -c $< -o $@

# Commands
$(BUILD_DIR)/commands/%.o: $(KERNEL_DIR)/commands/%.c | $(BUILD_DIR)/commands
	@echo ">>> Compiling $<..."
	$(CC) $(CFLAGS) -c $< -o $@

# Linken
$(KERNEL_ELF): $(KERNEL_OBJS)
	@echo ">>> Linking Kernel..."
	$(LD) $(LDFLAGS) -o $@ $^

$(KERNEL_BIN): $(KERNEL_ELF)
	@echo ">>> Extracting Kernel Binary..."
	$(OBJCOPY) -O binary $< $@
	@SIZE=$$(stat -c%s $(KERNEL_BIN)); \
	if [ $$SIZE -lt 32768 ]; then \
		dd if=/dev/zero bs=1 count=$$((32768 - $$SIZE)) >> $(KERNEL_BIN) 2>/dev/null; \
	fi

# Build-Verzeichnisse
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/arch:
	mkdir -p $(BUILD_DIR)/arch

$(BUILD_DIR)/drivers:
	mkdir -p $(BUILD_DIR)/drivers

$(BUILD_DIR)/fs:
	mkdir -p $(BUILD_DIR)/fs

$(BUILD_DIR)/proc:
	mkdir -p $(BUILD_DIR)/proc

$(BUILD_DIR)/mm:
	mkdir -p $(BUILD_DIR)/mm

$(BUILD_DIR)/commands:
	mkdir -p $(BUILD_DIR)/commands

# QEMU
run: $(OS_IMAGE)
	@echo ">>> Starting QEMU..."
	$(QEMU) -drive format=raw,file=$(OS_IMAGE) \
	        -m 256M \
	        -monitor stdio \
	        -display sdl,gl=on

run-debug: $(OS_IMAGE)
	@echo ">>> Starting QEMU with debug output..."
	$(QEMU) -drive format=raw,file=$(OS_IMAGE) \
	        -m 256M \
	        -monitor stdio \
	        -display sdl,gl=on \
	        -d int,cpu_reset,guest_errors,exec \
	        -D qemu.log \
	        -no-reboot \
	        -no-shutdown

run-serial: $(OS_IMAGE)
	@echo ">>> Starting QEMU with serial output..."
	$(QEMU) -drive format=raw,file=$(OS_IMAGE) \
	        -m 256M \
	        -serial file:serial.log \
	        -monitor stdio

debug: $(OS_IMAGE)
	@echo ">>> Starting QEMU with GDB server..."
	$(QEMU) -drive format=raw,file=$(OS_IMAGE) \
	        -m 256M \
	        -s -S \
	        -monitor stdio

clean:
	rm -rf $(BUILD_DIR)

disasm-kernel: $(KERNEL_ELF)
	objdump -d $< | head -200

hexdump: $(OS_IMAGE)
	hexdump -C $< | head -100