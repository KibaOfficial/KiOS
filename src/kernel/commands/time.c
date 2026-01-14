#include "../commands.h"
#include "../vga.h"
#include "../io.h"
#include "../types.h"

#define CMOS_ADDR_PORT  0x70
#define CMOS_DATA_PORT  0x71

static uint8_t cmos_read(uint8_t reg) {
    outb(CMOS_ADDR_PORT, reg);
    return inb(CMOS_DATA_PORT);
}

static uint8_t bcd_to_bin(uint8_t bcd) {
    return ((bcd >> 4) * 10) + (bcd & 0x0F);
}

void cmd_time(const char* args) {
    (void)args;
    while (cmos_read(0x0A) & 0x80);
    uint8_t second = cmos_read(0x00);
    uint8_t minute = cmos_read(0x02);
    uint8_t hour = cmos_read(0x04);
    uint8_t day = cmos_read(0x07);
    uint8_t month = cmos_read(0x08);
    uint8_t year = cmos_read(0x09);
    uint8_t regB = cmos_read(0x0B);
    if (!(regB & 0x04)) {
        second = bcd_to_bin(second);
        minute = bcd_to_bin(minute);
        hour = bcd_to_bin(hour & 0x7F) | (hour & 0x80);
        day = bcd_to_bin(day);
        month = bcd_to_bin(month);
        year = bcd_to_bin(year);
    }
    if (!(regB & 0x02) && (hour & 0x80)) {
        hour = ((hour & 0x7F) + 12) % 24;
    }
    vga_println("");
    vga_print("  Current time (UTC): ");
    vga_print("20");
    if (year < 10) vga_putchar('0');
    vga_print_dec(year);
    vga_putchar('-');
    if (month < 10) vga_putchar('0');
    vga_print_dec(month);
    vga_putchar('-');
    if (day < 10) vga_putchar('0');
    vga_print_dec(day);
    vga_putchar(' ');
    if (hour < 10) vga_putchar('0');
    vga_print_dec(hour);
    vga_putchar(':');
    if (minute < 10) vga_putchar('0');
    vga_print_dec(minute);
    vga_putchar(':');
    if (second < 10) vga_putchar('0');
    vga_print_dec(second);
    vga_println("");
    vga_println("");
}
