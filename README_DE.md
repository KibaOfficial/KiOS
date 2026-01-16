# KiOS - Ein einfaches 64-Bit-Betriebssystem

![Version](https://img.shields.io/badge/version-0.3.0--dev-blue.svg)
![License](https://img.shields.io/badge/license-MIT-green.svg)
![Architecture](https://img.shields.io/badge/arch-x86__64-orange.svg)

KiOS ist ein minimalistisches 64-Bit-Betriebssystem, geschrieben in C und Assembly.

## Features

### Kernsystem
✅ **64-bit Long Mode** - Vollständige x86_64-Unterstützung
✅ **Eigener Bootloader** - Zweistufiger Bootloader mit chunked Kernel-Loading
✅ **VGA Text Mode** - 80x25 Farbtext-Ausgabe
✅ **Interaktive Shell** - Kommandozeilen-Interface mit 16 eingebauten Befehlen
✅ **Interrupt-Behandlung** - IDT mit vollständiger Exception- und IRQ-Unterstützung
✅ **Exception-Handler** - Detaillierte Fehlerausgabe für CPU-Exceptions
✅ **PIC-Konfiguration** - IRQ-Remapping zur Konfliktvermeidung
✅ **GDT & TSS** - Korrekte Segment-Descriptor-Tabellen mit Double Fault IST
✅ **Tastatur-Interrupts** - IRQ-basierte Tastatureingabe (kein Polling!)
✅ **Scrolling-Unterstützung** - Automatisches Bildschirm-Scrolling
✅ **Modulares Design** - Saubere Trennung der Komponenten

### Speicherverwaltung (v0.3.0) ✅
✅ **Physical Memory Manager (PMM)** - Bitmap-basierter Allocator für 128MB RAM
✅ **Virtual Memory Manager (VMM)** - 4-Level Page Table Manipulation mit Memory Barriers
✅ **Page-Allokation** - pmm_alloc_page() und pmm_free_page()
✅ **Virtuelles Mapping** - vmm_map_page() und vmm_unmap_page()
✅ **Adressübersetzung** - vmm_virt_to_phys()
✅ **Heap Allocator** - kmalloc/kfree mit Bump Allocator und On-Demand Page Mapping
✅ **Dynamischer Bootloader** - Automatische Kernel-Sektor-Berechnung

## Systemanforderungen

**Build-Anforderungen:**
- NASM (Netwide Assembler)
- GCC (GNU Compiler Collection) mit x86_64-Unterstützung
- GNU Make
- GNU ld (Linker)
- objcopy

**Laufzeit-Anforderungen:**
- QEMU (für Emulation) oder echte x86_64-Hardware
- Mindestens 256MB RAM (empfohlen für v0.3.0 Speicherverwaltungs-Features)

## Schnellstart

### KiOS bauen

```bash
# Repository klonen
git clone <repository-url>
cd KiOS-New

# OS-Image bauen
make

# In QEMU ausführen
make run

# Mit Debug-Ausgabe ausführen
make run-debug
```

### Verfügbare Make-Targets

- `make` oder `make all` - Komplettes OS-Image bauen
- `make clean` - Alle Build-Artefakte entfernen
- `make run` - KiOS in QEMU mit Monitor ausführen
- `make run-debug` - Mit detailliertem Debug-Logging ausführen
- `make run-serial` - Mit serieller Konsolen-Ausgabe ausführen
- `make debug` - QEMU mit GDB-Server starten (Port 1234)

## Shell-Befehle

KiOS enthält eine interaktive Shell mit folgenden Befehlen:

| Befehl      | Beschreibung                                 |
|-------------|----------------------------------------------|
| `help`      | Zeigt alle verfügbaren Befehle an             |
| `clear`     | Bildschirm löschen                            |
| `info`      | System-Informationen anzeigen                 |
| `echo`      | Text auf Bildschirm ausgeben                  |
| `color`     | VGA-Farbpalette anzeigen                      |
| `mem`       | Speicher-Layout anzeigen                      |
| `mmap`      | Physische Memory Map anzeigen (E820)          |
| `meminfo`   | Detaillierte Speicher-Statistiken anzeigen    |
| `memtest`   | Umfassende Speicher-Stress-Tests durchführen  |
| `vmtest`    | Virtual Memory Manager (VMM) testen           |
| `time`      | Systemzeit/Uptime anzeigen                    |
| `fault`     | CPU-Exception auslösen (Test)                 |
| `netconf`   | Netzwerkkonfiguration anzeigen (Platzhalter)  |
| `reboot`    | System neu starten                            |
| `shutdown`  | System herunterfahren                         |
| `halt`      | System anhalten                               |

## Projektstruktur

```
KiOS-New/
├── src/
│   ├── bootloader/
│   │   └── new/
│   │       ├── stage1.asm      # Bootloader Stufe 1 (512 Bytes)
│   │       └── stage2.asm      # Bootloader Stufe 2 (lädt Kernel)
│   └── kernel/
│       ├── entry.asm           # Kernel-Einstiegspunkt (64-bit)
│       ├── main.c              # Kernel-Hauptfunktion
│       ├── shell.c             # Shell-Implementierung
│       ├── shell.h             # Shell-Header
│       ├── commands.c          # Command-Registry
│       ├── commands.h          # Command-Prototypen
│       ├── vga.c               # VGA-Treiber Implementierung
│       ├── vga.h               # VGA-Treiber Header
│       ├── keyboard.h          # PS/2-Tastatur-Treiber
│       ├── string.h            # String-Utilities
│       ├── io.h                # I/O-Port-Operationen
│       ├── types.h             # Typ-Definitionen
│       ├── linker.ld           # Kernel-Linker-Script
│       ├── mm/                 # Speicherverwaltung
│       │   ├── pmm.c           # Physical Memory Manager
│       │   ├── pmm.h           # PMM Header
│       │   ├── vmm.c           # Virtual Memory Manager
│       │   ├── vmm.h           # VMM Header
│       │   ├── heap.c          # Kernel Heap Allocator
│       │   ├── heap.h          # Heap Header
│       │   └── memory_map.h    # Memory Map Utilities
│       └── commands/           # Einzelne Command-Module
│           ├── help.c
│           ├── clear.c
│           ├── info.c
│           ├── echo.c
│           ├── color.c
│           ├── mem.c
│           ├── mmap.c
│           ├── meminfo.c       # Speicher-Statistik-Command
│           ├── memtest.c       # Speicher-Stress-Test-Command
│           ├── vmtest.c        # VMM Test-Command
│           ├── time.c
│           ├── reboot.c
│           ├── shutdown.c
│           ├── netconf.c
│           ├── halt.c
│           └── fault.c
├── build/                      # Build-Ausgabe-Verzeichnis
├── makefile                    # Haupt-Build-System
└── README.md                   # Diese Datei
```

## Technische Details

### Boot-Sequenz

1. **BIOS** lädt Stage1-Bootloader bei `0x7C00` (Sektor 0)
2. **Stage1** lädt Stage2-Bootloader bei `0x7E00` (Sektor 1-33)
3. **Stage2** führt aus:
   - E820 Memory Map Erkennung (mit Fallback auf hardcodierte Map)
   - A20-Gate-Aktivierung
   - Temporäres GDT-Setup
   - Page-Table-Konfiguration (1GB Identity Mapping mit 2MB-Pages)
   - Übergang zu Long Mode (64-bit)
   - Dynamische Kernel-Sektor-Berechnung (lädt automatisch korrekte Kernel-Größe)
   - Kernel von Sektor 34 nach `0x100000` (1MB) laden
4. **Kernel** führt aus:
   - VGA-Initialisierung
   - PIC-Konfiguration (alle IRQs initial maskieren)
   - TSS-Initialisierung (Double Fault IST Stack)
   - GDT-Setup mit TSS-Segment
   - IDT-Initialisierung mit 256 Einträgen
   - PMM-Initialisierung (Physical Memory Manager)
   - VMM-Initialisierung (Virtual Memory Manager)
   - Heap-Initialisierung (Kernel Heap Allocator)
   - Tastatur-Interrupt-Handler aktivieren (IRQ1)
   - Interrupt-Aktivierung (STI)
   - Shell starten

### Speicher-Layout

```
0x00000000 - 0x000003FF    IVT (Real Mode)
0x00000400 - 0x000004FF    BIOS Data Area
0x00007C00 - 0x00007DFF    Bootloader Stage 1
0x00007E00 - 0x0000BDFF    Bootloader Stage 2
0x00009000                 PML4 (Page Map Level 4)
0x0000A000                 PDPT (Page Directory Pointer Table)
0x0000B000                 PD (Page Directory)
0x0000C000                 PT (Page Table)
0x00010000                 PMM Bitmap (16 KB für 128 MB RAM)
0x000A0000 - 0x000BFFFF    VGA-Speicher
0x000B8000 - 0x000B8F9F    VGA Text Buffer (80x25)
0x00100000 - ...           Kernel (1MB+, ~97 Sektoren = 49KB)
0x00200000                 Stack Top
0xFFFF800000000000+        Kernel Heap (Virtuell, 16MB initiale Größe)
```

### Compiler-Flags

KiOS verwendet spezielle Compiler-Flags für Kernel-Kompatibilität:

- `-ffreestanding` - Keine Standard-Bibliothek
- `-fno-pie` - Kein Position-Independent Code
- `-mno-red-zone` - Erforderlich für Interrupt-Handler
- `-mgeneral-regs-only` - **Kritisch:** Verhindert SSE/AVX/FPU-Instruktionen
- `-nostdlib` - Keine Standard-C-Bibliothek

Das `-mgeneral-regs-only` Flag ist essentiell, um CPU-Exceptions durch SSE-Instruktionen im Kernel-Mode zu verhindern.

### Speicherverwaltung (v0.3.0)

**Physical Memory Manager (PMM)**
- Bitmap-basierter Page Allocator
- 4KB Page-Granularität
- Verwaltet bis zu 128MB RAM (32.768 Pages)
- Reserviert erste 1MB für BIOS/Bootloader
- API: `pmm_alloc_page()`, `pmm_free_page()`, `pmm_total_pages()`, `pmm_used_pages()`

**Virtual Memory Manager (VMM)**
- 4-Level Page Table Manipulation (PML4 → PDPT → PD → PT)
- Automatische Page Table Allokation via PMM
- Memory Barriers (`mfence`) für Synchronisation
- TLB-Invalidierung nach Page Table Änderungen
- API: `vmm_map_page()`, `vmm_unmap_page()`, `vmm_virt_to_phys()`

**Heap Allocator**
- Bump Allocator beginnend bei `0xFFFF800000000000`
- 16 MB initiale Heap-Größe
- On-Demand Page Mapping via VMM
- `kmalloc(size)` mit 16-Byte Alignment
- `kfree(ptr)` als No-Op (ausreichend für v0.3.0)
- API: `kmalloc()`, `kfree()`, `heap_total_allocated()`, `heap_current_size()`

**memtest Command**
Umfassende Stress-Tests mit 6 Test-Suites:
1. PMM Page Allocation (50 Pages)
2. VMM Page Mapping mit Verifikation
3. Memory Read/Write mit eindeutigen Test-Patterns
4. VMM Page Unmapping mit Verifikation
5. PMM Page Freeing
6. Heap Allocations (100 Blöcke × 256 Bytes) mit Datenintegritätsprüfung

**meminfo Command**
Zeigt detaillierte Statistiken für:
- PMM: Total/Used/Free Pages, Auslastung in %
- VMM: PML4-Adresse, Page-Größe, Paging-Levels
- Heap: Base-Adresse, allokierte Bytes, aktuelle Größe, gemappte Pages

## Bekannte Einschränkungen

- Keine Timer-Interrupts (IRQ0 noch nicht genutzt)
- Heap Allocator ist einfacher Bump Allocator (keine Free-List, kfree ist No-Op)
- Kein Multitasking/Prozess-Management
- Keine Dateisystem-Unterstützung
- Kein Netzwerk-Stack
- VGA Text Mode auf 80x25 Auflösung limitiert

## Mitwirken

Beiträge sind willkommen! Bitte zögere nicht, Issues oder Pull Requests einzureichen.

## Lizenz

Dieses Projekt ist unter der MIT-Lizenz lizenziert - siehe LICENSE-Datei für Details.

## KI Disclaimer
Teile dieses Projekts wurden mit Hilfe generativer KI-Modelle erstellt. Obwohl ich vieles überprüfe und docs wie OS Dev Wiki als referenz nutze, könnten einige Abschnitte Ungenauigkeiten oder Fehler enthalten. Ich bin auch nur ein Mensch ich kann nicht alles wissen aber mein bestes geben :)

## Danksagungen

- OSDev Wiki - Unschätzbare Ressource für OS-Entwicklung
- QEMU - Essentiell für Tests und Debugging
- Die OS-Entwickler-Community

## Autoren

- **KibaOfficial** - Initiale Arbeit und Entwicklung

---

**Hinweis:** KiOS ist ein Hobby-Betriebssystem-Projekt. Obwohl funktionsfähig, ist es nicht für den produktiven Einsatz gedacht.
