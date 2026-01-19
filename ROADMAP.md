# KiOS Development Roadmap

This document outlines the planned features and development milestones for KiOS.

## Version 0.1.0 (✅ Completed - 2026-01-12)

**Status:** Basic kernel with shell functionality

### Completed Features
- ✅ 64-bit Long Mode support
- ✅ Two-stage bootloader
- ✅ VGA text mode driver (80x25)
- ✅ PS/2 keyboard input (polling-based)
- ✅ Interactive shell with 9 commands
- ✅ Screen scrolling support
- ✅ Modular command structure
- ✅ Build system with QEMU integration

---

## Version 0.2.0 - Interrupt Handling (✅ Completed - 2026-01-12)

**Goal:** Implement proper interrupt handling and exception management

### Completed Features
- ✅ **IDT (Interrupt Descriptor Table)** setup
  - ✅ Create IDT with 256 entries
  - ✅ Define ISR (Interrupt Service Routine) stubs
  - ✅ Implement common interrupt handler

- ✅ **Exception Handlers**
  - ✅ Division by Zero (#DE)
  - ✅ Invalid Opcode (#UD)
  - ✅ General Protection Fault (#GP)
  - ✅ Page Fault (#PF)
  - ✅ Double Fault (#DF) with IST stack
  - ✅ Stack Fault (#SS)
  - ✅ All 32 CPU exceptions implemented

- ✅ **PIC (Programmable Interrupt Controller)**
  - ✅ Proper PIC initialization and remapping
  - ✅ IRQ remapping (IRQs 0-15 → Interrupts 32-47)
  - ✅ IRQ masking/unmasking functions

- ✅ **Hardware Interrupts**
  - ✅ Keyboard interrupt (IRQ1) - replaced polling
  - ⏸️ Timer interrupt (IRQ0) - deferred to v0.4.0

- ✅ **Debug Commands**
  - ✅ `fault` - Trigger CPU exceptions for testing (div0, ud, pf)

**Complexity:** Medium
**Time Taken:** Same day as v0.1.0 (2026-01-12)

---

## Version 0.3.0 - Memory Management (✅ Completed - 2026-01-13 to 2026-01-16)

**Goal:** Implement proper memory management for kernel and future user programs

### Completed Features
- ✅ **Physical Memory Manager (PMM)**
  - ✅ E820 memory map detection (with fallback to hardcoded 128MB map)
  - ✅ Bitmap-based physical page allocator (4KB pages)
  - ✅ `pmm_alloc_page()` / `pmm_free_page()` functions
  - ✅ Reserve first 1MB for BIOS/bootloader
  - ✅ Reserve kernel and PMM bitmap memory
  - ✅ 32,768 pages managed (128MB RAM)
  - ✅ Fixed Page 0 allocation bug

- ✅ **Virtual Memory Manager (VMM)**
  - ✅ 4-level page table manipulation (PML4 → PDPT → PD → PT)
  - ✅ `vmm_map_page()` / `vmm_unmap_page()` functions
  - ✅ `vmm_virt_to_phys()` address translation
  - ✅ Automatic page table allocation via PMM
  - ✅ Memory barriers (`mfence`) for synchronization
  - ✅ TLB invalidation after modifications

- ✅ **Heap Allocator**
  - ✅ Simple bump allocator starting at `0xFFFF800000000000`
  - ✅ `kmalloc()` / `kfree()` implementation (kfree is no-op)
  - ✅ 16 MB initial heap size
  - ✅ On-demand page mapping via VMM
  - ✅ 16-byte alignment

- ✅ **Memory Commands**
  - ✅ `vmtest` - Test VMM functionality (mapping, unmapping, read/write)
  - ✅ `mmap` - Display E820 memory map
  - ✅ `memtest` - Comprehensive stress test (6 test suites)
  - ✅ `meminfo` - Detailed memory statistics (PMM/VMM/Heap)

- ✅ **Build System Improvements**
  - ✅ Dynamic kernel sector calculation in bootloader
  - ✅ Automatic sector count via makefile and NASM -D flag
  - ✅ No more manual sector count updates needed

**Actual Complexity:** High
**Actual Time:** 4 days (2026-01-13 to 2026-01-16)

### Key Accomplishments
- Fixed critical boot issue when kernel exceeded bootloader capacity
- Implemented dynamic sector calculation to prevent future boot failures
- Created comprehensive memory test suite with 6 different tests
- All memory subsystems (PMM, VMM, Heap) tested and verified working
- System now has 16 shell commands (up from 14)

---

## Version 0.4.0 - Timer & Scheduling (✅ Completed - 2026-01-17)

**Goal:** Implement time-based operations and basic task scheduling

### Completed Features
- ✅ **PIT (Programmable Interval Timer)**
  - ✅ Configure PIT for regular interrupts (100Hz)
  - ✅ System tick counter
  - ✅ Accurate uptime tracking
  - ✅ `task_sleep()` function for tasks

- ✅ **Basic Scheduler**
  - ✅ Round-robin task scheduler
  - ✅ Task structure (TCB - Task Control Block)
  - ✅ Context switching via stack pointer switching
  - ✅ Kernel threads (tasks run in Ring 0)
  - ✅ Kernel Idle Task (PID 0)
  - ✅ Preemptive multitasking (every 100ms)

- ✅ **Timer Commands**
  - ✅ `uptime` - Show system uptime (h/m/s format)
  - ✅ `tasks` - List all running tasks (PID, State, Name)

**Actual Complexity:** Very High
**Actual Time:** 1 day (2026-01-17)

### Key Accomplishments
- Successfully implemented preemptive multitasking
- Stack pointer switching via Assembly (`irq_common_stub`)
- Solved General Protection Fault on first task switch via Kernel Idle Task
- 16-byte stack alignment for x86_64 compatibility
- IRQ handler now returns stack pointer for context switching
- Tasks can sleep with `task_sleep()` for X ticks
- Shell now runs as Task (PID 1) instead of directly in kernel context

---

## Version 0.4.1 - ISO Image Support (🔄 Postponed - Multiboot Approach)

**Goal:** Extend build system with bootable ISO generation

### Original Status
Direct ISO generation with our custom bootloader was **not feasible** because:
- El Torito ISO boot requires either GRUB/Multiboot or a custom ISO boot sector
- Our custom bootloader uses BIOS int 13h which expects raw disk access
- xorriso/genisoimage create ISO9660 filesystem which our bootloader can't read

### Alternative Plan: Multiboot2 + GRUB
Instead of rewriting our bootloader, we will add Multiboot2 support:
- [ ] **Multiboot2 Header** - Add Multiboot2 header to kernel entry
- [ ] **Dual Boot Support** - Kernel handles both our bootloader and GRUB
- [ ] **GRUB Configuration** - Create grub.cfg for ISO boot
- [ ] **grub-mkrescue** - Use GRUB tools for ISO generation
- [ ] **`make iso`** - Makefile target for ISO creation

**Benefits:**
- Our custom bootloader remains for raw disk images (`kios.bin`)
- GRUB handles ISO booting (well-tested, reliable)
- Same kernel binary works with both boot methods

**Estimated Complexity:** Medium
**Status:** Postponed to after v0.6.0

---

## Version 0.5.0 - User Mode & System Calls (✅ Completed - 2026-01-19)

**Goal:** Implement user space separation and system call interface

### Completed Features
- ✅ **User Mode Setup**
  - ✅ GDT entries for user mode (Ring 3) - User Data (0x18), User Code (0x20)
  - ✅ TSS RSP0 for kernel stack on privilege switches
  - ✅ User stack setup with PAGE_USER mapped pages

- ✅ **System Calls**
  - ✅ syscall/sysret interface (modern, fast)
  - ✅ MSR configuration (EFER, STAR, LSTAR, SFMASK)
  - ✅ swapgs mechanism for per-CPU data (GS_BASE/KERNEL_GS_BASE)
  - ✅ Basic syscalls: `sys_write()`, `sys_exit()`, `sys_read()` (placeholder), `sys_yield()` (placeholder)
  - ✅ Syscall dispatcher in C

- ✅ **User Mode Execution**
  - ✅ Position-independent bytecode execution
  - ✅ `usertest` command to test Ring 3 transition
  - ✅ PAGE_USER propagation through all page table levels (PML4, PDPT, PD, PT)
  - ✅ TLB flush after user page mapping

**Actual Complexity:** Extreme
**Actual Time:** 2 days (2026-01-18 to 2026-01-19)

### Key Accomplishments
- Solved swapgs mechanism: GS_BASE=0 (user), KERNEL_GS_BASE=&cpu_data (kernel)
- Fixed IRQ handlers to NOT touch GS register (breaks swapgs)
- Implemented cpu_data structure for per-CPU kernel/user stack pointers
- GDT order matters for SYSRET: User Data before User Code
- IRETQ for Ring 0 → Ring 3 transition, SYSRET for syscall return

---

## Version 0.6.0 - Filesystem Support

**Goal:** Implement basic filesystem for persistent storage

### Planned Features
- [ ] **Ramdisk**
  - [ ] Simple in-memory filesystem
  - [ ] File operations: create, read, write, delete

- [ ] **VFS (Virtual File System)**
  - [ ] Abstract filesystem interface
  - [ ] Mount points

- [ ] **FAT12/16 Driver (optional)**
  - [ ] Read FAT filesystem from disk
  - [ ] Write support

- [ ] **File Commands**
  - [ ] `ls` - List files
  - [ ] `cat <file>` - Display file contents
  - [ ] `mkdir`, `rm`, `touch`

**Estimated Complexity:** Very High
**Estimated Time:** 3-4 weeks

---

## Version 0.7.0 - Networking (Future)

**Goal:** Basic network stack and device drivers

### Planned Features
- [ ] **PCI Enumeration**
  - [ ] Detect PCI devices
  - [ ] Find network cards

- [ ] **Network Card Driver**
  - [ ] RTL8139 driver (simple, well-documented)
  - [ ] Or E1000 driver (Intel)
  - [ ] Or virtio-net (QEMU-optimized)

- [ ] **Network Stack**
  - [ ] Ethernet frames
  - [ ] ARP protocol
  - [ ] IP protocol (IPv4)
  - [ ] ICMP (ping)
  - [ ] UDP (optional)
  - [ ] TCP (optional, complex)

- [ ] **Network Commands**
  - [ ] `ifconfig` - Configure network interface
  - [ ] `ping <ip>` - Ping remote host
  - [ ] `netstat` - Show network statistics

**Estimated Complexity:** Extreme
**Estimated Time:** 4-6 weeks

---

## Version 1.0.0 - Stable Release (Long-term Goal)

**Goal:** Feature-complete, stable educational OS

### Goals
- [ ] All above features implemented and tested
- [ ] Comprehensive documentation
- [ ] Example programs
- [ ] Tutorial/guide for OS development learners
- [ ] Extensive test suite
- [ ] Performance optimizations

---

## Future Ideas (Beyond 1.0)

**Optional Advanced Features:**
- [ ] SMP (Multi-processor) support
- [ ] USB support
- [ ] Graphics mode (VESA/GOP)
- [ ] Sound support (AC97 or Sound Blaster)
- [ ] More filesystem types (ext2, custom FS)
- [ ] Shell scripting
- [ ] Package manager
- [ ] Port existing software (lua, games, etc.)

---

## Development Principles

1. **Simplicity First** - Keep implementations simple and understandable
2. **Educational Focus** - Code should be readable and well-commented
3. **Incremental Progress** - Small, testable steps
4. **Stability** - Don't break existing features
5. **Documentation** - Every feature should be documented

---

## How to Contribute

Want to help implement a feature from this roadmap? Here's how:

1. Pick a feature from the current version milestone
2. Create an issue discussing your implementation approach
3. Fork the repository and create a feature branch
4. Implement the feature with tests
5. Submit a pull request with documentation

---

**Last Updated:** 2026-01-19
**Current Focus:** v0.5.0 ✅ COMPLETED - User Mode & System Calls work!

**v0.5.0 Accomplishments:**
- ✅ GDT with User segments (Ring 3, DPL 3)
- ✅ syscall/sysret interface via MSRs (EFER, STAR, LSTAR, SFMASK)
- ✅ swapgs mechanism for per-CPU data access
- ✅ sys_write() and sys_exit() syscalls working
- ✅ PAGE_USER propagation through page table hierarchy
- ✅ `usertest` command executes "Hello Ring 3!" in user mode
- ✅ IRQ handlers fixed to not corrupt GS register
- ✅ IRETQ for Ring 0 → Ring 3 transition

**Next Steps:**
- Planning v0.6.0 - Filesystem Support
- Consider: Ramdisk, VFS, FAT12/16 driver
- Optional: ELF loader for proper user programs
