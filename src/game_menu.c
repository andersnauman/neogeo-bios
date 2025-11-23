#include "backup_ram.h"
#include "bios.h"
#include "mess_out.h"
#include "utils.h"

#include "game_menu.h"

void game_menu(void) {
    if (*BIOS_GAME_MENU != 1) {
        return;
    }
    if (*BIOS_GAME_MENU_TOGGLE == 1) {
        menu_init();
        *BIOS_GAME_MENU_TOGGLE = 0;
    }    
    menu_update();
}

void menu_init() {
    //*REG_SHADOW = 0;
    *BIOS_GAME_PALETTE_1 = *(uint32_t *)0x400020;
    uint16_t pos = BIOS_GAME_MENU_START_POSITION;
    for (uint16_t x = 0; x < BIOS_GAME_MENU_COLUMN_SIZE; x++) {
        for (uint16_t y = 0; y < BIOS_GAME_MENU_ROW_SIZE; y++) {
            *REG_VRAMADDR = pos + y;
            BIOS_GAME_MENU_SPRITE_BACKUP[(x * BIOS_GAME_MENU_ROW_SIZE) + y] = *REG_VRAMRW;
        }
        pos += 0x20;
    }
}

void menu_shutdown() {
    //*REG_NOSHADOW = 0;
    *(uint32_t *)0x400020 = *BIOS_GAME_PALETTE_1;
    uint16_t pos = BIOS_GAME_MENU_START_POSITION;
    for (uint16_t x = 0; x < BIOS_GAME_MENU_COLUMN_SIZE; x++) {
        for (uint16_t y = 0; y < BIOS_GAME_MENU_ROW_SIZE; y++) {
            *REG_VRAMADDR = pos + y;
            *REG_VRAMRW = BIOS_GAME_MENU_SPRITE_BACKUP[(x * BIOS_GAME_MENU_ROW_SIZE) + y];
        }
        pos += 0x20;
    }    
}

void menu_update() {
    *(uint32_t *)0x400020 = 0x0000FFFF; // Palette 1, color 0-1
    *(uint32_t *)0x400024 = 0x01110000; // Palette 1, color 2-3

    *BIOS_MESS_BUSY = 1;

    *(volatile uint32_t *) BIOS_MESS_BUFFER = (uint32_t)game_menu_box;
    *BIOS_MESS_POINT = BIOS_MESS_BUFFER_PTR + sizeof(uint32_t);

    volatile uint16_t *address = (volatile uint16_t *)*BIOS_MESS_POINT;
    *address = 0x0000;
    address++;
    *address = 0x0000;
    address++;
    *address = 0x0001;
    address++;
    *address = 0x10ff;  // Palette 1
    address++;
    *address = 0x2002;  // Increase each write with a column
    address++;
    *address = 0x0003;
    address++;
    *address = BIOS_GAME_MENU_START_POSITION + 32 + 1;    // Add one column and one row
    address++;
    *address = 0x0007;
    address++;

    uint8_t block = find_game_data_block();

    for (uint8_t i = 0; i < 16; i++) {
        *(volatile uint8_t *)address = BRAM_GAME_NAME(block)[i];
        address = (volatile uint16_t *)((volatile uint8_t *)address + 1);
    }
    *address = 0xFFFF;
    address++;

    *address = 0x0003;
    address++;
    *address = BIOS_GAME_MENU_START_POSITION + 32 + 2;    // Add one column and two rows
    address++;
    *address = 0x0007;
    address++;
    // Insert game name and dip settings
    uint8_t region = *SROM_COUNTRY_CODE;    
    uint32_t dips_addr = ROM_SOFTDIP_TABLE[region];
    volatile uint8_t *dips = (volatile uint8_t *)(uint32_t)dips_addr;
    dips += 32;
    for (uint8_t i = 0; i < 12; i++) {
        *(volatile uint8_t *)address = *dips;
        address = (volatile uint16_t *)((volatile uint8_t *)address + 1);
        dips++; 
    }

    *address = 0xFFFF;
    address++;

    *address = 0x0000;
    address++;
    *BIOS_MESS_POINT = (volatile uint32_t)address;

    *BIOS_MESS_BUSY = 0;    
}

void menu_toggle() {
    if (*BIOS_GAME_MENU != 1) {
        *BIOS_GAME_MENU_TOGGLE = 1;
        *BIOS_GAME_MENU = 1;
        *REG_SWPBIOS = 0;
        *BIOS_GAME_LSPCMODE = *REG_LSPCMODE;
        *REG_LSPCMODE  = (uint16_t)(*BIOS_GAME_LSPCMODE & ~LSPC_TIMER_EN);
    } else {
        *BIOS_GAME_MENU_TOGGLE = 0;
        menu_shutdown();
        *REG_LSPCMODE = *BIOS_GAME_LSPCMODE;
        *REG_SWPROM = 0;
        *BIOS_GAME_MENU = 0;
    }    
}

uint8_t menu_hotkey_pressed() {
    // P1 select
    if (((*BIOS_STATCHANGE_RAW) & 0x02) != 0) {
        return 1;
    }
    return 0;
}