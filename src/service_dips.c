#include <stdint.h>

#include "backup_ram.h"
#include "bios.h"
#include "mess_out.h"
#include "service.h"
#include "utils.h"

#include "service_dips.h"

void show_bios_menu_hard_dips() {
    *(uint32_t *)0x400002 = 0x0EEE0000; // Palette 0
    *BIOS_MESS_BUSY = 1;

    *(volatile uint32_t *) BIOS_MESS_BUFFER = (uint32_t)bios_menu_hard_dips;
    *BIOS_MESS_POINT = BIOS_MESS_BUFFER_PTR + sizeof(uint32_t);
    
    *BIOS_MESS_BUSY = 0;
}

void update_bios_menu_hard_dips() {
    *BIOS_MESS_BUSY = 1;

    volatile uint16_t *address = (volatile uint16_t *)*BIOS_MESS_POINT;
    *address++ = 0x0000;
    *address++ = 0x0000;

    *address++ = 0x2002;

    // Setting mode
    *address++ = 0x0003;
    *address++ = 0x724c;
    *address++ = 0x0108;
    *address++ = (*REG_DIPSW & 0x01) ? 0x30FF : 0x31FF;
    *address++ = 0x0003;
    *address++ = 0x738c;
    *address++ = 0x0108;
    uint8_t i = *REG_DIPSW & 0x01 ? 1 : 0;
    for (uint8_t c = 0; bios_menu_on_off[i][c] != '\0'; c++) {
        *(volatile uint8_t *)address = (bios_menu_on_off[i][c]);
        address = (volatile uint16_t *)((volatile uint8_t *)address + 1);
    }
    if (((uint32_t)address & 1) != 0 ) {
        *(volatile uint8_t *)address = 0xFF;
        address = (volatile uint16_t *)((volatile uint8_t *)address + 1);
    } else {
        *address++ = 0xFFFF;
    }

    // Controller
    *address++ = 0x0003;
    *address++ = 0x726e;
    *address++ = 0x0108;
    *address++ = (*REG_DIPSW & 0x02) ? 0x30FF : 0x31FF;
    *address++ = 0x0003;
    *address++ = 0x738e;
    *address++ = 0x0108;
    i = *REG_DIPSW & 0x02 ? 1 : 0;
    for (uint8_t c = 0; bios_menu_on_off[i][c] != '\0'; c++) {
        *(volatile uint8_t *)address = (bios_menu_on_off[i][c]);
        address = (volatile uint16_t *)((volatile uint8_t *)address + 1);
    }
    if (((uint32_t)address & 1) != 0 ) {
        *(volatile uint8_t *)address = 0xFF;
        address = (volatile uint16_t *)((volatile uint8_t *)address + 1);
    } else {
        *address++ = 0xFFFF;
    }   

    // Chute
    *address++ = 0x0003;
    *address++ = 0x7290;
    *address++ = 0x0108;
    *address++ = (*REG_DIPSW & 0x04) ? 0x30FF : 0x31FF;
    *address++ = 0x0003;
    *address++ = 0x7390;
    *address++ = 0x0108;
    i = *REG_DIPSW & 0x04 ? 1 : 0;
    for (uint8_t c = 0; bios_menu_on_off[i][c] != '\0'; c++) {
        *(volatile uint8_t *)address = (bios_menu_on_off[i][c]);
        address = (volatile uint16_t *)((volatile uint8_t *)address + 1);
    }
    if (((uint32_t)address & 1) != 0 ) {
        *(volatile uint8_t *)address = 0xFF;
        address = (volatile uint16_t *)((volatile uint8_t *)address + 1);
    } else {
        *address++ = 0xFFFF;
    }        

    // Communication - Dip 1
    *address++ = 0x0003;
    *address++ = 0x72b2;
    *address++ = 0x0108;
    *address++ = (*REG_DIPSW & 0x08) ? 0x30FF : 0x31FF;
    
    // Communication - Dip 2
    *address++ = 0x0003;
    *address++ = 0x72d2;
    *address++ = 0x0108;
    *address++ = (*REG_DIPSW & 0x10) ? 0x30FF : 0x31FF;
    *address++ = 0x0003;
    *address++ = 0x7412;
    *address++ = 0x0108;
    i = (~(*REG_DIPSW >> 3)) & 0x03;
    for (uint8_t c = 0; bios_menu_communication_link[i][c] != '\0'; c++) {
        *(volatile uint8_t *)address = (bios_menu_communication_link[i][c]);
        address = (volatile uint16_t *)((volatile uint8_t *)address + 1);
    }
    if (((uint32_t)address & 1) != 0 ) {
        *(volatile uint8_t *)address = 0xFF;
        address = (volatile uint16_t *)((volatile uint8_t *)address + 1);
    } else {
        *address++ = 0xFFFF;
    }

    // Communication - Dip 3
    *address++ = 0x0003;
    *address++ = 0x72f2;
    *address++ = 0x0108;
    *address++ = (*REG_DIPSW & 0x20) ? 0x30FF : 0x31FF;
    *address++ = 0x0003;
    *address++ = 0x7392;
    *address++ = 0x0108;
    i = (*REG_DIPSW & 0x20) ? 1 : 0;
    for (uint8_t c = 0; bios_menu_on_off[i][c] != '\0'; c++) {
        *(volatile uint8_t *)address = (bios_menu_on_off[i][c]);
        address = (volatile uint16_t *)((volatile uint8_t *)address + 1);
    }
    if (((uint32_t)address & 1) != 0 ) {
        *(volatile uint8_t *)address = 0xFF;
        address = (volatile uint16_t *)((volatile uint8_t *)address + 1);
    } else {
        *address++ = 0xFFFF;
    }

    // Free play
    *address++ = 0x0003;
    *address++ = 0x7314;
    *address++ = 0x0108;
    *address++ = (*REG_DIPSW & 0x40) ? 0x30FF : 0x31FF;
    *address++ = 0x0003;
    *address++ = 0x7394;
    *address++ = 0x0108;
    i = *REG_DIPSW & 0x40 ? 1 : 0;
    for (uint8_t c = 0; bios_menu_on_off[i][c] != '\0'; c++) {
        *(volatile uint8_t *)address = (bios_menu_on_off[i][c]);
        address = (volatile uint16_t *)((volatile uint8_t *)address + 1);
    }
    if (((uint32_t)address & 1) != 0 ) {
        *(volatile uint8_t *)address = 0xFF;
        address = (volatile uint16_t *)((volatile uint8_t *)address + 1);
    } else {
        *address++ = 0xFFFF;
    }

    // Stop mode / Freeze
    *address++ = 0x0003;
    *address++ = 0x7336;
    *address++ = 0x0108;
    *address++ = (*REG_DIPSW & 0x80) ? 0x30FF : 0x31FF;
    *address++ = 0x0003;
    *address++ = 0x7396;
    *address++ = 0x0108;
    i = *REG_DIPSW & 0x80 ? 1 : 0;
    for (uint8_t c = 0; bios_menu_on_off[i][c] != '\0'; c++) {
        *(volatile uint8_t *)address = (bios_menu_on_off[i][c]);
        address = (volatile uint16_t *)((volatile uint8_t *)address + 1);
    }
    if (((uint32_t)address & 1) != 0 ) {
        *(volatile uint8_t *)address = 0xFF;
        address = (volatile uint16_t *)((volatile uint8_t *)address + 1);
    } else {
        *address++ = 0xFFFF;
    }

    // End
    *address++ = 0x0000;
    *BIOS_MESS_POINT = (volatile uint32_t)address;

    *BIOS_MESS_BUSY = 0;
}

void show_bios_menu_soft_dips() {
    *SERVICE_CURSOR_MAX = 1;            // 1 because there are games + cabinet settings
    *(uint32_t *)0x400002 = 0x0EEE0000; // Palette 0
    *(uint32_t *)0x400022 = 0x0E000000; // Palette 1
    *BIOS_MESS_BUSY = 1;

    *(volatile uint32_t *) BIOS_MESS_BUFFER = (uint32_t)bios_menu_soft_dips;
    *BIOS_MESS_POINT = BIOS_MESS_BUFFER_PTR + sizeof(uint32_t);

    volatile uint16_t *address = (volatile uint16_t *)*BIOS_MESS_POINT;
    *address++ = 0x0000;
    *address++ = 0x0000;

    for (uint8_t i = 0; i < *BRAM_SLOT_COUNT; i++) {
        *address++ = 0x0005;
        *address++ = 0x0002;
        *address++ = 0x0108;
        *address++ = 0x534c;
        *address++ = 0x4f54;
        *address++ = 0x2031 + i;
        *address++ = 0x3A20;

        uint16_t ngh = BRAM_NGH_BLOCK[i].ngh;
        uint16_t block_id = BRAM_NGH_BLOCK[i].block;
        if (ngh != 0x0000) {
            volatile uint8_t *game_name = BRAM_GAME_NAME(block_id);
            for (uint8_t c = 0; c < 0x10; c++) {
                *(volatile uint8_t *)address = game_name[c];
                address = (volatile uint16_t *)((volatile uint8_t *)address + 1);
            }
            *address++ = 0xFFFF;
        } else {
            *address++ = 0x4E2F;
            *address++ = 0x41FF;
        }
        *SERVICE_CURSOR_MAX += 1;
    }

    *address++ = 0x0000;
    *BIOS_MESS_POINT = (volatile uint32_t)address;

    *BIOS_MESS_BUSY = 0;
}

void update_bios_menu_soft_dips() {
    *BIOS_MESS_BUSY = 1;
    uint8_t menu_items = *SERVICE_CURSOR_MAX;
    _move_cursor(menu_items, 0);

    volatile uint16_t *address = (volatile uint16_t *)*BIOS_MESS_POINT;
    *address++ = 0x0000;
    *address++ = 0x0000;

    *address++ = 0x0003;
    *address++ = 0x70E5;

    for (int8_t i = 0; i < menu_items; i++) {
        if (i != *SERVICE_CURSOR) {
            *address++ = 0x0108;
            *address++ = 0x20FF;  // Space (overwrite arrow)
        } else {
            *address++ = 0x1108;
            *address++ = 0x11FF;  // Arrow
        }
        if (menu_items - 1 != i) {
            *address++ = 0x0005;
            *address++ = 0x0002;
        }
    }

    *address++ = 0x0000;
    *BIOS_MESS_POINT = (volatile uint32_t)address;

    *BIOS_MESS_BUSY = 0;
}

void show_bios_menu_soft_dips_cabinet() {
    *(uint32_t *)0x400002 = 0x0EEE0000; // Palette 0
    *(uint32_t *)0x400022 = 0x0E000000; // Palette 1
    *BIOS_MESS_BUSY = 1;

    *(volatile uint32_t *) BIOS_MESS_BUFFER = (uint32_t)bios_menu_soft_dips_cabinet;
    *BIOS_MESS_POINT = BIOS_MESS_BUFFER_PTR + sizeof(uint32_t);

    *BIOS_MESS_BUSY = 0;
}

void update_bios_menu_soft_dips_cabinet() {
    *BIOS_MESS_BUSY = 1;
    uint8_t menu_items = 5;
    // TODO: Make the side_items-value dynamic. d = [1 2 1 2 2]; d[*SERVICE_CURSOR]
    _move_cursor(menu_items, 1);

    volatile uint16_t *address = (volatile uint16_t *)*BIOS_MESS_POINT;
    *address++ = 0x0000;
    *address++ = 0x0000;

    *address++ = 0x0003;
    *address++ = 0x7068;

    for (int8_t i = 0; i < menu_items; i++) {
        if (i != *SERVICE_CURSOR) {
            *address++ = 0x0108;
            *address++ = 0x20FF;  // Space (overwrite arrow)
        } else {
            *address++ = 0x1108;
            *address++ = 0x11FF;  // Arrow
        }
        if (menu_items - 1 != i) {
            *address++ = 0x0005;
            *address++ = 0x0002;
        }
    }

    // If 'A' button is pressed
    if ((*BIOS_P1CHANGE & A_BUTTON) != 0) {
        unlock_backup_ram();
        if (0 == *SERVICE_CURSOR && *SERVICE_CURSOR_SIDEWAYS == 0) {
            if (*BRAM_DIP_COIN_P1_NEED >= 9) {
                *BRAM_DIP_COIN_P1_NEED = 1;
            } else {
                *BRAM_DIP_COIN_P1_NEED += 1;
            }
        } else if (0 == *SERVICE_CURSOR && *SERVICE_CURSOR_SIDEWAYS == 1) {
            if (*BRAM_DIP_COIN_P1_GIVE >= 9) {
                *BRAM_DIP_COIN_P1_GIVE = 1;
            } else {
                *BRAM_DIP_COIN_P1_GIVE += 1;
            }
        } else if (1 == *SERVICE_CURSOR && *SERVICE_CURSOR_SIDEWAYS == 0) {
            if (*BRAM_DIP_COIN_P2_NEED >= 9) {
                *BRAM_DIP_COIN_P2_NEED = 1;
            } else {
                *BRAM_DIP_COIN_P2_NEED += 1;
            }
        } else if (1 == *SERVICE_CURSOR && *SERVICE_CURSOR_SIDEWAYS == 1) {
            if (*BRAM_DIP_COIN_P2_GIVE >= 9) {
                *BRAM_DIP_COIN_P2_GIVE = 1;
            } else {
                *BRAM_DIP_COIN_P2_GIVE += 1;
            }
        } else if (2 == *SERVICE_CURSOR) {
            *BRAM_DIP_GAME_SELECT_FREE ^= 1;
        } else if (3 == *SERVICE_CURSOR) {
            uint8_t seconds = from_bcd8(*BRAM_DIP_COMPULSION_TIMER);
            if (seconds >= 60) {
                seconds = 0;
                *BRAM_DIP_GAME_START_FORCE ^= 1;
            } else {
                if (seconds == 0) {
                    *BRAM_DIP_GAME_START_FORCE ^= 1;
                }
                seconds++;
            }
            *BRAM_DIP_COMPULSION_TIMER = to_bcd8(seconds);
        } else if (4 == *SERVICE_CURSOR) {
            *BRAM_DIP_DEMO_SOUND ^= 1;
        }
        lock_backup_ram();
    // If 'B' button is pressed
    } else if ((*BIOS_P1CHANGE & B_BUTTON) != 0) {
        unlock_backup_ram();
        if (0 == *SERVICE_CURSOR && *SERVICE_CURSOR_SIDEWAYS == 0) {
            if (*BRAM_DIP_COIN_P1_NEED <= 1) {
                *BRAM_DIP_COIN_P1_NEED = 9;
            } else {
                *BRAM_DIP_COIN_P1_NEED -= 1;
            }
        } else if (0 == *SERVICE_CURSOR && *SERVICE_CURSOR_SIDEWAYS == 1) {
            if (*BRAM_DIP_COIN_P1_GIVE <= 1) {
                *BRAM_DIP_COIN_P1_GIVE = 9;
            } else {
                *BRAM_DIP_COIN_P1_GIVE -= 1;
            }
        } else if (1 == *SERVICE_CURSOR && *SERVICE_CURSOR_SIDEWAYS == 0) {
            if (*BRAM_DIP_COIN_P2_NEED <= 1) {
                *BRAM_DIP_COIN_P2_NEED = 9;
            } else {
                *BRAM_DIP_COIN_P2_NEED -= 1;
            }
        } else if (1 == *SERVICE_CURSOR && *SERVICE_CURSOR_SIDEWAYS == 1) {
            if (*BRAM_DIP_COIN_P2_GIVE <= 1) {
                *BRAM_DIP_COIN_P2_GIVE = 9;
            } else {
                *BRAM_DIP_COIN_P2_GIVE -= 1;
            }
        } else if (2 == *SERVICE_CURSOR) {
            *BRAM_DIP_GAME_SELECT_FREE ^= 1;
        } else if (3 == *SERVICE_CURSOR) {
            uint8_t seconds = from_bcd8(*BRAM_DIP_COMPULSION_TIMER);
            if (seconds == 0) {
                seconds = 60;
                *BRAM_DIP_GAME_START_FORCE ^= 1;
            } else {
                if (seconds == 1) {
                    *BRAM_DIP_GAME_START_FORCE ^= 1;
                }
                seconds--;
            }
            *BRAM_DIP_COMPULSION_TIMER = to_bcd8(seconds);
        } else if (4 == *SERVICE_CURSOR) {
            *BRAM_DIP_DEMO_SOUND ^= 1;
        }
        lock_backup_ram();
    }

    // Coin1
    *address++ = 0x0003;
    *address++ = 0x7248;
    *address++ = ((*SERVICE_CURSOR == 0 && *SERVICE_CURSOR_SIDEWAYS == 0) ? 0x1108 : 0x0108);
    *address++ = ((uint16_t)(*BRAM_DIP_COIN_P1_NEED + 0x30) << 8) | 0x00FF;
    *address++ = 0x0003;
    *address++ = 0x7348;
    *address++ = ((*SERVICE_CURSOR == 0 && *SERVICE_CURSOR_SIDEWAYS == 1) ? 0x1108 : 0x0108);
    *address++ = ((uint16_t)(*BRAM_DIP_COIN_P1_GIVE + 0x30) << 8) | 0x00FF;
    // Pural 's'
    *address++ = 0x0003;
    *address++ = 0x72e8;
    *address++ = 0x0108;
    if (*BRAM_DIP_COIN_P1_NEED > 1) {
        *address++ = 0x53FF;
    } else {
        *address++ = 0x20FF;
    }
    *address++ = 0x0003;
    *address++ = 0x7428;
    *address++ = 0x0108;
    if (*BRAM_DIP_COIN_P1_GIVE > 1) {
        *address++ = 0x53FF;
    } else {
        *address++ = 0x20FF;
    }

    // Coin2
    *address++ = 0x0003;
    *address++ = 0x724b;
    *address++ = ((*SERVICE_CURSOR == 1 && *SERVICE_CURSOR_SIDEWAYS == 0) ? 0x1108 : 0x0108);
    *address++ = ((uint16_t)(*BRAM_DIP_COIN_P2_NEED + 0x30) << 8) | 0x00FF;
    *address++ = 0x0003;
    *address++ = 0x734b;
    *address++ = ((*SERVICE_CURSOR == 1 && *SERVICE_CURSOR_SIDEWAYS == 1) ? 0x1108 : 0x0108);
    *address++ = ((uint16_t)(*BRAM_DIP_COIN_P2_GIVE + 0x30) << 8) | 0x00FF;
    // Pural 's'
    *address++ = 0x0003;
    *address++ = 0x72eb;
    *address++ = 0x0108;
    if (*BRAM_DIP_COIN_P2_NEED > 1) {
        *address++ = 0x53FF;
    } else {
        *address++ = 0x20FF;
    }
    *address++ = 0x0003;
    *address++ = 0x742b;
    *address++ = 0x0108;
    if (*BRAM_DIP_COIN_P2_GIVE > 1) {
        *address++ = 0x53FF;
    } else {
        *address++ = 0x20FF;
    }

    // Game select
    address = _add_string(address, (*SERVICE_CURSOR == 2 ? 1 : 0), 0x724e, bios_menu_soft_dips_cabinet_game_select[*BRAM_DIP_GAME_SELECT_FREE]);

    // Game start (compulsion)
    address = _add_string(address, (*BRAM_DIP_GAME_START_FORCE == 1 ? 1 : 0), 0x7391, bios_menu_soft_dips_cabinet_game_start[*BRAM_DIP_GAME_START_FORCE]);
    if (0 == *BRAM_DIP_GAME_START_FORCE) {
        address = _add_large_char(address, (*SERVICE_CURSOR == 3 ? 1 : 0), 0x7391, *BRAM_DIP_COMPULSION_TIMER);
    }

    // Demo sound
    address = _add_string(address, (*SERVICE_CURSOR == 4 ? 1 : 0), 0x7254, bios_menu_soft_dips_cabinet_demo_sound[*BRAM_DIP_DEMO_SOUND]);

    *address++ = 0x0000;
    *BIOS_MESS_POINT = (volatile uint32_t)address;

    *BIOS_MESS_BUSY = 0;
}

void show_bios_menu_soft_dips_game() {
    *SERVICE_CURSOR_MAX = 0;
    *(uint32_t *)0x400002 = 0x0EEE0000; // Palette 0
    *(uint32_t *)0x400022 = 0x0E000000; // Palette 1
    *BIOS_MESS_BUSY = 1;

    uint8_t menu_items = 1;

    volatile uint16_t *address = (uint16_t *)BIOS_MESS_BUFFER_PTR;
    *address = 0x0000;
    address++;
    *address = 0x0000;
    address++;

    // Read text from cartridge for dips options
    *REG_SWPROM = 0;
    *REG_SLOT = *SOFT_DIPS_GAME_SELECT;

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
    for (uint8_t i = 0; i < 0x10; i++) {
        *(volatile uint8_t *)address = *dips;
        address = (volatile uint16_t *)((volatile uint8_t *)address + 1);
        dips++;
    }
    *address = 0xFFFF;
    address++;

    // Option values
    uint8_t choosen_option_value [14];
    uint16_t choosen_option_offset [14];
    for (int i = 0; i < 14; i++) {
        choosen_option_offset[i] = 0xFFFF;
    }
    uint16_t *choosen_option_offset_ptr = choosen_option_offset;

    for (uint8_t i = 0; i < 14; i++) {
        // Values from BRAM is already parsed and bit-shifted
        choosen_option_value[i] = BRAM_GAME_DIP_SETTINGS(*SOFT_DIPS_GAME_SELECT)[i];
    }

    // Special time settings
    uint16_t description_offset = 32;
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
        *SERVICE_CURSOR_MAX += 1;
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
        *SERVICE_CURSOR_MAX += 1;
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
        *SERVICE_CURSOR_MAX += 1;
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
}

void update_bios_menu_soft_dips_game() {
    *BIOS_MESS_BUSY = 1;
    int8_t menu_items = *SERVICE_CURSOR_MAX;
    _move_cursor(menu_items, 0);

    volatile uint16_t *address = (volatile uint16_t *)*BIOS_MESS_POINT;
    *address = 0x0000;
    address++;
    *address = 0x0000;
    address++;

    *address = 0x0003;
    address++;
    *address = 0x70E8;
    address++;        

    for (int8_t i = 0; i < menu_items; i++) {
        if (i != *SERVICE_CURSOR) {
            *address = 0x0108;
            address++;
            *address = 0x20FF;  // Space (overwrite arrow)
            address++;
        } else {
            *address = 0x1108;
            address++;
            *address = 0x11FF;  // Arrow
            address++;
        }
        if (menu_items - 1 != i) {
            *address = 0x0005;
            address++;
            *address = 0x0002;
            address++;
        }
    }

    *address = 0x0000;
    address++;
    *BIOS_MESS_POINT = (volatile uint32_t)address;

    *BIOS_MESS_BUSY = 0;
}
