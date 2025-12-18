#include <stdint.h>

#include "bios.h"
#include "calendar.h"
#include "mess_out.h"
#include "service_hardware.h"
#include "service_dips.h"
#include "utils.h"

#include "service.h"

void show_bios_menu() {
    reset_fix_layer();
    reset_palettes();

    uint8_t menu = MENU_BIOS_MAIN;
    while(1) {
        if (menu == MENU_BIOS_MAIN) {
            show_bios_main_menu();
        } else if (menu == MENU_BIOS_HARDWARE) {
            show_bios_hardware_test();
        } else if (menu == MENU_BIOS_HARD_DIPS) {
            show_bios_menu_hard_dips();
        } else if (menu == MENU_BIOS_SOFT_DIPS) {
            show_bios_menu_soft_dips();
        } else if (menu == MENU_BIOS_EXIT) {
            reset_system();
        }
        uint8_t previous_menu = menu;
        while (previous_menu == menu) {
            if (menu == MENU_BIOS_MAIN) {
                update_bios_main_menu();
                if (((*BIOS_P1CHANGE) & 0x10) != 0) {
                    menu = MENU_BIOS_MAIN + (*SERVICE_CURSOR + 1);   // increase with one since cursor starts from 0
                }
            } else if (menu == MENU_BIOS_HARD_DIPS) {
                update_bios_menu_hard_dips();
            } else if (menu == MENU_BIOS_SOFT_DIPS) {
                update_bios_menu_soft_dips();
            }
            if (menu != MENU_BIOS_MAIN) {
                if (((*BIOS_P1CHANGE) & 0x20) != 0) {
                    menu = MENU_BIOS_MAIN;
                }
            }
            wait_for_vblank();
        }
        wait_for_vblank();
        reset_fix_layer();
        reset_palettes();
    }
}

void show_bios_main_menu() {
    *SERVICE_CURSOR = 0;

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

    _read_calendar();

    if (*BIOS_COUNTRY_CODE == BIOS_COUNTRY_JAPAN || *BIOS_COUNTRY_CODE == BIOS_COUNTRY_EUROPE) {
        address = _add_large_char(address, 0, 0x72DA, *BIOS_YEAR);
        address = _add_large_char(address, 0, 0x727A, *BIOS_MONTH);
        address = _add_large_char(address, 0, 0x721A, *BIOS_DAY);
    } else if (*BIOS_COUNTRY_CODE == BIOS_COUNTRY_USA) {
        address = _add_large_char(address, 0, 0x72DA, *BIOS_YEAR);
        address = _add_large_char(address, 0, 0x721A, *BIOS_MONTH);
        address = _add_large_char(address, 0, 0x727A, *BIOS_DAY);
    }

    *address = 0x0000;
    address++;
    *BIOS_MESS_POINT = (volatile uint32_t)address;        

    *BIOS_MESS_BUSY = 0;
}

void update_bios_main_menu() {
    *BIOS_MESS_BUSY = 1;
    uint8_t menu_items = 7;
    _move_cursor(menu_items);

    volatile uint16_t *address = (volatile uint16_t *)*BIOS_MESS_POINT;
    *address = 0x0000;
    address++;
    *address = 0x0000;
    address++;

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
        if (menu_items - 1 != i) {
            *address = 0x0005;
            address += 1;
            *address = 0x0002;
            address += 1;
        }
    }

    *address = 0x0000;
    address++;
    *BIOS_MESS_POINT = (volatile uint32_t)address;

    *BIOS_MESS_BUSY = 0;    
}

void show_bios_hardware_test() {
    uint8_t menu = MENU_CROSSHATCH;

    while(1) {
        if (menu == MENU_CROSSHATCH) {
            show_crosshatch_test();
        } else if (menu == MENU_COLOR) {
            show_color_test();
        } else if (menu == MENU_IO) {
            show_io_test();
        } else if (menu == MENU_SOUND_TEST) {
            show_sound_test();
        } else if (menu == MENU_MEMORY_CARD) {
            show_memory_card_test();
        } else if (menu == MENU_CLEAR_BACKUP) {
            show_backup_clear();
        } else if (menu == MENU_SETUP_CALENDAR) {
            show_setup_calendar();
        }

        while (((*BIOS_STATCHANGE_RAW) & 0x01) == 0) {
            if (menu == MENU_IO) {
                update_io_test();
            } else if (menu == MENU_SOUND_TEST) {
                update_sound_test();
            } else if (menu == MENU_MEMORY_CARD) {
                update_memory_card_test();
            } else if (menu == MENU_CLEAR_BACKUP) {
                update_backup_clear();
            } else if (menu == MENU_SETUP_CALENDAR) {
                update_setup_calendar();
            }
            wait_for_vblank();
        }

        menu = menu + 1;
        if (menu >= MENU_CROSSHATCH + MAX_NUM_MENUS) {
            menu = MENU_CROSSHATCH;
        }

        wait_for_vblank();

        *REG_SOUND = 0x3;   // Reset sound
        reset_fix_layer();
        reset_palettes();
    }
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