#include "backup_ram.h"
#include "bios.h"
#include "bios_calls.h"
#include "bios_test.h"
#include "graphic.h"

void draw_fix_tile(uint8_t x, uint8_t y, uint16_t tile_number, uint8_t palette) {
    uint16_t vram_offset = 0x7000 + (x * 32) + y;
    //uint16_t vram_offset = 0x7022;

    *REG_VRAMADDR = vram_offset; // Set write address
    *REG_VRAMRW = (palette << 12) | (tile_number & 0x0FFF); // Write tile
}

void set_palette(uint16_t palette, uint16_t color1) {
    /*
    F	E	D	C	B	A	9	8	7	6	5	4	3	2	1	0
    D	R0	G0	B0	R4	R3	R2	R1	G4	G3	G2	G1	B4	B3	B2	B1
    16 colors / palette
    256 palettes
    */
    PALETTE_RAM[0] = 0x8000;           // Reference color
    uint16_t base = palette * 16;      // Palette RAM is already uint16_t so multiple with 16 is enough

    PALETTE_RAM[base + 0] = 0x0000;    // Color 0x00 (Background/Transparant color)
    PALETTE_RAM[base + 1] = color1;    // Color 0x01 (Foreground color 1)

    *PALETTE_BACKGROUND = 0x0000;
    *PALETTE_OFFSCREEN_BORDER = 0x5555;
}

void print_string(uint8_t start_x, uint8_t start_y, const char *str) {
    for (uint8_t i = 0; str[i] != '\0'; ++i) {
        start_x++;
        uint16_t c = str[i];
        draw_fix_tile(start_x, start_y, c, 1);
    }
}