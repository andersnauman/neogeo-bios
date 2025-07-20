#include <stdint.h>

#include "backup_ram.h"
#include "bios.h"
#include "graphic.h"
#include "led.h"
#include "mess_out.h"
#include "utils.h"

#include "service.h"

void show_crosshatch_test() {
    // Palette 0
    *(volatile uint32_t *)0x400002 = 0x0EEE0000;
    // Palette 1
    if (*BIOS_COUNTRY_CODE == BIOS_COUNTRY_JAPAN) {
        *(volatile uint32_t *)0x400022 = 0x0E000000;
    } else if (*BIOS_COUNTRY_CODE == BIOS_COUNTRY_USA) {
        *(volatile uint32_t *)0x400022 = 0x00EE0000;
    } else if (*BIOS_COUNTRY_CODE == BIOS_COUNTRY_EUROPE) {
        *(volatile uint32_t *)0x400022 = 0x000E0000; 
    }

    *BIOS_MESS_BUSY = 1;

    // Crosshatch design
    *REG_VRAMMOD = 1;
    *REG_VRAMADDR = 0x7000;
    for (uint8_t i = 0; i < 20; i++) {
        for (uint8_t x = 0; x < 16; x++) {
            *REG_VRAMRW = 1;
            __asm__ volatile ("nop");
            *REG_VRAMRW = 3;
        }

        for (uint8_t x = 0; x < 16; x++) {
            *REG_VRAMRW = 2;
            __asm__ volatile ("nop");
            *REG_VRAMRW = 4;            
        }
    }

    // Left columns
    *REG_VRAMADDR = 0x7000;
    for (uint8_t i = 0; i < 96; i++) {
        __asm__ volatile ("nop");
        __asm__ volatile ("nop");
        __asm__ volatile ("nop");
        uint16_t value = *REG_VRAMRW;
        *REG_VRAMRW = value | 0x1000;
    }

    // Right columns
    *REG_VRAMADDR = 0x74A0;
    for (uint8_t i = 0; i < 96; i++) {
        __asm__ volatile ("nop");
        __asm__ volatile ("nop");
        __asm__ volatile ("nop");
        uint16_t value = *REG_VRAMRW;
        *REG_VRAMRW = value | 0x1000;
    }

    // Top rows
    *REG_VRAMADDR = 0x7002;
    *REG_VRAMMOD = 0x20;
    for (uint8_t i = 0; i < 40; i++) {
        __asm__ volatile ("nop");
        __asm__ volatile ("nop");
        __asm__ volatile ("nop");
        uint16_t value = *REG_VRAMRW;
        *REG_VRAMRW = value | 0x1000;
    }

    // Bottom rows
    *REG_VRAMADDR = 0x701D;
    for (uint8_t i = 0; i < 40; i++) {
        __asm__ volatile ("nop");
        __asm__ volatile ("nop");
        __asm__ volatile ("nop");
        uint16_t value = *REG_VRAMRW;
        *REG_VRAMRW = value | 0x1000;
    }   

    *BIOS_MESS_BUSY = 0;
}

void show_color_test() {
    *(uint16_t *)0x400024 = 0x0E00;
    *(uint16_t *)0x400044 = 0x00E0;
    *(uint16_t *)0x400064 = 0x000E;
    *(uint16_t *)0x400084 = 0x0EEE;    
    *PALETTE_REFERENCE = 0x8000;
    *PALETTE_BACKGROUND = 0x0000;
    *(uint16_t *)0x400002 = 0x0EEE;

    *BIOS_MESS_BUSY = 1;

    *(volatile uint32_t *) BIOS_MESS_BUFFER = (uint32_t)red_green_blue;
    *BIOS_MESS_POINT = BIOS_MESS_BUFFER_PTR + sizeof(uint32_t);

    *BIOS_MESS_BUSY = 0;
}

void show_io_test() {
    *(uint16_t *)0x400002 = 0x0EEE;

    *BIOS_MESS_BUSY = 1;

    *(volatile uint32_t *) BIOS_MESS_BUFFER = (uint32_t)io;
    *BIOS_MESS_POINT = BIOS_MESS_BUFFER_PTR + sizeof(uint32_t);

    *SERVICE_LED_MARQUEE = 1;
    *SERVICE_LED_COIN = 1;

    *BIOS_MESS_BUSY = 0;    
}

void update_io_test() {
    *BIOS_MESS_BUSY = 1;

    *(volatile uint32_t *) *BIOS_MESS_POINT = (uint32_t)io_values;
    *BIOS_MESS_POINT = *BIOS_MESS_POINT + sizeof(uint32_t);

    volatile uint8_t *address = (volatile uint8_t *)0x10FC00;
    uint8_t frame_count_low = *BIOS_FRAME_COUNTER_LOW;

    if ((frame_count_low & 0x3F) == 0) {
        // EL (Marquee)
        *SERVICE_LED_MARQUEE += 1;
        if (*SERVICE_LED_MARQUEE > *BRAM_SLOT_COUNT) {
            *SERVICE_LED_MARQUEE = 1;
        }
        // LED
        *SERVICE_LED_COIN += 1;
        if (*SERVICE_LED_COIN > 9) {
            *SERVICE_LED_COIN = 1;
        }
    }
    // EL (Marquee) 
    set_led_data(LED_LATCH_MARQUEE, *SERVICE_LED_MARQUEE);
    uint16_t marquee_led_slot = *SERVICE_LED_MARQUEE | 0x30;
    *(volatile uint16_t *)address = marquee_led_slot << 8 | 0x20;
    address += 2;

    // LED 1
    set_led_data(LED_LATCH_LED1, *SERVICE_LED_COIN);
    uint16_t coin_led = *SERVICE_LED_COIN | 0x30;
    *(volatile uint16_t *)address = coin_led << 8 | coin_led;
    address += 2;

    // LED 2
    set_led_data(LED_LATCH_LED2, (10 - *SERVICE_LED_COIN));
    coin_led = (10 - *SERVICE_LED_COIN) | 0x30;
    *(volatile uint16_t *)address = coin_led << 8 | coin_led;
    address += 2;

    // P1
    uint8_t controller = *BIOS_P1CURRENT;
    for (uint8_t i = 0; i < 8; i++) {
        *address = (controller & 1) | 0x30;
        controller >>= 1;
        address++;
    }
    uint8_t start_select = *BIOS_STATCURRENT_RAW;
    for (uint8_t i = 0; i < 2; i++) {
        *address = (start_select & 1) | 0x30;
        start_select >>= 1;
        address++;
    }
    // P2
    controller = *BIOS_P2CURRENT;
    for (uint8_t i = 0; i < 8; i++) {
        *address = (controller & 1) | 0x30;
        controller >>= 1;
        address++;
    }
    for (uint8_t i = 0; i < 2; i++) {
        *address = (start_select & 1) | 0x30;
        start_select >>= 1;
        address++;
    }
    // Hardware dip switches
    uint8_t hw_dipsw = ~(*REG_DIPSW);
    for (uint8_t i = 0; i < 8; i++) {
        *address = (hw_dipsw & 1) | 0x30;
        hw_dipsw >>= 1;
        address++;
    }
    // Test button
    *address = ((~(*REG_SYSTYPE) & 0x80) >> 7) | 0x30;
    address++;

    uint8_t coin_service = *BRAM_COIN_STATUS_CURRENT;
    for (uint8_t i = 0; i < 3; i++) {
        *address = (coin_service & 1) | 0x30;
        coin_service >>= 1;
        address++;
    }

    *BIOS_MESS_BUSY = 0;
}

void show_sound_test() {
    // Prepare the z80
    *REG_SOUND = 0x3;
    for (uint8_t i = 0; i < 5; i++) {
        wait_for_vblank();
    }
    *REG_SOUND = 0x7;

    *(uint32_t *)0x400002 = 0x0EEE0000; // Palette 0
    *(uint32_t *)0x400022 = 0x0E000000; // Palette 1
    *BIOS_MESS_BUSY = 1;

    *(volatile uint32_t *) BIOS_MESS_BUFFER = (uint32_t)sound;
    *BIOS_MESS_POINT = BIOS_MESS_BUFFER_PTR + sizeof(uint32_t);

    *SERVICE_SOUND_CURSOR = 0;
    *SERVICE_SOUND_SELECT = 0;
    *BIOS_MESS_BUSY = 0;
}

void update_sound_test() {   
    *BIOS_MESS_BUSY = 1;

    // If 'A' button is pressed
    if ((*BIOS_P1CHANGE & 0x10) != 0) {
        // No sound
        if (0 == *SERVICE_SOUND_CURSOR) {
            *REG_SOUND = 0x3;
            *SERVICE_SOUND_SELECT = 0;
        // Left
        } else if (1 == *SERVICE_SOUND_CURSOR) {
            *REG_SOUND = 0x5D;
            *SERVICE_SOUND_SELECT = 1;
        // Right
        } else if (2 == *SERVICE_SOUND_CURSOR) {
            *REG_SOUND = 0x5C;
            *SERVICE_SOUND_SELECT = 2;
        // Center
        } else if (3 == *SERVICE_SOUND_CURSOR) {
            *REG_SOUND = 0x5E;
            *SERVICE_SOUND_SELECT = 3;
        }
    }

    // Menu "go-up"
    if ((*BIOS_P1CHANGE & 0x1) != 0) {
        *SERVICE_SOUND_CURSOR -= 1;
        if (*SERVICE_SOUND_CURSOR < 0) {
            *SERVICE_SOUND_CURSOR = 3;
        }
    // Menu "go-down"
    } else if ((*BIOS_P1CHANGE & 0x2) != 0) {
        *SERVICE_SOUND_CURSOR += 1;
        if (*SERVICE_SOUND_CURSOR > 3) {
            *SERVICE_SOUND_CURSOR = 0;
        }        
    }

    *(volatile uint32_t *) *BIOS_MESS_POINT = (uint32_t)sound;
    *BIOS_MESS_POINT = *BIOS_MESS_POINT + sizeof(uint32_t);

    volatile uint16_t *address = (volatile uint16_t *)0x10FC00;
    *address = 0x2002;
    address += 1;    
    *address = 0x0003;
    address += 1;
    *address = 0x718a;
    address += 1;

    // Print the cursor
    for (uint8_t i = 0; i < 4; i++) {
        if (i == *SERVICE_SOUND_CURSOR) {
            *address = ((uint16_t)0x11 << 8) | 0x08;    // Palette 1, additional 0x100 to char
            address += 1;
            *address = 0x11FF;                          // Arrow
            address += 1;
        } else {
            *address = ((uint16_t)0x01 << 8) | 0x08;    // Palette 0, additional 0x100 to char
            address += 1;
            *address = 0x20FF;                          // Space (must be here to clear the arrow)
            address += 1;
        }
        *address = 0x0005;
        address += 1;
        *address = 0x0002;
        address += 1;        
    }

    *address = 0x0003;
    address += 1;
    *address = 0x720a;
    address += 1;

    // Print the text
    for (uint8_t i = 0; i < 4; i++) {
        // Set the color for the selected option
        if (i == *SERVICE_SOUND_SELECT) {
            *address = ((uint16_t)0x11 << 8) | 0x08;    // Palette 1, additional 0x100 to char
            address += 1;
        } else {
            *address = ((uint16_t)0x01 << 8) | 0x08;    // Palette 0, additional 0x100 to char
            address += 1;
        }

        // Add the text from an array based on the loop iteration
        for (uint8_t c = 0; sound_menu[i][c] != '\0'; c++) {
            *(volatile uint8_t *)address = (sound_menu[i][c]);
            address = (volatile uint16_t *)((volatile uint8_t *)address + 1);
        }
        if (((uint32_t)address & 1) != 0 ) {
            *(volatile uint8_t *)address = 0xFF;
            address = (volatile uint16_t *)((volatile uint8_t *)address + 1);
        } else {
            *address = 0xFFFF;
            address += 1;            
        }
        *address = 0x0005;
        address += 1;
        *address = 0x0002;
        address += 1;
    }
    *address = 0x000B;
    *BIOS_MESS_BUSY = 0;
}