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

uint16_t frame_counter = 0;
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
        if (0 == *REG_DIPSW) {
            reset_backup_ram();
        }        
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

    // Test user-ram, why only 7fff? because 8000 * 2 == 10000
    // Test backup-ram string, if all dip switch are down, jump to test user-ram again(?) and than zero + write backup ram from fixed variables.
    // if rewrite, write 0x23 -> 0xd00122 and then 0xffff * 8 -> 0xd00124+
    // 0xf0f0f0f0 -> 0xd001ac
    // 0xffff -> 0xd001b0
    // test vram
    // If all dipswitch low
    //      loop until dipswitch not low
    // else
    // test RTC
    // wait_for_rtc_pulse_edge()
    // start frame_counter
    // enable interrupts
    // send 0x8 command to RTC
    // wait_for_rtc_pulse_edge()
    // disable interrupts
    // send 0x7 command to RTC
    // check if frame is > 57 and < 64
    // checksum-check
    //      add each byte of 0x00->0x7F to a uint8_t. watchdog each run
    //      add each byte of 0xC00082->0xC1FFFF to same uint_8. watchdog each run
    //      compare with 0xc00080
    // check memcard inserted and header string

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
            }

            while ((*BIOS_STATCHANGE_RAW & 0x01) == 0) {
                if (menu == MENU_IO) {
                    update_io_test();
                } else if (menu == MENU_SOUND_TEST) {
                    update_sound_test();
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

void software_dips() {
    // TODO: Read from BRAM instead

    // Country region from System ROM
    uint8_t region = *SROM_COUNTRY_CODE;

    // Get adress for the correct software dips based on region
    uint32_t dips_addr = ROM_SOFTDIP_TABLE[region];

    // Cast it to have uint8_t as offset when looping through values
    volatile uint8_t *dips = (volatile uint8_t *)(uintptr_t)dips_addr;
    dips = dips + 16;   // Game name = 16 byte

    // Get timed/counter options
    for (uint8_t i = 0; i < 6; i++) {
        BIOS_GAME_DIP[i] = dips[i];
    }

    // Get default values for all 10 potential soft dips.
    for (uint8_t i = 6; i < 16; i++) {
        BIOS_GAME_DIP[i] = dips[i] >> 4;
    }

/*
    Metal Slug default dipsw

    12 38 04 12 12 02 01 04 03 00
    continue        12      off/on*
    difficulty      38      1,2,3,4*,5,6,7,8
    play time       04      60*,70,80,90
    demo sound      12      off,on*
    play manual     12      off,on*
    blood           02      off*,on      

*/
}