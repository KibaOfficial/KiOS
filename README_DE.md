# KiOS - Ein einfaches 64-Bit-Betriebssystem

![Version](https://img.shields.io/badge/version-0.2.0-blue.svg)
![License](https://img.shields.io/badge/license-MIT-green.svg)
![Architecture](https://img.shields.io/badge/arch-x86__64-orange.svg)

KiOS ist ein minimalistisches 64-Bit-Betriebssystem, geschrieben in C und Assembly, entwickelt für Bildungszwecke und zum Lernen der OS-Entwicklung.

## Features

✅ **64-bit Long Mode** - Vollständige x86_64-Unterstützung
✅ **Eigener Bootloader** - Zweistufiger Bootloader (Stage1 + Stage2)
✅ **VGA Text Mode** - 80x25 Farbtext-Ausgabe
✅ **Interaktive Shell** - Kommandozeilen-Interface mit 12 eingebauten Befehlen
✅ **Interrupt-Behandlung** - IDT mit vollständiger Exception- und IRQ-Unterstützung
✅ **Exception-Handler** - Detaillierte Fehlerausgabe für CPU-Exceptions
✅ **PIC-Konfiguration** - IRQ-Remapping zur Konfliktvermeidung
✅ **GDT & TSS** - Korrekte Segment-Descriptor-Tabellen
✅ **Tastatur-Interrupts** - IRQ-basierte Tastatureingabe (kein Polling!)
✅ **Scrolling-Unterstützung** - Automatisches Bildschirm-Scrolling
✅ **Modulares Design** - Saubere Trennung der Komponenten

## Systemanforderungen

**Build-Anforderungen:**
- NASM (Netwide Assembler)
- GCC (GNU Compiler Collection) mit x86_64-Unterstützung
- GNU Make
- GNU ld (Linker)
- objcopy

**Laufzeit-Anforderungen:**
- QEMU (für Emulation) oder echte x86_64-Hardware
- Mindestens 128MB RAM

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
| `mmap`      | Physische Memory Map anzeigen                 |
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
│       └── commands/           # Einzelne Command-Module
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
├── build/                      # Build-Ausgabe-Verzeichnis
├── makefile                    # Haupt-Build-System
└── README.md                   # Diese Datei
```

## Technische Details

### Boot-Sequenz

1. **BIOS** lädt Stage1-Bootloader bei `0x7C00` (Sektor 0)
2. **Stage1** lädt Stage2-Bootloader bei `0x7E00` (Sektor 1-33)
3. **Stage2** führt aus:
   - A20-Gate-Aktivierung
   - GDT-Setup
   - Page-Table-Konfiguration (Identity Mapping mit 2MB-Pages)
   - Übergang zu Long Mode (64-bit)
   - Kernel von Sektor 34 nach `0x100000` (1MB) laden
4. **Kernel** initialisiert VGA, startet Shell

### Speicher-Layout

```
0x00000000 - 0x000003FF    IVT (Real Mode)
0x00000400 - 0x000004FF    BIOS Data Area
0x00007C00 - 0x00007DFF    Bootloader Stage 1
0x00007E00 - 0x0000BDFF    Bootloader Stage 2
0x00010000 - 0x00017FFF    Kernel Load Buffer
0x000A0000 - 0x000BFFFF    VGA-Speicher
0x000B8000 - 0x000B8F9F    VGA Text Buffer (80x25)
0x00100000 - ...           Kernel (1MB+)
0x00200000                 Stack Top
```

### Compiler-Flags

KiOS verwendet spezielle Compiler-Flags für Kernel-Kompatibilität:

- `-ffreestanding` - Keine Standard-Bibliothek
- `-fno-pie` - Kein Position-Independent Code
- `-mno-red-zone` - Erforderlich für Interrupt-Handler
- `-mgeneral-regs-only` - **Kritisch:** Verhindert SSE/AVX/FPU-Instruktionen
- `-nostdlib` - Keine Standard-C-Bibliothek

Das `-mgeneral-regs-only` Flag ist essentiell, um CPU-Exceptions durch SSE-Instruktionen im Kernel-Mode zu verhindern.

## Bekannte Einschränkungen

- Keine Timer-Interrupts (IRQ0 noch nicht genutzt)
- Kein Memory-Management (kein Heap-Allocator)
- Kein Multitasking/Prozess-Management
- Keine Dateisystem-Unterstützung
- Kein Netzwerk-Stack

## Mitwirken

Beiträge sind willkommen! Bitte zögere nicht, Issues oder Pull Requests einzureichen.

## Lizenz

Dieses Projekt ist unter der MIT-Lizenz lizenziert - siehe LICENSE-Datei für Details.

## Danksagungen

- OSDev Wiki - Unschätzbare Ressource für OS-Entwicklung
- QEMU - Essentiell für Tests und Debugging
- Die OS-Entwickler-Community

## Autoren

- **KibaOfficial** - Initiale Arbeit und Entwicklung

---

**Hinweis:** Dies ist ein Bildungsprojekt. Es ist nicht für den produktiven Einsatz gedacht.
