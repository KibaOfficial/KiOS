BUILD_DIR ?= build/
ASM ?= nasm
CC ?= gcc
LD ?= ld

# Compiler Flags für Freestanding 64-Bit Environment
# WICHTIG: -mgeneral-regs-only verhindert SSE/AVX-Instruktionen komplett!
CFLAGS = -ffreestanding -fno-pie -m64 -mno-red-zone -mgeneral-regs-only \
         -Wall -Wextra -O2 -nostdlib -nostdinc

# Linker Flags
LDFLAGS = -T linker.ld -nostdlib -z max-page-size=0x1000


# Alle Command-Module automatisch finden
COMMANDS_SRCS := $(wildcard commands/*.c)
COMMANDS_OBJS := $(patsubst %.c,$(BUILD_DIR)/%.o,$(COMMANDS_SRCS))

# Object Files
OBJS = $(BUILD_DIR)/entry.o $(BUILD_DIR)/main.o $(BUILD_DIR)/shell.o $(BUILD_DIR)/commands.o $(BUILD_DIR)/tss.o $(BUILD_DIR)/gdt.o $(COMMANDS_OBJS)
# Compile tss.c
$(BUILD_DIR)/tss.o: src/kernel/tss.c src/kernel/tss.h
	$(CC) $(CFLAGS) -c src/kernel/tss.c -o $(BUILD_DIR)/tss.o

# Compile gdt.c
$(BUILD_DIR)/gdt.o: src/kernel/gdt.c src/kernel/gdt.h
	$(CC) $(CFLAGS) -c src/kernel/gdt.c -o $(BUILD_DIR)/gdt.o

.PHONY: all kernel clean

all: kernel

kernel: $(BUILD_DIR)/kernel.bin

# Link kernel
$(BUILD_DIR)/kernel.bin: $(OBJS) linker.ld
	$(LD) $(LDFLAGS) -o $@ $(OBJS)

# Compile entry.asm (64-bit)
$(BUILD_DIR)/entry.o: entry.asm
	$(ASM) -f elf64 entry.asm -o $(BUILD_DIR)/entry.o



# Compile main.c
$(BUILD_DIR)/main.o: main.c types.h vga.h keyboard.h shell.h string.h io.h
	$(CC) $(CFLAGS) -c main.c -o $(BUILD_DIR)/main.o

# Compile shell.c
$(BUILD_DIR)/shell.o: shell.c shell.h vga.h keyboard.h string.h io.h
	$(CC) $(CFLAGS) -c shell.c -o $(BUILD_DIR)/shell.o

# Compile commands.c (defines shell_commands array)
$(BUILD_DIR)/commands.o: commands.c commands.h shell.h
	$(CC) $(CFLAGS) -c commands.c -o $(BUILD_DIR)/commands.o

# Compile all command modules
$(BUILD_DIR)/commands/%.o: commands/%.c commands.h vga.h
	mkdir -p $(BUILD_DIR)/commands
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(BUILD_DIR)/entry.o $(BUILD_DIR)/main.o $(BUILD_DIR)/kernel.bin