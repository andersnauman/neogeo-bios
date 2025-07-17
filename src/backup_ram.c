#include <string.h>

#include "bios.h"
#include "utils.h"

#include "backup_ram.h"

// Mimics the original backup-reset code
void reset_backup_ram() {
    unlock_backup_ram();

    // Reset backup ram to 0x0000
    volatile uint16_t *address = BACKUP_RAM;
    volatile uint16_t *end = address + 0x8000;  // 0x8000 words = 0x10000 bytes
    while (address < end) {
        *WATCHDOG = 0;
        *address = 0x0000;
        address++;
    }

    // Write backup-string
    for (uint8_t i = 0; i < BRAM_SIGNATURE_SIZE; i++) {
        BRAM_SIGNATURE_STR[i] = _bram_signature_str[i];
    }

    // Default cab settings
    *BRAM_DIP_COIN_P1_NEED = 0x01;
    *BRAM_DIP_COIN_P1_GIVE = 0x01;
    *BRAM_DIP_COIN_P2_NEED = 0x01;
    *BRAM_DIP_COIN_P2_GIVE = 0x02;
    *BRAM_DIP_0x3E = 0x01;
    *BRAM_DIP_0x3F = 0x01;
    *BRAM_DIP_0x40 = 0x01;
    *BRAM_DIP_0x41 = 0x03;
    *BRAM_DIP_GAME_SELECT_FREE = 0x00;
    *BRAM_DIP_GAME_START_FORCE = 0x00;
    *BRAM_DIP_GAME_START_TIME = to_bcd8(30);
    *BRAM_DIP_0x45 = 0x3B;
    *BRAM_DIP_DEMO_SOUND = 0x00;
  
    // Reset saved slot data
    for (uint8_t i = 0; i < 8; i++) {
        *BRAM_SLOT_NGH_ID(i) = 0x0000FFFF;
    }

    lock_backup_ram();
}

// Writes additional values into backup-ram. Not part of the reset, but values that are added later on.
void setup_backup_ram() {
    unlock_backup_ram();

    *BRAM_PLAY_SECONDS_HEX = 0;
    *BRAM_COIN_STATUS_PREVIOUS = 0;
    *BRAM_PLAY_FRAME_TIMER = 0x3B;  // Same as BRAM_DIP_0x45? 

    // Zero out credits
    // TODO: Maybe save credit if restarted?
    volatile uint8_t *address = (volatile uint8_t *)BRAM_COIN_COUNTER_INTERNAL;
    for (uint8_t i = 0; i < 22; i++) {
        *address = 0x00;
        address++;
    }

    // TODO: Read calendar from RTC
    memcpy((void *)BIOS_CLOCK_PTR, (void *)BRAM_CLOCK_PTR, 4);
    memcpy((void *)BIOS_DATE_PTR, (void *)BRAM_DATE_PTR, 4);

    // Update the MVS slot count
    *BRAM_SLOT_COUNT = 2;
    if (((*REG_SYSTYPE) & 0x40) != 0) {
        if(((*REG_STATUS_A) & 0x20) == 0) {
            *BRAM_SLOT_COUNT = 4;
        } else {
            *BRAM_SLOT_COUNT = 6;
        }
    }

    // Find first working slot
    *BRAM_FIRST_PLAYABLE_SLOT = 0xFF;
    *REG_SWPROM = 0;
    for (uint8_t slot = 0; slot < *BRAM_SLOT_COUNT; slot++) {
        *REG_SLOT = slot;
        if (*ROM_NGH_NUMER != 0xFFFF) {
            if (0xFF == *BRAM_FIRST_PLAYABLE_SLOT) {
                *BRAM_FIRST_PLAYABLE_SLOT = slot;
            }
            // TODO: Verify security string 
            break;
        }
    }
    *REG_SWPBIOS = 0;    
    
    lock_backup_ram();
}

void lock_backup_ram() {
    if (*BIOS_BRAM_USED) {
        *BIOS_BRAM_USED = 0;
    }
    *BACKUP_RAM_LOCK = 0;
}

void unlock_backup_ram() {
    *BIOS_BRAM_USED = 1;
    *BACKUP_RAM_UNLOCK = 0;
}

void init_game_data() {
    load_game_data();

    *REG_SWPROM = 0;
    *BIOS_SWPMODE = 0x0;
    *BIOS_USER_REQUEST = 0;
    *BIOS_SYSRET_STATUS = 0;
    // With BIOS_USER_REQUEST = 0 this initialize the default highscore etc
    // Will not return here but call the system_return()
    SUBR_CART_USER();
}

// Loads game data from workram. Tries to reserve a block if none is assigned yet.
// Before calling this function, make sure you set REG_SWPROM
void load_game_data() {
    unlock_backup_ram();

    *REG_SWPROM = 0;
    *BIOS_SWPMODE = 0x0;

    uint8_t block = find_game_data_block();

    // If we did not find an already reserved block, take the next available and fill it with data
    if (0xFF == block) {
        for (uint8_t i = 0; i < 8; i++) {
            if ((uint16_t) *BRAM_SLOT_NGH_ID(i) != 0x0000) {
                block = i;
                break;
            }
        }

        // TODO: Make sure to add an error if there is no more available block
        if (0xFF == block) {
            return;
        }

        // Insert the NGH-number and selected slot
        *BRAM_SLOT_NGH_ID(block) = (*ROM_NGH_NUMER << 16) | (uint16_t)block;

        // Insert game name and dip settings
        if (*BIOS_COUNTRY_CODE == BIOS_COUNTRY_JAPAN) {
            memcpy((void *)BRAM_GAME_NAME(block), (const void *) *ROM_JPN_SOFTDIP, 16);
            memcpy((void *)BRAM_GAME_DIP_SETTINGS(block), (const void *) *ROM_JPN_SOFTDIP + 16, 16);
        } else if (*BIOS_COUNTRY_CODE == BIOS_COUNTRY_USA) {
            memcpy((void *)BRAM_GAME_NAME(block), (const void *) *ROM_USA_SOFTDIP, 16);
            memcpy((void *)BRAM_GAME_DIP_SETTINGS(block), (const void *) *ROM_USA_SOFTDIP + 16, 16);
        } else if (*BIOS_COUNTRY_CODE == BIOS_COUNTRY_EUROPE) {
            memcpy((void *)BRAM_GAME_NAME(block), (const void *) *ROM_EU_SOFTDIP, 16);
            memcpy((void *)BRAM_GAME_DIP_SETTINGS(block), (const void *) *ROM_EU_SOFTDIP + 16, 16);
        }
    }

    // Copy backup-data into workram
    volatile uint8_t *bram_game_data = BRAM_GAME_BLOCK(block);
    volatile uint8_t *rom_game_data = (volatile uint8_t *) *ROM_BACKUP_RAM_PTR;

    volatile uint16_t size = *ROM_GAME_SAVE_SIZE;
    if (size > 0x1000) {
        size = (uint16_t)BRAM_GAME_BLOCK_SIZE;
    }

    for (uint16_t i = 0; i < size; i++) {
        rom_game_data[i] = bram_game_data[i];
    }

    // Load dip settings
    // Get timed/counter options
    for (uint8_t i = 0; i < 6; i++) {
        BIOS_GAME_DIP[i] = BRAM_GAME_DIP_SETTINGS(block)[i];
    }

    // Get default values for all 10 potential soft dips.
    for (uint8_t i = 6; i < 16; i++) {
        BIOS_GAME_DIP[i] = BRAM_GAME_DIP_SETTINGS(block)[i] >> 4;
    }    

    *REG_SWPBIOS = 0;
    *BIOS_SWPMODE = 0xFF;

    lock_backup_ram();
}

// Saves game data from workram
// Before calling this function, make sure you set REG_SWPROM
void save_game_data() {
    unlock_backup_ram();

    *REG_SWPROM = 0;
    *BIOS_SWPMODE = 0x00;

    uint8_t block = find_game_data_block();

    if (0xFF == block) {
        return;
    }

    // Copy game data into backup
    volatile uint8_t *bram_game_data = BRAM_GAME_BLOCK(block);
    volatile uint8_t *rom_game_data = (volatile uint8_t *) *ROM_BACKUP_RAM_PTR;

    volatile uint16_t size = *ROM_GAME_SAVE_SIZE;
    if (size > 0x1000) {
        size = (uint16_t)BRAM_GAME_BLOCK_SIZE;
    }

    for (uint16_t i = 0; i < size; i++) {
        bram_game_data[i] = rom_game_data[i];
    }

    *REG_SWPBIOS = 0;
    *BIOS_SWPMODE = 0xFF;

    lock_backup_ram();
}

uint8_t find_game_data_block() {
    uint8_t block = 0xFF;
    for (uint8_t i = 0; i < 8; i++) {
        uint16_t bram_ngh_id = (uint16_t) ((*BRAM_SLOT_NGH_ID(i) & 0xFFFF0000) >> 16);
        if (bram_ngh_id == *ROM_NGH_NUMER) {
            block = i;
            break;
        }
    }
    return block;
}