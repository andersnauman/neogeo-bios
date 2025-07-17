#include "bios.h"

#include "utils.h"

// Convert an 8-bit binary value 0–99 to packed BCD (0x00–0x99).
uint8_t to_bcd8(uint8_t v) {
    uint8_t tens = v / 10;
    uint8_t ones = v % 10;
    return (uint8_t)((tens << 4) | ones);
}

// Convert a packed-BCD byte 0x00–0x99 back to binary 0–99.
uint8_t from_bcd8(uint8_t b) {
    uint8_t tens = (b >> 4) & 0x0F;
    uint8_t ones = b & 0x0F;
    return tens * 10 + ones;
}

// Weird NOP-loop in both unibios and SNK bios
void nop_loop(uint8_t count) {
    for (int i = 0; i < count; i++) {
        __asm__ volatile ("nop");
    }
}

void write_rtc_command(uint8_t command) {
    for (uint8_t i = 0; i < 4; i++) {        
        *WATCHDOG = 0;
        uint8_t write = command;
        write &= 0x1;
        *REG_RTCCTRL = write;
        write |= 0x2;
        *REG_RTCCTRL = write;
        write ^= 0x2;
        *REG_RTCCTRL = write;
        command >>= 1;
    }
    *REG_RTCCTRL = 0x4;
    *REG_RTCCTRL = 0x0;
}

void wait_for_rtc_pulse_edge(uint8_t *prev) {
    // Original code waited for infinity. Limit it to have an honest change to see the error.
    for (uint32_t i = 0; i < 0x20000; i++) {
        *WATCHDOG = 0;
        uint8_t curr = *REG_STATUS_A;
        // Detect rising edge on bit 6 (RTC pulse)
        if (((*prev ^ curr) & curr) & 0x40) {
            *prev = curr;
            return;
        }
        *prev = curr;
    }
}

void wait_for_vblank() {
    *BIOS_VBLANK_CLEAR = 1;
    // TODO: This infinite loop looks bad? (original code is infinite)
    // limit to 50000 breaks things... 
    while(1 == *BIOS_VBLANK_CLEAR) {
        __asm__ volatile ("nop");
    };
}

void wait_for_z80() {
    if (1 == *REG_SOUND ) {
        return;
    }

    uint16_t sr;
    __asm__ volatile ("move.w %%sr,%0" : "=d"(sr));    
    __asm__ volatile ("move #0x2000, %sr");
    *BIOS_Z80_BUSY = 1;
    for (uint8_t attempt = 0; attempt < 8; attempt++) {
        uint16_t deadline = (uint16_t)(*BIOS_FRAME_COUNTER) + 4;
        *REG_SOUND = 1;
        while ((uint16_t)*BIOS_FRAME_COUNTER != deadline) {
            *WATCHDOG = 0;
            if (*REG_SOUND == 1) {
                goto _wait_z80_end;
            }
        }
    }

_wait_z80_end:
    nop_loop(54);
    *BIOS_Z80_BUSY = 0;
    __asm__ volatile ("move.w %0,%%sr" :: "d"(sr));
}

// Zero out all palettes in both bank 0 och bank 1
void reset_palettes() {
    for (int8_t bank = 1; bank >= 0; bank--) {
        if (bank == 0) {
            *REG_PALBANK0 = 0;
        } else if (bank == 1) {
            *REG_PALBANK1 = 0;
        }
        volatile uint32_t *address = (volatile uint32_t *) PALETTE_RAM;
        while (address < (volatile uint32_t *) PALETTE_RAM + 0x2000) {
            *WATCHDOG = 0;
            *address = 0;
            address++;
        }
    }
}

void reset_fix_layer() {
    *REG_VRAMMOD = 1;
    *REG_VRAMADDR = 0x7000;

    // First row
    for (uint16_t i = 0; i < 0x20; i++) {
        *WATCHDOG = 0;
        *REG_VRAMRW = (uint16_t)0x0020;
    }
    // Middle
    for (uint16_t i = 0; i < 0x4C0; i++) {
        *WATCHDOG = 0;
        *REG_VRAMRW = (uint16_t)0x00FF;
    }
    // Last row
    for (uint16_t i = 0; i < 0x20; i++) {
        *WATCHDOG = 0;
        *REG_VRAMRW = (uint16_t)0x0020;
    }
}

void reset_sprites() {
    *REG_VRAMMOD = 1;
    *REG_VRAMADDR = 0;          // $0000-$001F  32      Lower   SCB1
    // TODO: Investigate if only 32 tiles are ever used(?)
    for (uint16_t i = 0; i < 0x20; i++) {
        *WATCHDOG = 0;
        *REG_VRAMRW = (uint16_t)0x00FF;
    }
    *REG_VRAMADDR = 0x8000;     // $8000-$81FF  512     Upper	SCB2
    for (uint16_t i = 0; i < 0x200; i++) {
        *WATCHDOG = 0;
        *REG_VRAMRW = (uint16_t)0x0FFF;
    }
    *REG_VRAMADDR = 0x8200;     // $8200-$83FF  512     Upper   SCB3
    for (uint16_t i = 0; i < 0x200; i++) {
        *WATCHDOG = 0;
        *REG_VRAMRW = (uint16_t)0x0000;
    }

    *REG_VRAMADDR = 0x8400;     // $8400-$85FF  512     Upper   SCB4
    for (uint16_t i = 0; i < 0x200; i++) {
        *WATCHDOG = 0;
        *REG_VRAMRW = (uint16_t)0xBE00;
    }    
}