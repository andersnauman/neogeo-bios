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
}

void show_bios_menu_soft_dips() {
    *SERVICE_CURSOR_MAX = 1;            // 1 because there are games + cabinet settings
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

        uint16_t ngh = BRAM_NGH_BLOCK[i].ngh;
        uint16_t block_id = BRAM_NGH_BLOCK[i].block;
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
        *SERVICE_CURSOR_MAX += 1;
    }

    *address = 0x0000;
    address++;
    *BIOS_MESS_POINT = (volatile uint32_t)address;

    *BIOS_MESS_BUSY = 0;
}

void update_bios_menu_soft_dips() {
    *BIOS_MESS_BUSY = 1;
    uint8_t menu_items = *SERVICE_CURSOR_MAX;
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

void show_bios_menu_soft_dips_cabinet() {
    *(uint32_t *)0x400002 = 0x0EEE0000; // Palette 0
    *BIOS_MESS_BUSY = 1;

    *(volatile uint32_t *) BIOS_MESS_BUFFER = (uint32_t)bios_menu_soft_dips_cabinet;
    *BIOS_MESS_POINT = BIOS_MESS_BUFFER_PTR + sizeof(uint32_t);

    *BIOS_MESS_BUSY = 0;
}

void update_bios_menu_soft_dips_cabinet() {

}

void show_bios_menu_soft_dips_game() {
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
}

void update_bios_menu_soft_dips_game() {
    
}
