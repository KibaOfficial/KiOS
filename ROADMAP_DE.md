# KiOS Entwicklungs-Roadmap

Dieses Dokument beschreibt die geplanten Features und Entwicklungs-Meilensteine für KiOS.

## Version 0.1.0 (✅ Abgeschlossen - 2026-01-12)

**Status:** Basis-Kernel mit Shell-Funktionalität

### Abgeschlossene Features
- ✅ 64-bit Long Mode Unterstützung
- ✅ Zweistufiger Bootloader
- ✅ VGA Text Mode Treiber (80x25)
- ✅ PS/2 Tastatur-Eingabe (Polling-basiert)
- ✅ Interaktive Shell mit 9 Befehlen
- ✅ Bildschirm-Scrolling Unterstützung
- ✅ Modulare Command-Struktur
- ✅ Build-System mit QEMU-Integration

---

## Version 0.2.0 - Interrupt-Behandlung (✅ Abgeschlossen - 2026-01-12)

**Ziel:** Implementierung von ordentlicher Interrupt-Behandlung und Exception-Management

### Abgeschlossene Features
- ✅ **IDT (Interrupt Descriptor Table)** Setup
  - ✅ IDT mit 256 Einträgen erstellt
  - ✅ ISR Stubs in Assembly definiert
  - ✅ Gemeinsamen Interrupt-Handler implementiert

- ✅ **Exception-Handler**
  - ✅ Alle 32 CPU-Exceptions
  - ✅ Detaillierte Exception-Ausgabe mit Register-Dump
  - ✅ Double Fault mit separatem IST-Stack

- ✅ **PIC (Programmable Interrupt Controller)**
  - ✅ Ordentliche PIC-Initialisierung
  - ✅ IRQ-Remapping (32-47)
  - ✅ IRQ Masking/Unmasking Funktionen

- ✅ **Hardware-Interrupts**
  - ✅ Tastatur-Interrupt (IRQ1) - Polling ersetzt!
  - ✅ IRQ-Handler-Registrierungssystem

- ✅ **GDT & TSS**
  - ✅ Korrektes GDT-Setup mit Code/Data/TSS-Segmenten
  - ✅ TSS für Interrupt-Stack-Switching

- ✅ **Debug-Befehle**
  - ✅ `fault` - CPU-Exceptions zum Testen auslösen

**Tatsächliche Komplexität:** Mittel
**Tatsächliche Zeit:** Selber Tag wie v0.1.0 (2026-01-12)

---

## Version 0.3.0 - Speicherverwaltung (✅ Abgeschlossen - 2026-01-13 bis 2026-01-16)

**Ziel:** Implementierung ordentlicher Speicherverwaltung für Kernel und zukünftige User-Programme

### Abgeschlossene Features
- ✅ **Physical Memory Manager (PMM)**
  - ✅ E820 Memory Map Erkennung
  - ✅ Bitmap-basierter Physical Page Allocator (4KB Pages)
  - ✅ `pmm_alloc_page()` / `pmm_free_page()`
  - ✅ 32.768 Pages verwaltet (128MB RAM)

- ✅ **Virtual Memory Manager (VMM)**
  - ✅ 4-Level Page Table Manipulation (PML4 → PDPT → PD → PT)
  - ✅ `vmm_map_page()` / `vmm_unmap_page()`
  - ✅ `vmm_virt_to_phys()` Adressübersetzung
  - ✅ TLB-Invalidierung und Memory Barriers

- ✅ **Heap-Allocator**
  - ✅ Bump Allocator bei `0xFFFF800000000000`
  - ✅ `kmalloc()` / `kfree()` (kfree ist No-Op)
  - ✅ 16 MB initiale Heap-Größe

- ✅ **Speicher-Befehle**
  - ✅ `vmtest`, `mmap`, `memtest`, `meminfo`

- ✅ **Build-System-Verbesserungen**
  - ✅ Dynamische Kernel-Sektor-Berechnung

**Tatsächliche Komplexität:** Hoch
**Tatsächliche Zeit:** 4 Tage (2026-01-13 bis 2026-01-16)

---

## Version 0.4.0 - Timer & Scheduling (✅ Abgeschlossen - 2026-01-17)

**Ziel:** Implementierung zeitbasierter Operationen und grundlegendes Task-Scheduling

### Abgeschlossene Features
- ✅ **PIT (Programmable Interval Timer)** - 100Hz
- ✅ **Round-Robin Scheduler**
- ✅ **Task Control Blocks (TCB)**
- ✅ **Context Switching** via Stack-Pointer Umbiegen
- ✅ **Kernel Idle Task** (PID 0)
- ✅ **Preemptive Multitasking** (alle 100ms)
- ✅ `uptime`, `tasks` Befehle

**Tatsächliche Komplexität:** Sehr Hoch
**Tatsächliche Zeit:** 1 Tag (2026-01-17)

---

## Version 0.4.1 - ISO Image Support (🔄 Verschoben - Multiboot Ansatz)

**Status:** Verschoben auf nach v0.6.0

---

## Version 0.5.0 - User Mode & System Calls (✅ Abgeschlossen - 2026-01-19)

**Ziel:** Implementierung von User-Space-Trennung und System-Call-Interface

### Abgeschlossene Features
- ✅ **Ring 3 User Mode** mit GDT User Segmenten (DPL 3)
- ✅ **syscall/sysret** Interface via MSRs (EFER, STAR, LSTAR, SFMASK)
- ✅ **swapgs Mechanismus** für Per-CPU Daten
- ✅ **sys_write(), sys_exit()** Syscalls
- ✅ **PAGE_USER Propagierung** durch Page Table Hierarchie
- ✅ **TSS RSP0** für Kernel-Stack bei Privilege-Wechsel
- ✅ `usertest` Command - "Hello Ring 3!" im User Mode

**Tatsächliche Komplexität:** Extrem
**Tatsächliche Zeit:** 2 Tage (2026-01-18 bis 2026-01-19)

---

## Version 0.6.0 - Prozessverwaltung & Dateisystem (🔄 In Arbeit - 2026-04-21)

**Ziel:** Sauberes Prozess-Lifecycle-Management und grundlegende Dateisystem-Unterstützung

### Abgeschlossen
- ✅ **sys_exit Process Switch** - Shell kehrt nach Programmende korrekt zurück
  - ✅ `task_exit_current()` noreturn mit sofortigem iretq Context Switch
  - ✅ `task_restore()` ASM Helper für direkten Task-Switch ohne Timer
  - ✅ `task_set_shell()` zum Registrieren der Shell als Rückkehrziel
  - ✅ `initial_regs` im TCB für sauberen Shell-Neustart
  - ✅ `usertest` → "Hello Ring 3!" → `kiba@KiOS>` ✅
  - ✅ Shell vollständig funktional nach Rückkehr aus Ring 3
  - ✅ **Datum:** 2026-09-01 (nach 4-monatiger Pause fortgesetzt)

- ✅ **VFS Skeleton (In-Memory)** - Unix-artiger virtueller Dateisystem-Layer
  - ✅ Generische `vnode` Struktur mit `vfs_ops` vtable
  - ✅ `open`, `read`, `write`, `close` Interface
  - ✅ `/dev/stdout` → VGA-Treiber
  - ✅ `/dev/stdin` → Tastatur-Treiber
  - ✅ `sys_write` geht jetzt durch VFS statt VGA direkt aufzurufen
  - ✅ **Datum:** 2026-09-01

### Geplant
- [ ] **Quellcode-Restrukturierung**
  - [ ] `arch/`, `drivers/`, `fs/`, `proc/`, `lib/`
  - [ ] Makefile anpassen

- [ ] **Ramdisk**
  - [ ] Einfaches In-Memory-Dateisystem
  - [ ] Dateioperationen: create, read, write, delete

- [ ] **FAT12-Treiber (optional)**
  - [ ] FAT12-Dateisystem von Disk lesen (zunächst read-only)
  - [ ] Zunächst nur Root Directory

- [ ] **Datei-Befehle**
  - [ ] `ls` - Dateien auflisten
  - [ ] `cat <file>` - Dateiinhalt anzeigen

**Tatsächliche Komplexität:** Sehr hoch
**Geschätzte Zeit:** 3-4 Wochen

---


## Version 0.7.0 - Networking (Zukunft)

**Ziel:** Grundlegender Netzwerk-Stack und Gerätetreiber

### Geplante Features
- [ ] PCI-Enumeration
- [ ] RTL8139 / E1000 / virtio-net Treiber
- [ ] Ethernet, ARP, IPv4, ICMP, UDP, TCP
- [ ] `netconf`, `ping`, `netstat` Befehle

**Geschätzte Komplexität:** Extrem
**Geschätzte Zeit:** 4-6 Wochen

---

## Version 1.0.0 - Stable Release (Langfristiges Ziel)

**Ziel:** Feature-Complete, stabiles Bildungs-OS

---

## Zukünftige Ideen (Nach 1.0)

- [ ] SMP (Multi-Prozessor) Unterstützung
- [ ] USB-Unterstützung
- [ ] Grafik-Modus (VESA/GOP)
- [ ] Sound-Unterstützung
- [ ] Mehr Dateisystem-Typen (ext2, KiFS)
- [ ] Shell-Scripting
- [ ] Paket-Manager

---

## Entwicklungs-Prinzipien

1. **Einfachheit Zuerst**
2. **Bildungsfokus**
3. **Inkrementeller Fortschritt**
4. **Stabilität**
5. **Dokumentation**

---

**Zuletzt aktualisiert:** 2026-09-01
**Aktueller Fokus:** v0.6.0 🔄 IN PROGRESS - Process Management & Dateisystem

**v0.6.0 Errungenschaften bisher:**
- ✅ sys_exit kehrt korrekt zur Shell zurück via iretq Context Switch
- ✅ task_restore() ASM Helper für sofortigen Task-Switch
- ✅ task_set_shell() registriert Shell als Rückkehrziel
- ✅ initial_regs im TCB für sauberen Shell-Neustart
- ✅ Shell vollständig funktional nach Ring 3 Programm-Ende
- ✅ VFS Skeleton mit /dev/stdout und /dev/stdin
- ✅ sys_write geht durch VFS Layer (Ring3 → syscall → VFS → devfs → VGA)

**Nächste Schritte:**
- Quellcode-Restrukturierung (arch/, drivers/, fs/, proc/, lib/)
- Ramdisk
- FAT12 Treiber (optional)