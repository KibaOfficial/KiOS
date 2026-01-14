# KiOS - A Simple 64-bit Operating System

![Version](https://img.shields.io/badge/version-0.2.0-blue.svg)
![License](https://img.shields.io/badge/license-MIT-green.svg)
![Architecture](https://img.shields.io/badge/arch-x86__64-orange.svg)

KiOS is a minimalist 64-bit operating system written in C and Assembly, designed for educational purposes and OS development learning.

## Features

✅ **64-bit Long Mode** - Full x86_64 support
✅ **Custom Bootloader** - Two-stage bootloader (Stage1 + Stage2)
✅ **VGA Text Mode** - 80x25 color text output
✅ **Interactive Shell** - Command-line interface with 12 built-in commands
✅ **Interrupt Handling** - IDT with full exception and IRQ support
✅ **Exception Handlers** - Detailed error reporting for CPU exceptions
✅ **PIC Configuration** - IRQ remapping to avoid conflicts
✅ **GDT & TSS** - Proper segment descriptor tables
✅ **Keyboard Interrupts** - IRQ-based keyboard input (no polling!)
✅ **Scrolling Support** - Automatic screen scrolling
✅ **Modular Design** - Clean separation of components

## System Requirements

**Build Requirements:**
- NASM (Netwide Assembler)
- GCC (GNU Compiler Collection) with x86_64 support
- GNU Make
- GNU ld (Linker)
- objcopy

**Runtime Requirements:**
- QEMU (for emulation) or real x86_64 hardware
- At least 128MB RAM

## Quick Start

### Building KiOS

```bash
# Clone the repository
git clone <repository-url>
cd KiOS-New

# Build the OS image
make

# Run in QEMU
make run

# Run with debug output
make run-debug
```

### Available Make Targets

- `make` or `make all` - Build the complete OS image
- `make clean` - Remove all build artifacts
- `make run` - Run KiOS in QEMU with monitor
- `make run-debug` - Run with detailed debug logging
- `make run-serial` - Run with serial console output
- `make debug` - Start QEMU with GDB server (port 1234)

## Shell Commands

KiOS includes an interactive shell with the following commands:

| Command    | Description                                 |
|------------|---------------------------------------------|
| `help`     | Display all available commands               |
| `clear`    | Clear the screen                            |
| `info`     | Show system information                     |
| `echo`     | Echo text to the screen                     |
| `color`    | Display VGA color palette                   |
| `mem`      | Show memory layout                          |
| `mmap`     | Show physical memory map                    |
| `time`     | Display current system time (uptime)        |
| `fault`    | Trigger a CPU exception for testing         |
| `netconf`  | Show network configuration (placeholder)    |
| `reboot`   | Reboot the system                           |
| `shutdown` | Shutdown the system                         |
| `halt`     | Halt the system                             |

## Project Structure

```
KiOS-New/
├── src/
│   ├── bootloader/
│   │   └── new/
│   │       ├── stage1.asm      # First-stage bootloader (512 bytes)
│   │       └── stage2.asm      # Second-stage bootloader (loads kernel)
│   └── kernel/
│       ├── entry.asm           # Kernel entry point (64-bit)
│       ├── main.c              # Kernel main function
│       ├── shell.c             # Shell implementation
│       ├── shell.h             # Shell header
│       ├── commands.c          # Command registry
│       ├── commands.h          # Command prototypes
│       ├── vga.c               # VGA driver implementation
│       ├── vga.h               # VGA driver header
│       ├── keyboard.h          # PS/2 keyboard driver
│       ├── keyboard_irq.c      # Keyboard interrupt handler
│       ├── idt.c               # IDT initialization
│       ├── idt.h               # IDT structures
│       ├── idt_asm.asm         # ISR/IRQ stubs (Assembly)
│       ├── isr.c               # Exception and IRQ handlers
│       ├── isr.h               # ISR structures
│       ├── pic.c               # PIC configuration
│       ├── pic.h               # PIC definitions
│       ├── gdt.c               # GDT initialization
│       ├── gdt.h               # GDT structures
│       ├── tss.c               # TSS setup
│       ├── tss.h               # TSS structures
│       ├── string.h            # String utilities
│       ├── io.h                # I/O port operations
│       ├── types.h             # Type definitions
│       ├── linker.ld           # Kernel linker script
│       └── commands/           # Individual command modules
│           ├── help.c
│           ├── clear.c
│           ├── info.c
│           ├── echo.c
│           ├── color.c
│           ├── mem.c
│           ├── mmap.c
│           ├── time.c
│           ├── reboot.c
│           ├── shutdown.c
│           ├── netconf.c
│           ├── halt.c
│           ├── fault.c
├── build/                      # Build output directory
├── makefile                    # Main build system
└── README.md                   # This file
```

## Technical Details

### Boot Sequence

1. **BIOS** loads Stage1 bootloader at `0x7C00` (sector 0)
2. **Stage1** loads Stage2 bootloader at `0x7E00` (sector 1-33)
3. **Stage2** performs:
   - A20 gate activation
   - Temporary GDT setup
   - Page table configuration (identity mapping with 2MB pages)
   - Transition to Long Mode (64-bit)
   - Load kernel from sector 34 to `0x100000` (1MB)
4. **Kernel** performs:
   - VGA initialization
   - GDT and TSS setup
   - PIC remapping (IRQs 0-15 → Interrupts 32-47)
   - IDT initialization with 256 entries
   - Interrupt activation
   - Shell startup

### Memory Layout

```
0x00000000 - 0x000003FF    IVT (Real Mode)
0x00000400 - 0x000004FF    BIOS Data Area
0x00007C00 - 0x00007DFF    Bootloader Stage 1
0x00007E00 - 0x0000BDFF    Bootloader Stage 2
0x00010000 - 0x00017FFF    Kernel Load Buffer
0x000A0000 - 0x000BFFFF    VGA Memory
0x000B8000 - 0x000B8F9F    VGA Text Buffer (80x25)
0x00100000 - ...           Kernel (1MB+)
0x00200000                 Stack Top
```

### Compiler Flags

KiOS uses special compiler flags to ensure kernel compatibility:

- `-ffreestanding` - No standard library
- `-fno-pie` - No position-independent code
- `-mno-red-zone` - **Critical:** Required for interrupt handlers
- `-mgeneral-regs-only` - **Critical:** Prevents SSE/AVX/FPU instructions
- `-nostdlib` - No standard C library

The `-mgeneral-regs-only` flag is essential to prevent CPU exceptions from SSE instructions in kernel mode.

### Interrupt Architecture

KiOS implements a full interrupt handling system:

**IDT (Interrupt Descriptor Table)**
- 256 entries for exceptions and interrupts
- Exceptions 0-31: CPU exceptions (Division by Zero, Page Fault, etc.)
- Interrupts 32-47: Hardware IRQs (Timer, Keyboard, etc.)
- Interrupts 48-255: Available for software interrupts

**Exception Handling**
When a CPU exception occurs, KiOS displays:
- Exception name and number
- Error code
- Register dump (RIP, RSP, RAX, RBX, RCX, RDX, etc.)
- RFLAGS, CS, and SS

**IRQ Handling**
- PIC is remapped to avoid conflicts with CPU exceptions
- IRQ handlers can be registered dynamically
- Automatic EOI (End of Interrupt) to PIC
- Keyboard uses IRQ1 for interrupt-driven input

## Known Limitations

- No timer interrupts (IRQ0 not used yet)
- No memory management (no heap allocator)
- No multitasking/process management
- No filesystem support
- No network stack

## Contributing

Contributions are welcome! Please feel free to submit issues or pull requests.

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Acknowledgments

- OSDev Wiki - Invaluable resource for OS development
- QEMU - Essential for testing and debugging
- The OS development community

## Authors

- **KibaOfficial** - Initial work and development

---

**Note:** This is an educational project. It is not intended for production use.
