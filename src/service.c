#include <stdint.h>

#include "backup_ram.h"
#include "bios.h"
#include "calendar.h"
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
    *address = 0x71ca;
    address += 1;

    // Print the cursor
    for (uint8_t i = 0; i < 4; i++) {
        if (i == *SERVICE_SOUND_CURSOR) {
            *address = ((uint16_t)0x11 << 8) | 0x08;    // Palette 1, additional 0x1100 to char
            address += 1;
            *address = 0x11FF;                          // Arrow
            address += 1;
        } else {
            *address = ((uint16_t)0x01 << 8) | 0x08;    // Palette 0, additional 0x0100 to char
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
    *address = 0x724a;
    address += 1;

    // Print the text
    for (uint8_t i = 0; i < 4; i++) {
        // Set the color for the selected option
        if (i == *SERVICE_SOUND_SELECT) {
            *address = ((uint16_t)0x11 << 8) | 0x08;    // Palette 1, additional 0x1100 to char
            address += 1;
        } else {
            *address = ((uint16_t)0x01 << 8) | 0x08;    // Palette 0, additional 0x0100 to char
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

void show_memory_card_test() {
    *(uint32_t *)0x400002 = 0x0EEE0000; // Palette 0
    *(uint32_t *)0x400022 = 0x0EEE0000; // Palette 1

    *BIOS_MESS_BUSY = 1;

    *(volatile uint32_t *) BIOS_MESS_BUFFER = (uint32_t)memory_card;
    *BIOS_MESS_POINT = BIOS_MESS_BUFFER_PTR + sizeof(uint32_t);

    *BIOS_MESS_BUSY = 0;
}

void update_memory_card_test() {
    uint8_t frame_count_low = *BIOS_FRAME_COUNTER_LOW;
    if ((frame_count_low & 0x1F) == 0) {
        *(uint32_t *)0x400022 ^= 0x0EEE0000;
    }
}

void show_backup_clear() {
    *(uint32_t *)0x400002 = 0x0EEE0000; // Palette 0
    *(uint32_t *)0x400022 = 0x0EEE0000; // Palette 1

    *BIOS_MESS_BUSY = 1;

    *(volatile uint32_t *) BIOS_MESS_BUFFER = (uint32_t)backup_clear;
    *BIOS_MESS_POINT = BIOS_MESS_BUFFER_PTR + sizeof(uint32_t);

    *BIOS_MESS_BUSY = 0;
}

void update_backup_clear() {
    volatile uint8_t start = 0;
    // P1 pressed 'A', 'B', 'C'
    if ((*BIOS_P1CHANGE & 0x70) != 0) {
        reset_backup_ram();
        wait_for_vblank();
        *(uint32_t *)0x400022 = 0x0E000000; // Palette 1
        start = *BIOS_FRAME_COUNTER_LOW;
    }

    // Keep the pressed color for a short period of time and then reset it back.
    if (0 != start) {
        while ((volatile uint8_t)(*BIOS_FRAME_COUNTER_LOW - start) < 0x3F) {
            wait_for_vblank();
        }
        *(uint32_t *)0x400022 = 0x0EEE0000; // Palette 1
    }
}

void show_setup_calendar() {
    *(uint32_t *)0x400002 = 0x0EEE0000; // Palette 0
    *(uint32_t *)0x400022 = 0x0E000000; // Palette 1

    *BIOS_MESS_BUSY = 1;

    *(volatile uint32_t *) BIOS_MESS_BUFFER = (uint32_t)service_setup_calendar;
    *BIOS_MESS_POINT = BIOS_MESS_BUFFER_PTR + sizeof(uint32_t);

    _read_calendar();
    for (uint8_t i = 0; i < 7; i++) {
        *NEW_BIOS_DATE(i) = *BIOS_DATE(i);
    }

    *BIOS_MESS_BUSY = 0;

    *SERVICE_CALENDAR_CURSOR = 1;
}

void update_setup_calendar() {
    _read_calendar();
    *BIOS_MESS_BUSY = 1;

    volatile uint16_t *address = (volatile uint16_t *)*BIOS_MESS_POINT;
    *address = 0x0000;
    address++;
    *address = 0x0000;
    address++;

    // Current date section
    address = _add_large_char(address, 0, 0x722C, *BIOS_YEAR);
    address = _add_large_char(address, 0, 0x716C, *BIOS_MONTH);
    address = _add_large_char(address, 0, 0x71CC, *BIOS_DAY);
    address = _add_large_char(address, 0, 0x728C, *BIOS_HOUR);
    address = _add_large_char(address, 0, 0x72EC, *BIOS_MINUTE);
    address = _add_large_char(address, 0, 0x734C, *BIOS_SECOND);

    // +1 = down, -1 = up, 0 = none or both
    int8_t dir = (!!(*BIOS_P1CHANGE & 0x02)) - (!!(*BIOS_P1CHANGE & 0x01));

    if (dir && (*SERVICE_CALENDAR_CURSOR & 0x01)) {
        uint8_t month = from_bcd8(*NEW_BIOS_MONTH);
        if (dir < 0) {
            month = (month == 12) ? 1 : (month + 1);
        } else if (dir > 0) {
            month = (month == 1) ? 12 : (month - 1);
        }
        *NEW_BIOS_MONTH = to_bcd8(month);
    } else if (dir && (*SERVICE_CALENDAR_CURSOR & 0x02)) {
        uint8_t day = from_bcd8(*NEW_BIOS_DAY);
        if (dir < 0) {
            day = (day == 31) ? 1 : (day + 1);
        } else if (dir > 0) {
            day = (day == 1) ? 31 : (day - 1);
        }
        *NEW_BIOS_DAY = to_bcd8(day);
    } else if (dir && (*SERVICE_CALENDAR_CURSOR & 0x04)) {
        uint8_t year = from_bcd8(*NEW_BIOS_YEAR);
        if (dir < 0) {
            year = (year == 99) ? 0 : (year + 1);
        } else if (dir > 0) {
            year = (year == 0) ? 99 : (year - 1);
        }
        *NEW_BIOS_YEAR = to_bcd8(year);
    } else if (dir && (*SERVICE_CALENDAR_CURSOR & 0x08)) {
        uint8_t hour = from_bcd8(*NEW_BIOS_HOUR);
        if (dir < 0) {
            hour = (hour == 23) ? 0 : (hour + 1);
        } else if (dir > 0) {
            hour = (hour == 0) ? 23 : (hour - 1);
        }
        *NEW_BIOS_HOUR = to_bcd8(hour);
    } else if (dir && (*SERVICE_CALENDAR_CURSOR & 0x10)) {
        uint8_t minute = from_bcd8(*NEW_BIOS_MINUTE);
        if (dir < 0) {
            minute = (minute == 59) ? 0 : (minute + 1);
        } else if (dir > 0) {
            minute = (minute == 0) ? 59 : (minute - 1);
        }
        *NEW_BIOS_MINUTE = to_bcd8(minute);
    } else if (dir && (*SERVICE_CALENDAR_CURSOR & 0x20)) {
        uint8_t second = from_bcd8(*NEW_BIOS_SECOND);
        if (dir < 0) {
            second = (second == 59) ? 0 : (second + 1);
        } else if (dir > 0) {
            second = (second == 0) ? 59 : (second - 1);
        }
        *NEW_BIOS_SECOND = to_bcd8(second);
    }

    // P1 pressed left
    if ((*BIOS_P1CHANGE & 0x4) != 0) {
        if (*SERVICE_CALENDAR_CURSOR != 1) {
            *SERVICE_CALENDAR_CURSOR >>= 1;
        }
    // P1 pressed right
    } else if ((*BIOS_P1CHANGE & 0x8) != 0) {
        if (*SERVICE_CALENDAR_CURSOR != 32) {
            *SERVICE_CALENDAR_CURSOR <<= 1;
        }
    }

    // P1 pressed 'D', save the time
    if ((*BIOS_P1CHANGE & 0x80) != 0) {
        *BIOS_YEAR = *NEW_BIOS_YEAR;
        *BIOS_MONTH = *NEW_BIOS_MONTH;
        *BIOS_DAY = *NEW_BIOS_DAY;
        *BIOS_HOUR = *NEW_BIOS_HOUR;
        *BIOS_MINUTE = *NEW_BIOS_MINUTE;
        *BIOS_SECOND = *NEW_BIOS_SECOND;
        _setup_calendar();
    }

    // Change date section
    address = _add_large_char(address, (*SERVICE_CALENDAR_CURSOR & 1  ? 1 : 0), 0x7173, *NEW_BIOS_MONTH);
    address = _add_large_char(address, (*SERVICE_CALENDAR_CURSOR & 2  ? 1 : 0), 0x71D3, *NEW_BIOS_DAY);
    address = _add_large_char(address, (*SERVICE_CALENDAR_CURSOR & 4  ? 1 : 0), 0x7233, *NEW_BIOS_YEAR);
    address = _add_large_char(address, (*SERVICE_CALENDAR_CURSOR & 8  ? 1 : 0), 0x7293, *NEW_BIOS_HOUR);
    address = _add_large_char(address, (*SERVICE_CALENDAR_CURSOR & 16 ? 1 : 0), 0x72F3, *NEW_BIOS_MINUTE);
    address = _add_large_char(address, (*SERVICE_CALENDAR_CURSOR & 32 ? 1 : 0), 0x7353, *NEW_BIOS_SECOND);

    *address = 0x0000;
    address++;
    *BIOS_MESS_POINT = (volatile uint32_t)address;

    *BIOS_MESS_BUSY = 0;
}

volatile uint16_t * _add_large_char(volatile uint16_t *address, uint8_t selected, uint16_t position, uint16_t value) {
    *address = 0x0003;
    address++;
    *address = position;
    address++;
    if (selected == 1) {    
        *address = 0x1108;
    } else {
        *address = 0x0108;
    }
    address++;
    uint8_t upper = (value >> 4) & 0x0F;
    uint8_t lower = value & 0x0F;
    *address = ((upper + 0x30) << 8) | (lower + 0x30);
    address++;
    *address = 0xFFFF;
    address++;
    return address;
}

void _move_cursor(uint8_t max_menu_items) {
    // Menu "go-up"
    if ((*BIOS_P1CHANGE & 0x1) != 0) {
        *SERVICE_CURSOR -= 1;
        if (*SERVICE_CURSOR < 0) {
            *SERVICE_CURSOR = max_menu_items - 1;
        }
    // Menu "go-down"
    } else if ((*BIOS_P1CHANGE & 0x2) != 0) {
        *SERVICE_CURSOR += 1;
        if (*SERVICE_CURSOR > (max_menu_items - 1)) {
            *SERVICE_CURSOR = 0;
        }
    }
}

void show_bios_menu() {
    reset_fix_layer();
    reset_palettes();
    *SERVICE_CURSOR = 0;
    uint8_t menu_items = 7;
    _read_calendar();
    while(1) {
        *WATCHDOG = 0;
        *(uint32_t *)0x400002 = 0x0EEE0000; // Palette 0
        *(uint32_t *)0x400022 = 0x0E000000; // Palette 1
        *BIOS_MESS_BUSY = 1;

        *(volatile uint32_t *) BIOS_MESS_BUFFER = (uint32_t)bios_menu;
        *BIOS_MESS_POINT = BIOS_MESS_BUFFER_PTR + sizeof(uint32_t);

        volatile uint16_t *address = (volatile uint16_t *)*BIOS_MESS_POINT;
        *address = 0x0000;
        address++;
        *address = 0x0000;
        address++;

        if (*BIOS_COUNTRY_CODE == BIOS_COUNTRY_JAPAN || *BIOS_COUNTRY_CODE == BIOS_COUNTRY_EUROPE) {
            address = _add_large_char(address, 0, 0x72DA, *BIOS_YEAR);
            address = _add_large_char(address, 0, 0x727A, *BIOS_MONTH);
            address = _add_large_char(address, 0, 0x721A, *BIOS_DAY);
        } else if (*BIOS_COUNTRY_CODE == BIOS_COUNTRY_USA) {
            address = _add_large_char(address, 0, 0x72DA, *BIOS_YEAR);
            address = _add_large_char(address, 0, 0x721A, *BIOS_MONTH);
            address = _add_large_char(address, 0, 0x727A, *BIOS_DAY);
        }

        _move_cursor(menu_items);
   
        *address = 0x0003;
        address += 1;
        *address = 0x70E5;
        address += 1;        
        for (uint8_t i = 0; i < menu_items; i++) {
            if (i != *SERVICE_CURSOR) {
                *address = 0x0108;
                address++;
                *address = 0x20FF;  // Space (overwrite arrow)
                address += 1;
            } else {
                *address = 0x1108;
                address += 1;
                *address = 0x11FF;  // Arrow
                address += 1;
            }
            *address = 0x0005;
            address += 1;
            *address = 0x0002;
            address += 1;
        }
        *address = 0x0000;
        address++;
        *BIOS_MESS_POINT = (volatile uint32_t)address;        

        *BIOS_MESS_BUSY = 0;
        wait_for_vblank();
    }
}

void show_bios_menu_hard_dips() {
    reset_fix_layer();
    reset_palettes();
    while(1) {
        *WATCHDOG = 0;
        *(uint32_t *)0x400002 = 0x0EEE0000; // Palette 0
        *BIOS_MESS_BUSY = 1;

        *(volatile uint32_t *) BIOS_MESS_BUFFER = (uint32_t)bios_menu_hard_dips;
        *BIOS_MESS_POINT = BIOS_MESS_BUFFER_PTR + sizeof(uint32_t);

        volatile uint16_t *address = (volatile uint16_t *)*BIOS_MESS_POINT;
        *address = 0x0000;
        address++;
        *address = 0x0000;
        address++;
        *address = 0x2002;
        address += 1;
        // Setting mode        
        *address = 0x0003;
        address += 1;
        *address = 0x724c;
        address += 1;
        *address = 0x0108;
        address += 1;
        *address = (*REG_DIPSW & 0x01) ? 0x30FF : 0x31FF;
        address += 1;
        *address = 0x0003;
        address += 1;
        *address = 0x738c;
        address += 1;
        *address = 0x0108;
        address += 1;        
        uint8_t i = *REG_DIPSW & 0x01 ? 1 : 0;
        for (uint8_t c = 0; bios_menu_on_off[i][c] != '\0'; c++) {
            *(volatile uint8_t *)address = (bios_menu_on_off[i][c]);
            address = (volatile uint16_t *)((volatile uint8_t *)address + 1);
        }
        if (((uint32_t)address & 1) != 0 ) {
            *(volatile uint8_t *)address = 0xFF;
            address = (volatile uint16_t *)((volatile uint8_t *)address + 1);
        } else {
            *address = 0xFFFF;
            address += 1;
        }

        // Controller
        *address = 0x0003;
        address += 1;
        *address = 0x726e;
        address += 1;
        *address = 0x0108;
        address += 1;
        *address = (*REG_DIPSW & 0x02) ? 0x30FF : 0x31FF;
        address += 1;
        *address = 0x0003;
        address += 1;
        *address = 0x738e;
        address += 1;
        *address = 0x0108;
        address += 1;        
        i = *REG_DIPSW & 0x02 ? 1 : 0;
        for (uint8_t c = 0; bios_menu_on_off[i][c] != '\0'; c++) {
            *(volatile uint8_t *)address = (bios_menu_on_off[i][c]);
            address = (volatile uint16_t *)((volatile uint8_t *)address + 1);
        }
        if (((uint32_t)address & 1) != 0 ) {
            *(volatile uint8_t *)address = 0xFF;
            address = (volatile uint16_t *)((volatile uint8_t *)address + 1);
        } else {
            *address = 0xFFFF;
            address += 1;
        }   

        // Chute
        *address = 0x0003;
        address += 1;
        *address = 0x7290;
        address += 1;
        *address = 0x0108;
        address += 1;
        *address = (*REG_DIPSW & 0x04) ? 0x30FF : 0x31FF;
        address += 1;
        *address = 0x0003;
        address += 1;
        *address = 0x7390;
        address += 1;
        *address = 0x0108;
        address += 1;        
        i = *REG_DIPSW & 0x04 ? 1 : 0;
        for (uint8_t c = 0; bios_menu_on_off[i][c] != '\0'; c++) {
            *(volatile uint8_t *)address = (bios_menu_on_off[i][c]);
            address = (volatile uint16_t *)((volatile uint8_t *)address + 1);
        }
        if (((uint32_t)address & 1) != 0 ) {
            *(volatile uint8_t *)address = 0xFF;
            address = (volatile uint16_t *)((volatile uint8_t *)address + 1);
        } else {
            *address = 0xFFFF;
            address += 1;
        }        

        // Communication - Dip 1
        *address = 0x0003;
        address += 1;
        *address = 0x72b2;
        address += 1;
        *address = 0x0108;
        address += 1;
        *address = (*REG_DIPSW & 0x08) ? 0x30FF : 0x31FF;
        address += 1;
        
        // Communication - Dip 2
        *address = 0x0003;
        address += 1;
        *address = 0x72d2;
        address += 1;
        *address = 0x0108;
        address += 1;
        *address = (*REG_DIPSW & 0x10) ? 0x30FF : 0x31FF;
        address += 1;
        *address = 0x0003;
        address += 1;
        *address = 0x7412;
        address += 1;
        *address = 0x0108;
        address += 1;        
        i = (~(*REG_DIPSW >> 3)) & 0x03;
        for (uint8_t c = 0; bios_menu_communication_link[i][c] != '\0'; c++) {
            *(volatile uint8_t *)address = (bios_menu_communication_link[i][c]);
            address = (volatile uint16_t *)((volatile uint8_t *)address + 1);
        }
        if (((uint32_t)address & 1) != 0 ) {
            *(volatile uint8_t *)address = 0xFF;
            address = (volatile uint16_t *)((volatile uint8_t *)address + 1);
        } else {
            *address = 0xFFFF;
            address += 1;
        }

        // Communication - Dip 3
        *address = 0x0003;
        address += 1;
        *address = 0x72f2;
        address += 1;
        *address = 0x0108;
        address += 1;
        *address = (*REG_DIPSW & 0x20) ? 0x30FF : 0x31FF;
        address += 1;
        *address = 0x0003;
        address += 1;
        *address = 0x7392;
        address += 1;
        *address = 0x0108;
        address += 1;        
        i = *REG_DIPSW & 0x20 ? 1 : 0;
        for (uint8_t c = 0; bios_menu_on_off[i][c] != '\0'; c++) {
            *(volatile uint8_t *)address = (bios_menu_on_off[i][c]);
            address = (volatile uint16_t *)((volatile uint8_t *)address + 1);
        }
        if (((uint32_t)address & 1) != 0 ) {
            *(volatile uint8_t *)address = 0xFF;
            address = (volatile uint16_t *)((volatile uint8_t *)address + 1);
        } else {
            *address = 0xFFFF;
            address += 1;
        }

        // Free play
        *address = 0x0003;
        address += 1;
        *address = 0x7314;
        address += 1;
        *address = 0x0108;
        address += 1;        
        *address = (*REG_DIPSW & 0x40) ? 0x30FF : 0x31FF;
        address += 1;
        *address = 0x0003;
        address += 1;
        *address = 0x7394;
        address += 1;
        *address = 0x0108;
        address += 1;        
        i = *REG_DIPSW & 0x40 ? 1 : 0;
        for (uint8_t c = 0; bios_menu_on_off[i][c] != '\0'; c++) {
            *(volatile uint8_t *)address = (bios_menu_on_off[i][c]);
            address = (volatile uint16_t *)((volatile uint8_t *)address + 1);
        }
        if (((uint32_t)address & 1) != 0 ) {
            *(volatile uint8_t *)address = 0xFF;
            address = (volatile uint16_t *)((volatile uint8_t *)address + 1);
        } else {
            *address = 0xFFFF;
            address += 1;
        }

        // Stop mode / Freeze
        *address = 0x0003;
        address += 1;
        *address = 0x7336;
        address += 1;
        *address = 0x0108;
        address += 1;
        *address = (*REG_DIPSW & 0x80) ? 0x30FF : 0x31FF;
        address += 1;
        *address = 0x0003;
        address += 1;
        *address = 0x7396;
        address += 1;
        *address = 0x0108;
        address += 1;        
        i = *REG_DIPSW & 0x80 ? 1 : 0;
        for (uint8_t c = 0; bios_menu_on_off[i][c] != '\0'; c++) {
            *(volatile uint8_t *)address = (bios_menu_on_off[i][c]);
            address = (volatile uint16_t *)((volatile uint8_t *)address + 1);
        }
        if (((uint32_t)address & 1) != 0 ) {
            *(volatile uint8_t *)address = 0xFF;
            address = (volatile uint16_t *)((volatile uint8_t *)address + 1);
        } else {
            *address = 0xFFFF;
            address += 1;
        }

        // End
        *address = 0x0000;
        address++;
        *BIOS_MESS_POINT = (volatile uint32_t)address;

        *BIOS_MESS_BUSY = 0;
    
        wait_for_vblank();
    }
}

void update_bios_menu_hard_dips() {
    
}

void show_bios_menu_soft_dips() {
    reset_fix_layer();
    reset_palettes();
    *SERVICE_CURSOR = 0;
    while(1) {
        uint8_t menu_items = 1;

        *WATCHDOG = 0;
        *(uint32_t *)0x400002 = 0x0EEE0000; // Palette 0
        *(uint32_t *)0x400022 = 0x0E000000; // Palette 1
        *BIOS_MESS_BUSY = 1;

        *(volatile uint32_t *) BIOS_MESS_BUFFER = (uint32_t)bios_menu_soft_dips;
        *BIOS_MESS_POINT = BIOS_MESS_BUFFER_PTR + sizeof(uint32_t);

        volatile uint16_t *address = (volatile uint16_t *)*BIOS_MESS_POINT;

        *address = 0x0000;
        address++;
        *address = 0x0000;
        address++;
        for (uint8_t i = 0; i < *BRAM_SLOT_COUNT; i++) {
            *address = 0x0005;
            address++;
            *address = 0x0002;
            address++;            
            *address = 0x0108;
            address++;
            *address = 0x534c;
            address++;
            *address = 0x4f54;
            address++;
            *address = 0x2031 + i;
            address++;
            *address = 0x3A20;
            address++;   
            volatile uint32_t ngh_id = *BRAM_SLOT_NGH_ID(i);
            uint16_t ngh = (uint16_t)(ngh_id >> 16);
            uint16_t block_id = (uint16_t)(ngh_id & 0xFFFF);
            if (ngh != 0x0000) {
                volatile uint8_t *game_name = BRAM_GAME_NAME(block_id);
                for (uint8_t c = 0; c < 0x10; c++) {
                    *(volatile uint8_t *)address = game_name[c];
                    address = (volatile uint16_t *)((volatile uint8_t *)address + 1);
                }
                *address = 0xFFFF;
                address++;
            } else {
                *address = 0x4E2F;
                address++;
                *address = 0x41FF;
                address++;
            }
            menu_items += 1;
        }

        _move_cursor(menu_items);

        *address = 0x0003;
        address += 1;
        *address = 0x70E5;
        address += 1;        
        for (uint8_t i = 0; i < menu_items; i++) {
            if (i != *SERVICE_CURSOR) {
                *address = 0x0108;
                address++;
                *address = 0x20FF;  // Space (overwrite arrow)
                address += 1;
            } else {
                *address = 0x1108;
                address += 1;
                *address = 0x11FF;  // Arrow
                address += 1;
            }
            *address = 0x0005;
            address += 1;
            *address = 0x0002;
            address += 1;
        }

        *address = 0x0000;
        address++;
        *BIOS_MESS_POINT = (volatile uint32_t)address;
        *BIOS_MESS_BUSY = 0;
        wait_for_vblank();
    }
}

void update_bios_menu_soft_dips() {

}

void show_bios_menu_game_soft_dips() {
    reset_fix_layer();
    reset_palettes();
    *SERVICE_CURSOR = 0;
    while(1) {
        uint8_t menu_items = 1;

        *WATCHDOG = 0;
        *(uint32_t *)0x400002 = 0x0EEE0000; // Palette 0
        *(uint32_t *)0x400022 = 0x0E000000; // Palette 1
        *BIOS_MESS_BUSY = 1;

        volatile uint16_t *address = (uint16_t *)BIOS_MESS_BUFFER_PTR;

        *address = 0x0000;
        address++;
        *address = 0x0000;
        address++;

        *REG_SWPROM = 0;
        *REG_SLOT = 0;  // TODO: Make dynamic

        uint8_t region = *SROM_COUNTRY_CODE;
        uint32_t dips_addr = ROM_SOFTDIP_TABLE[region];
        volatile uint8_t *dips = (volatile uint8_t *)dips_addr;

        *address = 0x0003;
        address++;
        *address = 0x7145;
        address++;
        *address = 0x0108;
        address++;

        // Game name
        for (uint8_t i = 0; i < 16; i++) {
            *(volatile uint8_t *)address = *dips;
            address = (volatile uint16_t *)((volatile uint8_t *)address + 1);
            dips++;
        }
        *address = 0xFFFF;
        address++;

        uint16_t description_offset = 32;

        uint8_t choosen_option_value [14];
        uint16_t choosen_option_offset [14];
        for (int i = 0; i < 14; i++) {
            choosen_option_offset[i] = 0xFFFF;
        }        
        uint16_t *choosen_option_offset_ptr = choosen_option_offset;

        for (uint8_t i = 0; i < 14; i++) {
            // Values from BRAM is already parsed and bit-shifted
            choosen_option_value[i] = BRAM_GAME_DIP_SETTINGS(0)[i];
        }

        // Special time settings
        for (uint8_t i = 0; i < 2; i++) {
            uint16_t setting = *(uint16_t *) dips;
            dips += 2;
            if (setting == 0xFFFF) {
                continue;
            }
            *address = 0x0005;
            address++;
            *address = 0x0002;
            address++;            
            *address = 0x0108;
            address++;
            volatile uint8_t *start = (volatile uint8_t *)dips_addr + description_offset;
            for (uint8_t c = 0; c < 0xC; c++) {
                *(volatile uint8_t *)address = start[c];
                address = (volatile uint16_t *)((volatile uint8_t *)address + 1);
            }
            *address = 0xFFFF;
            address++;
            description_offset += 0xC;
        }

        // Special count settings
        for (uint8_t i = 0; i < 2; i++) {
            uint8_t setting = *dips;
            dips++;
            if (setting == 0xFF) {
                continue;
            }
            *address = 0x0005;
            address++;
            *address = 0x0002;
            address++;   
            *address = 0x0108;
            address++;
            volatile uint8_t *start = (volatile uint8_t *)dips_addr + description_offset;
            for (uint8_t c = 0; c < 0xC; c++) {
                *(volatile uint8_t *)address = start[c];
                address = (volatile uint16_t *)((volatile uint8_t *)address + 1);
            }
            *address = 0xFFFF;
            address++;
            description_offset += 0xC;
        }

        // Simple settings
        for (uint8_t i = 0; i < 7; i++) {
            uint8_t setting = *dips;
            dips++;
            if (setting == 0x00) {
                continue;
            }
            uint8_t choices = setting & 0x0F;
            *address = 0x0005;
            address++;
            *address = 0x0002;
            address++;   
            *address = 0x0108;
            address++;
            volatile uint8_t *start = (volatile uint8_t *)dips_addr + description_offset;
            for (uint8_t c = 0; c < 0xC; c++) {
                *(volatile uint8_t *)address = start[c];
                address = (volatile uint16_t *)((volatile uint8_t *)address + 1);
            }
            *address = 0xFFFF;
            address++;
            *choosen_option_offset_ptr = description_offset + 0xC;
            choosen_option_offset_ptr++;            
            description_offset += 0xC + (0xC * choices);
        }

        // Values
        *address = 0x0003;
        address++;
        *address = 0x72c5;
        address++;
        for (uint8_t i = 0; i < 2; i++) {
            *address = 0x0108;
            address++;
            uint8_t value = choosen_option_value[i];
            *address = ((uint16_t)value << 8) | 0xFF;
            address++;
        }

        *REG_SWPBIOS = 0;

        *address = 0x0000;
        address++;
        *BIOS_MESS_POINT = (volatile uint32_t)address;
        *BIOS_MESS_BUSY = 0;
        wait_for_vblank();
    }
}