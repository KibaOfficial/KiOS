#include "vga.h"
#include "types.h"

#define MEMORY_MAP_BASE 0x8000
#define MEMORY_MAP_ENTRY_SIZE 24

typedef struct {
    uint64_t base;
    uint64_t length;
    uint32_t type;
    uint32_t reserved;
} __attribute__((packed)) memory_map_entry_t;

void cmd_mmap(const char* args) {
    (void)args;
    #define MEMORY_MAP_BASE 0x8000
    typedef struct {
        uint64_t base;
        uint64_t length;
        uint32_t type;
        uint32_t reserved;
    } __attribute__((packed)) memory_map_entry_t;
    uint16_t entry_count = *(uint16_t*)MEMORY_MAP_BASE;
    memory_map_entry_t* entries = (memory_map_entry_t*)(MEMORY_MAP_BASE + 2);
    vga_println("Detected Memory Map:");
    for (uint16_t i = 0; i < entry_count; i++) {
        memory_map_entry_t* entry = &entries[i];
        vga_print("  Entry ");
        vga_print_dec(i);
        vga_print(": base=");
        vga_print_hex(entry->base);
        vga_print(", length=");
        vga_print_hex(entry->length);
        vga_print(", type=");
        vga_print_dec(entry->type);
        vga_println("");
    }
    vga_println("");
}
