
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
BOOT_DIR = src/bootloader/new
KERNEL_DIR = src/kernel
BUILD_DIR = build

# Compiler Flags
# -ffreestanding: Keine Standard-Bibliothek
# -fno-pie: Kein Position Independent Code
# -fno-stack-protector: Keine Stack-Schutz-Funktionen (brauchen libc)
# -mno-red-zone: Kein Red Zone (wichtig für Interrupt Handler)
# -mgeneral-regs-only: KEINE SSE/AVX/FPU Instruktionen (verhindert Crashes!)
# -m64: 64-Bit Code generieren
CFLAGS = -ffreestanding \
         -fno-pie \
         -fno-stack-protector \
         -mno-red-zone \
         -mgeneral-regs-only \
         -m64 \
         -Wall \
         -Wextra \
         -O2 \
         -I$(KERNEL_DIR)

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

# Kernel
KERNEL_ENTRY_SRC = $(KERNEL_DIR)/entry.asm
KERNEL_ENTRY_OBJ = $(BUILD_DIR)/entry.o

# Ergänze tss.c und gdt.c
KERNEL_C_SRCS = $(KERNEL_DIR)/main.c $(KERNEL_DIR)/shell.c $(KERNEL_DIR)/commands.c $(KERNEL_DIR)/vga.c $(KERNEL_DIR)/idt.c $(KERNEL_DIR)/isr.c $(KERNEL_DIR)/pic.c $(KERNEL_DIR)/keyboard_irq.c $(KERNEL_DIR)/tss.c $(KERNEL_DIR)/gdt.c $(KERNEL_DIR)/mm/pmm.c $(KERNEL_DIR)/mm/vmm.c
KERNEL_C_OBJS = $(BUILD_DIR)/main.o $(BUILD_DIR)/shell.o $(BUILD_DIR)/commands.o $(BUILD_DIR)/vga.o $(BUILD_DIR)/idt.o $(BUILD_DIR)/isr.o $(BUILD_DIR)/pic.o $(BUILD_DIR)/keyboard_irq.o $(BUILD_DIR)/tss.o $(BUILD_DIR)/gdt.o $(BUILD_DIR)/mm/pmm.o $(BUILD_DIR)/mm/vmm.o

# IDT Assembly
IDT_ASM_SRC = $(KERNEL_DIR)/idt_asm.asm
IDT_ASM_OBJ = $(BUILD_DIR)/idt_asm.o

# Alle Command-Module automatisch finden
COMMANDS_SRCS = $(wildcard $(KERNEL_DIR)/commands/*.c)
COMMANDS_OBJS = $(patsubst $(KERNEL_DIR)/commands/%.c,$(BUILD_DIR)/commands/%.o,$(COMMANDS_SRCS))

# Alle Kernel Object Files
KERNEL_OBJS = $(KERNEL_ENTRY_OBJ) $(IDT_ASM_OBJ) $(KERNEL_C_OBJS) $(COMMANDS_OBJS)

KERNEL_ELF = $(BUILD_DIR)/kernel.elf
KERNEL_BIN = $(BUILD_DIR)/kernel.bin

# =============================================================================
# Targets
# =============================================================================

.PHONY: all clean run debug

all: $(OS_IMAGE)
	@echo ""
	@echo "========================================="
	@echo "  KiOS built successfully!"
	@echo "  Run with: make run"
	@echo "========================================="

# Finales OS Image zusammenbauen
# Layout: [Stage1: 512B][Stage2: 16KB][Padding][Kernel: 32KB+]
$(OS_IMAGE): $(STAGE1_BIN) $(STAGE2_BIN) $(KERNEL_BIN)
	@echo ">>> Creating OS image..."
	@# Stage 1 (512 Bytes, Sektor 0)
	cp $(STAGE1_BIN) $(OS_IMAGE)
	@# Stage 2 (wird automatisch gepadded von stage2.asm)
	cat $(STAGE2_BIN) >> $(OS_IMAGE)
	@# Padding hinzufügen, damit Kernel bei Sektor 34 startet
	@SIZE=$$(stat -c%s $(OS_IMAGE)); \
	SECTOR_34=$$((34 * 512)); \
	if [ $$SIZE -lt $$SECTOR_34 ]; then \
		PADDING=$$(($$SECTOR_34 - $$SIZE)); \
		echo "  Adding $$PADDING bytes padding to reach sector 34"; \
		dd if=/dev/zero bs=1 count=$$PADDING >> $(OS_IMAGE) 2>/dev/null; \
	fi
	@# Kernel
	cat $(KERNEL_BIN) >> $(OS_IMAGE)
	@# Auf 1.44MB Floppy-Größe auffüllen (optional, für Kompatibilität)
	@# truncate -s 1474560 $(OS_IMAGE)
	@echo ">>> Image size:"
	@ls -lh $(OS_IMAGE)

# Stage 1 Bootloader
$(STAGE1_BIN): $(STAGE1_SRC) | $(BUILD_DIR)
	@echo ">>> Assembling Stage 1..."
	$(ASM) -f bin $< -o $@

# Stage 2 Bootloader
$(STAGE2_BIN): $(STAGE2_SRC) | $(BUILD_DIR)
	@echo ">>> Assembling Stage 2..."
	$(ASM) -f bin $< -o $@

# Kernel Entry (Assembly)
$(KERNEL_ENTRY_OBJ): $(KERNEL_ENTRY_SRC) | $(BUILD_DIR)
	@echo ">>> Assembling Kernel Entry..."
	$(ASM) -f elf64 $< -o $@

# IDT Assembly
$(IDT_ASM_OBJ): $(IDT_ASM_SRC) | $(BUILD_DIR)
	@echo ">>> Assembling IDT stubs..."
	$(ASM) -f elf64 $< -o $@

# Kernel C Code - main.c
$(BUILD_DIR)/main.o: $(KERNEL_DIR)/main.c | $(BUILD_DIR)
	@echo ">>> Compiling main.c..."
	$(CC) $(CFLAGS) -c $< -o $@

# Kernel C Code - shell.c
$(BUILD_DIR)/shell.o: $(KERNEL_DIR)/shell.c | $(BUILD_DIR)
	@echo ">>> Compiling shell.c..."
	$(CC) $(CFLAGS) -c $< -o $@

# Kernel C Code - commands.c
$(BUILD_DIR)/commands.o: $(KERNEL_DIR)/commands.c | $(BUILD_DIR)
	@echo ">>> Compiling commands.c..."
	$(CC) $(CFLAGS) -c $< -o $@

# Kernel C Code - vga.c
$(BUILD_DIR)/vga.o: $(KERNEL_DIR)/vga.c | $(BUILD_DIR)
	@echo ">>> Compiling vga.c..."
	$(CC) $(CFLAGS) -c $< -o $@

# Kernel C Code - idt.c
$(BUILD_DIR)/idt.o: $(KERNEL_DIR)/idt.c | $(BUILD_DIR)
	@echo ">>> Compiling idt.c..."
	$(CC) $(CFLAGS) -c $< -o $@

# Kernel C Code - isr.c
$(BUILD_DIR)/isr.o: $(KERNEL_DIR)/isr.c | $(BUILD_DIR)
	@echo ">>> Compiling isr.c..."
	$(CC) $(CFLAGS) -c $< -o $@

# Kernel C Code - pic.c
$(BUILD_DIR)/pic.o: $(KERNEL_DIR)/pic.c | $(BUILD_DIR)
	@echo ">>> Compiling pic.c..."
	$(CC) $(CFLAGS) -c $< -o $@

# Kernel C Code - keyboard_irq.c
$(BUILD_DIR)/keyboard_irq.o: $(KERNEL_DIR)/keyboard_irq.c | $(BUILD_DIR)
	@echo ">>> Compiling keyboard_irq.c..."
	$(CC) $(CFLAGS) -c $< -o $@

# tss.o
$(BUILD_DIR)/tss.o: src/kernel/tss.c src/kernel/tss.h | $(BUILD_DIR)
	@echo ">>> Compiling tss.c..."
	$(CC) $(CFLAGS) -c src/kernel/tss.c -o $(BUILD_DIR)/tss.o

# gdt.o
$(BUILD_DIR)/gdt.o: src/kernel/gdt.c src/kernel/gdt.h | $(BUILD_DIR)
	@echo ">>> Compiling gdt.c..."
	$(CC) $(CFLAGS) -c src/kernel/gdt.c -o $(BUILD_DIR)/gdt.o

# pmm.o
$(BUILD_DIR)/mm/pmm.o: src/kernel/mm/pmm.c src/kernel/mm/pmm.h | $(BUILD_DIR)/mm
	@echo ">>> Compiling pmm.c..."
	$(CC) $(CFLAGS) -c src/kernel/mm/pmm.c -o $(BUILD_DIR)/mm/pmm.o

$(BUILD_DIR)/mm/vmm.o: src/kernel/mm/vmm.c src/kernel/mm/vmm.h | $(BUILD_DIR)/mm
	@echo ">>> Compiling vmm.c..."
	$(CC) $(CFLAGS) -c src/kernel/mm/vmm.c -o $(BUILD_DIR)/mm/vmm.o

# Command modules
$(BUILD_DIR)/commands/%.o: $(KERNEL_DIR)/commands/%.c | $(BUILD_DIR)/commands
	@echo ">>> Compiling $<..."
	$(CC) $(CFLAGS) -c $< -o $@

# Kernel ELF linken
$(KERNEL_ELF): $(KERNEL_OBJS)
	@echo ">>> Linking Kernel..."
	$(LD) $(LDFLAGS) -o $@ $^

# Kernel Binary extrahieren
$(KERNEL_BIN): $(KERNEL_ELF)
	@echo ">>> Extracting Kernel Binary..."
	$(OBJCOPY) -O binary $< $@
	@# Auf mindestens 32KB auffüllen
	@SIZE=$$(stat -c%s $(KERNEL_BIN)); \
	if [ $$SIZE -lt 32768 ]; then \
		dd if=/dev/zero bs=1 count=$$((32768 - $$SIZE)) >> $(KERNEL_BIN) 2>/dev/null; \
	fi

# Build-Verzeichnis erstellen
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/commands:
	mkdir -p $(BUILD_DIR)/commands

$(BUILD_DIR)/mm:
	mkdir -p $(BUILD_DIR)/mm

# QEMU starten
run: $(OS_IMAGE)
	@echo ">>> Starting QEMU..."
	$(QEMU) -drive format=raw,file=$(OS_IMAGE) \
	        -m 256M \
	        -monitor stdio \
	        -display sdl,gl=on

# QEMU mit Debug-Ausgabe (CPU-Register, Interrupts, etc.)
run-debug: $(OS_IMAGE)
	@echo ">>> Starting QEMU with debug output..."
	@echo ">>> Log wird nach qemu.log geschrieben"
	@echo ">>> Verfügbare -d Flags: int,cpu_reset,guest_errors,exec,cpu"
	$(QEMU) -drive format=raw,file=$(OS_IMAGE) \
	        -m 256M \
	        -monitor stdio \
	        -display sdl,gl=on \
	        -d int,cpu_reset,guest_errors,exec \
	        -D qemu.log \
	        -no-reboot \
	        -no-shutdown

# QEMU mit serieller Konsole (für printf-style debugging)
run-serial: $(OS_IMAGE)
	@echo ">>> Starting QEMU with serial output..."
	$(QEMU) -drive format=raw,file=$(OS_IMAGE) \
	        -m 256M \
	        -serial file:serial.log \
	        -monitor stdio

# QEMU mit GDB Debug Server
debug: $(OS_IMAGE)
	@echo ">>> Starting QEMU with GDB server..."
	@echo ">>> Connect with: gdb -ex 'target remote localhost:1234'"
	$(QEMU) -drive format=raw,file=$(OS_IMAGE) \
	        -m 256M \
	        -s -S \
	        -monitor stdio

# Aufräumen
clean:
	rm -rf $(BUILD_DIR)

# Disassembly anzeigen
disasm-stage1: $(STAGE1_BIN)
	ndisasm -b 16 $<

disasm-stage2: $(STAGE2_BIN)
	ndisasm -b 16 $< | head -100

disasm-kernel: $(KERNEL_ELF)
	objdump -d $< | head -200

# Hexdump
hexdump: $(OS_IMAGE)
	hexdump -C $< | head -100
