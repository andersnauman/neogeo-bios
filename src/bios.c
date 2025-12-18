#include "backup_ram.h"
#include "bios_calls.h"
#include "bios_test.h"
#include "eyecatcher.h"
#include "graphic.h"
#include "how_to_play.h"
#include "mess_out.h"
#include "service.h"
#include "service_dips.h"
#include "service_hardware.h"
#include "utils.h"

#include "bios.h"

uint8_t menu = MENU_CROSSHATCH;

__attribute__((used, section(".text._start")))
void _start() {
    __asm__ volatile ("move #0x2700, %sr");  // Disable interrupts
    *WATCHDOG = 0;
    *IRQ_ACK = 0x07;
    *LSPCMODE = 0x4000;

    *REG_BRDFIX = 0;
    *REG_SWPBIOS = 0;
    *REG_RESETCL1 = 0;
    *REG_RESETCL2 = 0;

    test_led();
    test_work_ram();
    test_backup_ram();
    test_palette_ram();
    test_video_ram();

    *BIOS_SWPMODE = 0xFF;

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

    *BIOS_SYSRET_STATUS = 0;
    unlock_backup_ram();    
    *BRAM_SLOT_CURSOR = 8;
    if (0xFF != *BRAM_FIRST_PLAYABLE_SLOT) {
        *BRAM_SLOT_CURSOR = *BRAM_FIRST_PLAYABLE_SLOT;
        *REG_SLOT = *BRAM_FIRST_PLAYABLE_SLOT;
        *REG_SWPROM = 0;
        *BIOS_SWPMODE = 0;        
        *BIOS_USER_REQUEST = 0;                 // 0 = Initialize the default highscore etc   
        lock_backup_ram();
        SUBR_CART_USER();
    }
    lock_backup_ram();
    system_return();
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

void change_slot_incremental() {
    uint8_t slot = (*BRAM_SLOT_SELECTED + 1) & (BRAM_MAX_SLOTS - 1);
    for (int i = 0; i < BRAM_MAX_SLOTS; i++) {
        if (BRAM_NGH_BLOCK[slot].ngh != 0x0000) {
            *REG_SLOT = slot;
            unlock_backup_ram();
            *BRAM_SLOT_SELECTED = slot;
            lock_backup_ram();
            return;
        }
        slot = (slot + 1) & (BRAM_MAX_SLOTS - 1);
    }
}

void change_slot_decremental() {
    uint8_t slot = (*BRAM_SLOT_SELECTED - 1) & (BRAM_MAX_SLOTS - 1);
    for (int i = 0; i < BRAM_MAX_SLOTS; i++) {
        if (BRAM_NGH_BLOCK[slot].ngh != 0x0000) {
            *REG_SLOT = slot;
            unlock_backup_ram();
            *BRAM_SLOT_SELECTED = slot;
            lock_backup_ram();
            return;
        }
        slot = (slot - 1) & (BRAM_MAX_SLOTS - 1);
    }
}