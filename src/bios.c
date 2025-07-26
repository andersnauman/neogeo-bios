#include "backup_ram.h"
#include "bios_calls.h"
#include "bios_test.h"
#include "eyecatcher.h"
#include "graphic.h"
#include "how_to_play.h"
#include "mess_out.h"
#include "service.h"
#include "utils.h"

#include "bios.h"

uint8_t menu = MENU_CROSSHATCH;

void init() {
    __asm__ volatile ("move #0x2700, %sr");  // Disable interrupts
    *WATCHDOG = 0;
    *IRQ_ACK = 0x07;
    *LSPCMODE = 0x4000;

    *BIOS_SWPMODE = 0xFF;
    *REG_BRDFIX = 0;
    *REG_SWPBIOS = 0;
    *REG_RESETCL1 = 0;
    *REG_RESETCL2 = 0;

    test_led();
    test_work_ram();
    test_backup_ram();
    test_palette_ram();
    test_video_ram();

    uint8_t request_reset_backup = 0;
    while (0 == *REG_DIPSW) {
        test_work_ram();
        test_backup_ram();
        request_reset_backup = 1;
    }
    if (1 == request_reset_backup) {
        reset_backup_ram();
    }

    test_rtc();

    test_bios_checksum();   
    test_sound();
    test_memory_card();

    // 0x00 = AES, 0x80 = MVS
    if (*SROM_MVS_FLAG == 0x80) {
        *BIOS_MVS_FLAG = 0x01;
    } else {
        *BIOS_MVS_FLAG = 0x00;
    }
    
    *BIOS_COUNTRY_CODE = *SROM_COUNTRY_CODE;

    setup_backup_ram();
    controller_setup();

    *IRQ_ACK = 0x07;                            // Ack all interrupts
    __asm__ volatile ("move #0x2000, %sr");     // Set interrupt mask to level 2

    // If no game was found (could not read NGH-number), show test-menu
    if (*BRAM_FIRST_PLAYABLE_SLOT == 0xFF) {
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

            while ((*BIOS_STATCHANGE_RAW & 0x01) == 0) {
                if (menu == MENU_IO) {
                    update_io_test();
                } else if (menu == MENU_SOUND_TEST) {
                    update_sound_test();
                } else if (menu == MENU_MEMORY_CARD) {
                    update_memory_card_test();
                } else if (menu == MENU_SETUP_CALENDAR) {
                    update_setup_calendar();
                }
                wait_for_vblank();
            }

            menu = menu + 1;
            if (menu >= MAX_NUM_MENUS) {
                menu = MENU_CROSSHATCH;
            }
            wait_for_vblank();
            reset_fix_layer();
            reset_palettes();
        }
    }  

    *REG_SLOT = *BRAM_FIRST_PLAYABLE_SLOT;
    if (0xFF == find_game_data_block()) {
        init_game_data();
    } else {
        load_game_data();
        *BIOS_SYSRET_STATUS = 0;
        system_return();
    }
};

void start_game() {
    *BIOS_PLAYER_MOD1 = 0x00;
    *BIOS_PLAYER_MOD2 = 0x00;
    *BIOS_PLAYER_MOD3 = 0x00;
    *BIOS_PLAYER_MOD4 = 0x00;

    *IRQ_ACK = 0x07;                            // Acknowledge all interrupt
    *BIOS_SYSTEM_MODE &= 0x7F;                  // Make sure BIOS vblank interrupt is used
    __asm__ volatile ("move #0x2000, %sr");     // Enable interrupt
    *REG_SOUND = 0x03;                          // Reset z80
    wait_for_vblank();
    __asm__ volatile ("move #0x2700, %sr");     // Disable interrupt
    *BIOS_SYSTEM_MODE |= 0x80;                  // Return vblank interrupt to game

    lock_backup_ram();

    *LSPCMODE = 0x4000;
    *REG_SWPROM = 0;
    *REG_CRTFIX = 0;
    *BIOS_SWPMODE = 0;

    __asm__ volatile ("move.l #0x10F300, %sp");
    __asm__ volatile ("jmp 0x000122.w");            // 0x122 = User routine
}

void set_default_values() {
    *BIOS_INT1_SKIP = 0;        // Only used to skip input when checking for RTC-pulse with interrupts enabled?
}
