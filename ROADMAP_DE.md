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
  - ✅ 48 ISR (Interrupt Service Routine) Stubs in Assembly definiert
  - ✅ Gemeinsamen Interrupt-Handler implementiert

- ✅ **Exception-Handler**
  - ✅ Alle 32 CPU-Exceptions (Division durch Null, Page Fault, GPF, etc.)
  - ✅ Detaillierte Exception-Ausgabe mit Register-Dump
  - ✅ Double Fault mit separatem IST-Stack für Sicherheit

- ✅ **PIC (Programmable Interrupt Controller)**
  - ✅ Ordentliche PIC-Initialisierung
  - ✅ IRQ-Remapping (32-47 um Konflikte mit Exceptions zu vermeiden)
  - ✅ IRQ Masking/Unmasking Funktionen

- ✅ **Hardware-Interrupts**
  - ✅ Tastatur-Interrupt (IRQ1) - Polling ersetzt!
  - ✅ IRQ-Handler-Registrierungssystem

- ✅ **GDT & TSS**
  - ✅ Korrektes GDT-Setup mit Code/Data/TSS-Segmenten
  - ✅ TSS für Interrupt-Stack-Switching
  - ✅ Segment-Register-Reload nach GDT-Wechsel

- ✅ **Debug-Befehle**
  - ✅ `fault` - Page Fault zum Testen auslösen

**Tatsächliche Komplexität:** Mittel
**Tatsächliche Zeit:** Selber Tag wie v0.1.0 (2026-01-12)

---

## Version 0.3.0 - Speicherverwaltung (✅ Abgeschlossen - 2026-01-13 bis 2026-01-16)

**Ziel:** Implementierung ordentlicher Speicherverwaltung für Kernel und zukünftige User-Programme

### Abgeschlossene Features
- ✅ **Physical Memory Manager (PMM)**
  - ✅ E820 Memory Map Erkennung (mit Fallback auf hardcodierte 128MB Map)
  - ✅ Bitmap-basierter Physical Page Allocator (4KB Pages)
  - ✅ `pmm_alloc_page()` / `pmm_free_page()` Funktionen
  - ✅ Erste 1MB für BIOS/Bootloader reserviert
  - ✅ Kernel und PMM Bitmap Speicher reserviert
  - ✅ 32.768 Pages verwaltet (128MB RAM)
  - ✅ Page 0 Allokations-Bug behoben

- ✅ **Virtual Memory Manager (VMM)**
  - ✅ 4-Level Page Table Manipulation (PML4 → PDPT → PD → PT)
  - ✅ `vmm_map_page()` / `vmm_unmap_page()` Funktionen
  - ✅ `vmm_virt_to_phys()` Adressübersetzung
  - ✅ Automatische Page Table Allokation via PMM
  - ✅ Memory Barriers (`mfence`) für Synchronisation
  - ✅ TLB-Invalidierung nach Modifikationen

- ✅ **Heap-Allocator**
  - ✅ Einfacher Bump Allocator beginnend bei `0xFFFF800000000000`
  - ✅ `kmalloc()` / `kfree()` Implementierung (kfree ist No-Op)
  - ✅ 16 MB initiale Heap-Größe
  - ✅ On-Demand Page Mapping via VMM
  - ✅ 16-Byte Alignment

- ✅ **Speicher-Befehle**
  - ✅ `vmtest` - VMM-Funktionalität testen (Mapping, Unmapping, Read/Write)
  - ✅ `mmap` - E820 Memory Map anzeigen
  - ✅ `memtest` - Umfassender Stress-Test (6 Test-Suites)
  - ✅ `meminfo` - Detaillierte Speicher-Statistiken (PMM/VMM/Heap)

- ✅ **Build-System-Verbesserungen**
  - ✅ Dynamische Kernel-Sektor-Berechnung im Bootloader
  - ✅ Automatische Sektor-Anzahl via Makefile und NASM -D Flag
  - ✅ Keine manuellen Sektor-Updates mehr nötig

**Tatsächliche Komplexität:** Hoch
**Tatsächliche Zeit:** 4 Tage (2026-01-13 bis 2026-01-16)

### Wichtige Errungenschaften
- Kritisches Boot-Problem behoben als Kernel Bootloader-Kapazität überschritt
- Dynamische Sektor-Berechnung implementiert um zukünftige Boot-Fehler zu verhindern
- Umfassende Memory Test-Suite mit 6 verschiedenen Tests erstellt
- Alle Memory-Subsysteme (PMM, VMM, Heap) getestet und verifiziert
- System hat jetzt 16 Shell-Befehle (vorher 14)

---

## Version 0.4.0 - Timer & Scheduling (✅ Abgeschlossen - 2026-01-17)

**Ziel:** Implementierung zeitbasierter Operationen und grundlegendes Task-Scheduling

### Abgeschlossene Features
- ✅ **PIT (Programmable Interval Timer)**
  - ✅ PIT für regelmäßige Interrupts konfigurieren (100Hz)
  - ✅ System-Tick-Counter
  - ✅ Genaues Uptime-Tracking
  - ✅ `task_sleep()` Funktion für Tasks

- ✅ **Basis-Scheduler**
  - ✅ Round-Robin Task Scheduler
  - ✅ Task-Struktur (TCB - Task Control Block)
  - ✅ Context-Switching via Stack-Pointer Umbiegen
  - ✅ Kernel-Threads (Tasks laufen in Ring 0)
  - ✅ Kernel Idle Task (PID 0)
  - ✅ Preemptive Multitasking (alle 100ms)

- ✅ **Timer-Befehle**
  - ✅ `uptime` - System-Laufzeit anzeigen (h/m/s Format)
  - ✅ `tasks` - Alle laufenden Tasks auflisten (PID, State, Name)

**Tatsächliche Komplexität:** Sehr Hoch
**Tatsächliche Zeit:** 1 Tag (2026-01-17)

### Wichtige Errungenschaften
- Erfolgreich Preemptive Multitasking implementiert
- Stack-Pointer Umbiegen via Assembly (`irq_common_stub`)
- General Protection Fault beim ersten Task-Switch gelöst via Kernel Idle Task
- 16-Byte Stack Alignment für x86_64 Kompatibilität
- IRQ Handler gibt nun Stack-Pointer zurück für Context Switching
- Tasks können mit `task_sleep()` für X Ticks schlafen
- Shell läuft jetzt als Task (PID 1) statt direkt im Kernel-Kontext

---

## Version 0.4.1 - ISO Image Support (🔄 Verschoben - Multiboot Ansatz)

**Ziel:** Build-System um bootfähige ISO-Generierung erweitern

### Ursprünglicher Status
Direkte ISO-Generierung mit unserem eigenen Bootloader war **nicht machbar** weil:
- El Torito ISO Boot benötigt entweder GRUB/Multiboot oder einen eigenen ISO Boot-Sektor
- Unser eigener Bootloader nutzt BIOS int 13h, welches direkten Disk-Zugriff erwartet
- xorriso/genisoimage erstellen ISO9660 Dateisystem, das unser Bootloader nicht lesen kann

### Alternativer Plan: Multiboot2 + GRUB
Anstatt unseren Bootloader neu zu schreiben, fügen wir Multiboot2-Support hinzu:
- [ ] **Multiboot2 Header** - Multiboot2 Header zum Kernel-Entry hinzufügen
- [ ] **Dual Boot Support** - Kernel unterstützt sowohl unseren Bootloader als auch GRUB
- [ ] **GRUB Konfiguration** - grub.cfg für ISO-Boot erstellen
- [ ] **grub-mkrescue** - GRUB-Tools für ISO-Generierung nutzen
- [ ] **`make iso`** - Makefile-Target für ISO-Erstellung

**Vorteile:**
- Unser eigener Bootloader bleibt für Raw Disk Images (`kios.bin`)
- GRUB übernimmt ISO-Booting (gut getestet, zuverlässig)
- Gleiche Kernel-Binary funktioniert mit beiden Boot-Methoden

**Geschätzte Komplexität:** Mittel
**Status:** Verschoben auf nach v0.6.0

---

## Version 0.5.0 - User Mode & System Calls (✅ Abgeschlossen - 2026-01-19)

**Ziel:** Implementierung von User-Space-Trennung und System-Call-Interface

### Abgeschlossene Features
- ✅ **User Mode Setup**
  - ✅ GDT-Einträge für User Mode (Ring 3) - User Data (0x18), User Code (0x20)
  - ✅ TSS RSP0 für Kernel-Stack bei Privilege-Wechsel
  - ✅ User-Stack Setup mit PAGE_USER gemappten Pages

- ✅ **System Calls**
  - ✅ syscall/sysret Interface (modern, schnell)
  - ✅ MSR-Konfiguration (EFER, STAR, LSTAR, SFMASK)
  - ✅ swapgs Mechanismus für Per-CPU Daten (GS_BASE/KERNEL_GS_BASE)
  - ✅ Basis-Syscalls: `sys_write()`, `sys_exit()`, `sys_read()` (Platzhalter), `sys_yield()` (Platzhalter)
  - ✅ Syscall-Dispatcher in C

- ✅ **User Mode Ausführung**
  - ✅ Position-unabhängiger Bytecode-Ausführung
  - ✅ `usertest` Command zum Testen des Ring 3 Übergangs
  - ✅ PAGE_USER Propagierung durch alle Page Table Levels (PML4, PDPT, PD, PT)
  - ✅ TLB Flush nach User Page Mapping

**Tatsächliche Komplexität:** Extrem
**Tatsächliche Zeit:** 2 Tage (2026-01-18 bis 2026-01-19)

### Wichtige Errungenschaften
- swapgs Mechanismus gelöst: GS_BASE=0 (User), KERNEL_GS_BASE=&cpu_data (Kernel)
- IRQ-Handler gefixt - dürfen GS-Register NICHT anfassen (bricht swapgs)
- cpu_data Struktur für Per-CPU Kernel/User Stack-Pointer implementiert
- GDT-Reihenfolge wichtig für SYSRET: User Data vor User Code
- IRETQ für Ring 0 → Ring 3 Übergang, SYSRET für Syscall-Return

---

## Version 0.6.0 - Dateisystem-Unterstützung

**Ziel:** Implementierung eines grundlegenden Dateisystems für persistente Speicherung

### Geplante Features
- [ ] **Ramdisk**
  - [ ] Einfaches In-Memory-Dateisystem
  - [ ] Datei-Operationen: create, read, write, delete

- [ ] **VFS (Virtual File System)**
  - [ ] Abstraktes Dateisystem-Interface
  - [ ] Mount-Points

- [ ] **FAT12/16 Treiber (optional)**
  - [ ] FAT-Dateisystem von Disk lesen
  - [ ] Schreib-Unterstützung

- [ ] **Datei-Befehle**
  - [ ] `ls` - Dateien auflisten
  - [ ] `cat <file>` - Datei-Inhalt anzeigen
  - [ ] `mkdir`, `rm`, `touch`

**Geschätzte Komplexität:** Sehr Hoch
**Geschätzte Zeit:** 3-4 Wochen

---

## Version 0.7.0 - Networking (Zukunft)

**Ziel:** Grundlegender Netzwerk-Stack und Gerätetreiber

### Geplante Features
- [ ] **PCI-Enumeration**
  - [ ] PCI-Geräte erkennen
  - [ ] Netzwerkkarten finden

- [ ] **Netzwerkkarten-Treiber**
  - [ ] RTL8139 Treiber (einfach, gut dokumentiert)
  - [ ] Oder E1000 Treiber (Intel)
  - [ ] Oder virtio-net (QEMU-optimiert)

- [ ] **Netzwerk-Stack**
  - [ ] Ethernet-Frames
  - [ ] ARP-Protokoll
  - [ ] IP-Protokoll (IPv4)
  - [ ] ICMP (ping)
  - [ ] UDP (optional)
  - [ ] TCP (optional, komplex)

- [ ] **Netzwerk-Befehle**
  - [ ] `netconf` - Netzwerk-Interface konfigurieren
  - [ ] `slap <ip>` - Remote-Host anpingen
  - [ ] `netstat` - Netzwerk-Statistiken anzeigen

**Geschätzte Komplexität:** Extrem
**Geschätzte Zeit:** 4-6 Wochen

---

## Version 1.0.0 - Stable Release (Langfristiges Ziel)

**Ziel:** Feature-Complete, stabiles Bildungs-OS

### Ziele
- [ ] Alle oben genannten Features implementiert und getestet
- [ ] Umfassende Dokumentation
- [ ] Beispiel-Programme
- [ ] Tutorial/Guide für OS-Entwicklungs-Lernende
- [ ] Umfangreiche Test-Suite
- [ ] Performance-Optimierungen

---

## Zukünftige Ideen (Nach 1.0)

**Optional Advanced Features:**
- [ ] SMP (Multi-Prozessor) Unterstützung
- [ ] USB-Unterstützung
- [ ] Grafik-Modus (VESA/GOP)
- [ ] Sound-Unterstützung (AC97 oder Sound Blaster)
- [ ] Mehr Dateisystem-Typen (ext2, eigenes FS)
- [ ] Shell-Scripting
- [ ] Paket-Manager
- [ ] Bestehende Software portieren (lua, Spiele, etc.)

---

## Entwicklungs-Prinzipien

1. **Einfachheit Zuerst** - Implementierungen einfach und verständlich halten
2. **Bildungsfokus** - Code sollte lesbar und gut kommentiert sein
3. **Inkrementeller Fortschritt** - Kleine, testbare Schritte
4. **Stabilität** - Bestehende Features nicht kaputt machen
5. **Dokumentation** - Jedes Feature sollte dokumentiert sein

---

## Wie man beitragen kann

Willst du helfen, ein Feature aus dieser Roadmap zu implementieren? So geht's:

1. Wähle ein Feature aus dem aktuellen Versions-Meilenstein
2. Erstelle ein Issue, das deinen Implementierungs-Ansatz diskutiert
3. Forke das Repository und erstelle einen Feature-Branch
4. Implementiere das Feature mit Tests
5. Reiche einen Pull Request mit Dokumentation ein

---

**Zuletzt aktualisiert:** 2026-01-19
**Aktueller Fokus:** v0.5.0 ✅ ABGESCHLOSSEN - User Mode & System Calls funktionieren!

**v0.5.0 Errungenschaften:**
- ✅ GDT mit User Segmenten (Ring 3, DPL 3)
- ✅ syscall/sysret Interface via MSRs (EFER, STAR, LSTAR, SFMASK)
- ✅ swapgs Mechanismus für Per-CPU Datenzugriff
- ✅ sys_write() und sys_exit() Syscalls funktionieren
- ✅ PAGE_USER Propagierung durch Page Table Hierarchie
- ✅ `usertest` Command führt "Hello Ring 3!" im User Mode aus
- ✅ IRQ-Handler gefixt - korumpieren GS-Register nicht mehr
- ✅ IRETQ für Ring 0 → Ring 3 Übergang

**Nächste Schritte:**
- Planung von v0.6.0 - Dateisystem-Unterstützung
- Überlegen: Ramdisk, VFS, FAT12/16 Treiber
- Optional: ELF-Loader für richtige User-Programme
