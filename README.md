# KiOS - A Simple 64-bit Operating System

![Version](https://img.shields.io/badge/version-0.5.0-blue.svg)
![License](https://img.shields.io/badge/license-MIT-green.svg)
![Architecture](https://img.shields.io/badge/arch-x86__64-orange.svg)

KiOS is a minimalist 64-bit operating system written in C and Assembly.

## Features

### Core System
- ✅ **64-bit Long Mode** - Full x86_64 support
- ✅ **Custom Bootloader** - Two-stage bootloader with chunkwise kernel loading
- ✅ **VGA Text Mode** - 80x25 color text output
- ✅ **Interactive Shell** - Command-line interface with 16 built-in commands
- ✅ **Interrupt Handling** - IDT with full exception and IRQ support
- ✅ **Exception Handlers** - Detailed error reporting for CPU exceptions
- ✅ **PIC Configuration** - IRQ remapping to avoid conflicts
- ✅ **GDT & TSS** - Proper segment descriptor tables with Double Fault IST
- ✅ **Keyboard Interrupts** - IRQ-based keyboard input (no polling!)
- ✅ **Scrolling Support** - Automatic screen scrolling
- ✅ **Modular Design** - Clean separation of components

### Memory Management (v0.3.0) ✅
- ✅ **Physical Memory Manager (PMM)** - Bitmap-based allocator managing 128MB RAM
- ✅ **Virtual Memory Manager (VMM)** - 4-level page table manipulation with memory barriers
- ✅ **Page Allocation** - pmm_alloc_page() and pmm_free_page()
- ✅ **Virtual Mapping** - vmm_map_page() and vmm_unmap_page()
- ✅ **Address Translation** - vmm_virt_to_phys()
- ✅ **Heap Allocator** - kmalloc/kfree with bump allocator and on-demand page mapping
- ✅ **Dynamic Bootloader** - Automatic kernel sector calculation

### Multitasking & Scheduling (v0.4.0) ✅
- ✅ **PIT Timer** - Programmable Interval Timer running at 100Hz
- ✅ **Preemptive Multitasking** - Task switching every 100ms
- ✅ **Round-Robin Scheduler** - Fair CPU time distribution
- ✅ **Task Control Blocks (TCB)** - Full task state management
- ✅ **Context Switching** - Stack-pointer based task switching
- ✅ **Kernel Threads** - Tasks running in Ring 0
- ✅ **System Uptime** - Precise time tracking since boot

### User Mode & System Calls (v0.5.0) ✅
- ✅ **Ring 3 User Mode** - Protected user space execution
- ✅ **GDT User Segments** - User Code/Data segments (DPL 3)
- ✅ **syscall/sysret** - Modern fast system call interface
- ✅ **swapgs Mechanism** - Per-CPU data via GS segment register
- ✅ **System Calls** - sys_write, sys_exit, sys_read (placeholder), sys_yield (placeholder)
- ✅ **User Page Mapping** - PAGE_USER propagation through page table hierarchy
- ✅ **TSS RSP0** - Kernel stack for privilege level switches

## System Requirements

**Build Requirements:**
- NASM (Netwide Assembler)
- GCC (GNU Compiler Collection) with x86_64 support
- GNU Make
- GNU ld (Linker)
- objcopy

**Runtime Requirements:**
- QEMU (for emulation) or real x86_64 hardware
- At least 256MB RAM (recommended for v0.3.0 memory management features)

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
| `mmap`     | Show physical memory map (E820)             |
| `meminfo`  | Show detailed memory statistics             |
| `memtest`  | Run comprehensive memory stress tests       |
| `vmtest`   | Test Virtual Memory Manager (VMM)           |
| `usertest` | Test Ring 3 User Mode with syscalls         |
| `time`     | Display current system time                 |
| `uptime`   | Show system uptime (h/m/s)                  |
| `tasks`    | List all running tasks (PID/State/Name)     |
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
│       ├── mm/                 # Memory Management
│       │   ├── pmm.c           # Physical Memory Manager
│       │   ├── pmm.h           # PMM Header
│       │   ├── vmm.c           # Virtual Memory Manager
│       │   ├── vmm.h           # VMM Header
│       │   ├── heap.c          # Kernel Heap Allocator
│       │   ├── heap.h          # Heap Header
│       │   └── memory_map.h    # Memory Map utilities
│       └── commands/           # Individual command modules
│           ├── help.c
│           ├── clear.c
│           ├── info.c
│           ├── echo.c
│           ├── color.c
│           ├── mem.c
│           ├── mmap.c
│           ├── meminfo.c       # Memory statistics command
│           ├── memtest.c       # Memory stress test command
│           ├── vmtest.c        # VMM Test command
│           ├── time.c
│           ├── reboot.c
│           ├── shutdown.c
│           ├── netconf.c
│           ├── halt.c
│           └── fault.c
├── build/                      # Build output directory
├── makefile                    # Main build system
└── README.md                   # This file
```

## Technical Details

### Boot Sequence

1. **BIOS** loads Stage1 bootloader at `0x7C00` (sector 0)
2. **Stage1** loads Stage2 bootloader at `0x7E00` (sector 1-33)
3. **Stage2** performs:
   - E820 memory map detection (with fallback to hardcoded map)
   - A20 gate activation
   - Temporary GDT setup
   - Page table configuration (1GB identity mapping with 2MB pages)
   - Transition to Long Mode (64-bit)
   - Dynamic kernel sector calculation (automatically loads correct kernel size)
   - Load kernel from sector 34 to `0x100000` (1MB)
4. **Kernel** performs:
   - VGA initialization
   - PIC configuration (mask all IRQs initially)
   - TSS initialization (Double Fault IST stack)
   - GDT setup with TSS segment
   - IDT initialization with 256 entries
   - PMM initialization (Physical Memory Manager)
   - VMM initialization (Virtual Memory Manager)
   - Heap initialization (Kernel Heap Allocator)
   - Keyboard interrupt handler activation (IRQ1)
   - Interrupt activation (STI)
   - Shell startup

### Memory Layout

```
0x00000000 - 0x000003FF    IVT (Real Mode)
0x00000400 - 0x000004FF    BIOS Data Area
0x00007C00 - 0x00007DFF    Bootloader Stage 1
0x00007E00 - 0x0000BDFF    Bootloader Stage 2
0x00009000                 PML4 (Page Map Level 4)
0x0000A000                 PDPT (Page Directory Pointer Table)
0x0000B000                 PD (Page Directory)
0x0000C000                 PT (Page Table)
0x00010000                 PMM Bitmap (16 KB for 128 MB RAM)
0x000A0000 - 0x000BFFFF    VGA Memory
0x000B8000 - 0x000B8F9F    VGA Text Buffer (80x25)
0x00100000 - ...           Kernel (1MB+, ~97 sectors = 49KB)
0x00200000                 Stack Top
0xFFFF800000000000+        Kernel Heap (Virtual, 16MB initial size)
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

### Memory Management (v0.3.0)

**Physical Memory Manager (PMM)**
- Bitmap-based page allocator
- 4KB page granularity
- Manages up to 128MB RAM (32,768 pages)
- Reserves first 1MB for BIOS/bootloader
- API: `pmm_alloc_page()`, `pmm_free_page()`, `pmm_total_pages()`, `pmm_used_pages()`

**Virtual Memory Manager (VMM)**
- 4-level page table manipulation (PML4 → PDPT → PD → PT)
- Automatic page table allocation via PMM
- Memory barriers (`mfence`) for synchronization
- TLB invalidation after page table modifications
- API: `vmm_map_page()`, `vmm_unmap_page()`, `vmm_virt_to_phys()`

**Heap Allocator**
- Bump allocator starting at `0xFFFF800000000000`
- 16 MB initial heap size
- On-demand page mapping via VMM
- `kmalloc(size)` with 16-byte alignment
- `kfree(ptr)` as no-op (sufficient for v0.3.0)
- API: `kmalloc()`, `kfree()`, `heap_total_allocated()`, `heap_current_size()`

**memtest Command**
Comprehensive stress testing with 6 test suites:
1. PMM page allocation (50 pages)
2. VMM page mapping with verification
3. Memory read/write with unique test patterns
4. VMM page unmapping with verification
5. PMM page freeing
6. Heap allocations (100 blocks × 256 bytes) with data integrity verification

**meminfo Command**
Displays detailed statistics for:
- PMM: Total/Used/Free pages, usage percentage
- VMM: PML4 address, page size, paging levels
- Heap: Base address, allocated bytes, current size, pages mapped

## Known Limitations

- Heap allocator is simple bump allocator (no free-list, kfree is no-op)
- No filesystem support
- No network stack
- VGA Text Mode limited to 80x25 resolution
- User mode programs are bytecode only (no ELF loader yet)
- `usertest` command halts the system after sys_exit (no process management yet)

## Contributing

Contributions are welcome! Please feel free to submit issues or pull requests.

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## AI Disclaimer

Parts of this project were created with the assistance of generative AI models. While I review much of the code and reference documentation such as the OSDev Wiki, some sections may contain inaccuracies or errors. I'm only human and can't know everything, but I do my best! :)

## Acknowledgments

- OSDev Wiki - Invaluable resource for OS development
- QEMU - Essential for testing and debugging
- The OS development community

## Authors

- **KibaOfficial** - Initial work and development

---

**Note:** KiOS is a hobby operating system project. While functional, it is not intended for production use.
