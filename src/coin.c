#include "backup_ram.h"
#include "bios.h"
#include "utils.h"

#include "coin.h"

void update_coin() {
    unlock_backup_ram();

    uint8_t coin_status = ~(*REG_STATUS_A);
    uint8_t old_previous = *BRAM_COIN_STATUS_PREVIOUS;
    uint8_t old_current = *BRAM_COIN_STATUS_CURRENT;
    *BRAM_COIN_STATUS_PREVIOUS = old_current;
    *BRAM_COIN_STATUS_CURRENT = coin_status;

    *BRAM_COIN_STATUS_CHANGE = (old_current ^ coin_status) & coin_status;

    uint8_t ripple_coin_read = coin_status & ~(old_previous | old_current);

    lock_backup_ram();

    // Check for freeplay hardware dip-switch
    if ((~(*REG_DIPSW)) & 0x40) {
        return;
    }

    uint8_t coin_inserted = 0;

    // USA - Individual coin system
    if (*BIOS_COUNTRY_CODE == BIOS_COUNTRY_USA) {
        if ((*BRAM_COIN_STATUS_CHANGE) & 0x01) {
            increase_credit(1, *BRAM_DIP_COIN_P1_GIVE);
            coin_inserted = 1;
        }
        if ((*BRAM_COIN_STATUS_CHANGE) & 0x02) {
            increase_credit(2, *BRAM_DIP_COIN_P2_GIVE);
            coin_inserted = 1;
        }
    }

    // Europe / Japan - Shared coin system
    if ((*BIOS_COUNTRY_CODE == BIOS_COUNTRY_EUROPE) || (*BIOS_COUNTRY_CODE == BIOS_COUNTRY_JAPAN)) {
        if ((*BRAM_COIN_STATUS_CHANGE) & 0x01) {
            increase_credit(1, *BRAM_DIP_COIN_P1_GIVE);
            coin_inserted = 1;
        }
        if ((*BRAM_COIN_STATUS_CHANGE) & 0x02) {
            increase_credit(1, *BRAM_DIP_COIN_P2_GIVE);
            coin_inserted = 1;
        }
    }

    if (coin_inserted > 0) {
        if (0x00 == *BRAM_DIP_GAME_START_FORCE) {
            // Reset compulsion timer
            // TODO: Not all regions have compulsion timer!
            *BIOS_COMPULSION_TIMER = *BRAM_DIP_GAME_START_TIME;
            *BIOS_COMPULSION_FRAME_TIMER = 59;   
        }

        // Play sound if we have a active game
        // TODO: Create a sound that isnt depended on a game being active?
        if (0x00 == *BIOS_SWPMODE) {
            SUBR_CART_COIN_SOUND();

            if (0x03 != *BIOS_USER_REQUEST) {
                *BIOS_USER_REQUEST = 0x03;
                SUBR_CART_DEMO_END();
                start_game();
            }            
        }
    }
}

// P1 = 1, P2 = 2 etc
void increase_credit(uint8_t player, uint8_t amount) {
    // Return if freeplay dip-switch is active
    if ((~(*REG_DIPSW)) & 0x40) {
        return;
    }
    unlock_backup_ram();
    
    int8_t credit = from_bcd8(BRAM_CREDIT[player - 1]);
    credit += amount;
    if (credit > 99) {
        credit = 99;
    }

    BRAM_CREDIT[player - 1] = to_bcd8(credit);
    lock_backup_ram();
}

void decrease_credit(uint8_t player, uint8_t amount) {
    // Return if freeplay dip-switch is active
    if ((~(*REG_DIPSW)) & 0x40) {
        return;
    }
    unlock_backup_ram();
    
    int8_t credit = from_bcd8(BRAM_CREDIT[player - 1]);
    credit -= amount;
    if (credit < 0) {
        credit = 0;
    }

    BRAM_CREDIT[player - 1] = to_bcd8(credit);
    lock_backup_ram();
}