#include <string.h>

#include "bios.h"
#include "bios_calls.h"
#include "graphic.h"
#include "mess_out.h"
#include "utils.h"

#include "how_to_play.h"

void _how_to_play() {
    *BIOS_SYSTEM_MODE &= 0x7F;                  // Make sure BIOS vblank interrupt is used
    *IRQ_ACK = 0x07;
    *BIOS_MESS_BUSY = 1;

    __asm__ volatile ("move #0x2000, %sr");     // Set interrupt mask to level 2
    wait_for_z80();
    wait_for_vblank();
    __asm__ volatile ("move #0x2700, %sr");     // Disable interrupt
    *REG_SOUND = 0x3;
    *BIOS_SWPMODE = 0xFF;
    *REG_SWPBIOS = 0;
    *REG_BRDFIX = 0;
    __asm__ volatile ("move #0x2000, %sr");     // Set interrupt mask to level 2

    // Insert palettes
    for (uint8_t i = 0; i < sizeof(how_to_play_palette) / sizeof(how_to_play_palette[0]); i++) {
        *WATCHDOG = 0;
        PALETTE_RAM[i] = how_to_play_palette[i];
    }
    *PALETTE_BACKGROUND = 0x0000;

    sprite_clear();
    *BIOS_MESS_BUSY = 1;

    // Insert mess-out commands
    *BIOS_MESS_POINT = BIOS_MESS_BUFFER_PTR;
    for (uint8_t i = 0; i < sizeof(how_to_play_graphic) / sizeof(how_to_play_graphic[0]); i++) {
        *(volatile uint32_t *) *BIOS_MESS_POINT = (uintptr_t)how_to_play_graphic[i];
        *BIOS_MESS_POINT = *BIOS_MESS_POINT + sizeof(uintptr_t);        
    }

    *HTP_LOOP_COUNT = 0x00;
    uint16_t *cmd_buffer = (uint16_t *)MESS_GAME_VAR;
    uint32_t command = *(uint32_t *) cmd_buffer;
    cmd_buffer += 2;
    uint16_t *loop_timemachine = 0;

    while (command != 0x00000000) {
        while (*MESS_WAIT_COUNT > 0) {
            wait_for_vblank();
            if (((*BIOS_P1CHANGE & 0xF0) != 0x00) || ((*BIOS_P2CHANGE & 0xF0) != 0x00)) {
                goto how_to_play_end;
            }
            *MESS_WAIT_COUNT -= 1;
        }

        uint8_t argument = (uint8_t)(command & 0x00FF);
        uint8_t type = (uint8_t)((command & 0xFF00) >> 8);

        // Loop control
        if (0x00 == type) {
            if (argument != 0xFF) {
                *HTP_LOOP_COUNT = argument;
                loop_timemachine = cmd_buffer;
            } else {
                *HTP_LOOP_COUNT -= 1;
                if (*HTP_LOOP_COUNT != 0) {
                    cmd_buffer = loop_timemachine;
                } else {
                    *MESS_TEMP = (uint32_t)cmd_buffer;                    
                }
            }
        // Joystick state
        } else if (0x01 == type) {
            volatile uint16_t *start = (volatile uint16_t *)0x40000a;
            uint16_t color1 = *(volatile uint16_t *)0x40002c;
            uint16_t color2 = *(volatile uint16_t *)0x40002a;

            uint8_t carry = 0;
            for (uint8_t i = 0; i < 8; i++) {
                uint8_t carry = argument & 0x1;
                argument >>= 1;
                if (0 == carry) {
                    start[i] = color1;
                } else {
                    start[i] = color2;
                }
            }            
        // Button state
        } else if (0x02 == type) {
            *BIOS_MESS_BUSY = 1;
            for (uint8_t i = 0; i < 4; i++) {
                uint8_t carry = argument & 0x1;
                argument >>= 1;
                uint8_t array_offset = i * 2;
                if (0 == carry) {
                    *(volatile uint32_t *) *BIOS_MESS_POINT = (uintptr_t)how_to_play_buttons[array_offset];
                    *BIOS_MESS_POINT = *BIOS_MESS_POINT + sizeof(uintptr_t);
                } else {
                    *(volatile uint32_t *) *BIOS_MESS_POINT = (uintptr_t)how_to_play_buttons[array_offset+1];
                    *BIOS_MESS_POINT = *BIOS_MESS_POINT + sizeof(uintptr_t);
                }
            }
            *BIOS_MESS_BUSY = 0;
        // Text update
        } else {
            *BIOS_MESS_BUSY = 1;
            *(volatile uint32_t *) *BIOS_MESS_POINT = command;
            *BIOS_MESS_POINT = *BIOS_MESS_POINT + sizeof(uintptr_t);
            *BIOS_MESS_BUSY = 0;
            *MESS_WAIT_COUNT = (*cmd_buffer) & 0x00FF;
            cmd_buffer += 1;
        }
        command = *(uint32_t *) cmd_buffer;
        cmd_buffer += 2;
        if (command == 0x00000000 && *HTP_LOOP_COUNT > 0) {
            *MESS_WAIT_COUNT = (*cmd_buffer) & 0x00FF;
            cmd_buffer += 1;
            command = *(uint32_t *) cmd_buffer;
            cmd_buffer += 2;
        }
    }

how_to_play_end:
    *(PALETTE_ADDR + 1) = 0x00000000;
    reset_fix_layer();
    wait_for_z80();
    wait_for_vblank();
    *REG_SOUND = 0x3;

    *REG_CRTFIX = 0;
    *REG_SWPROM = 0;
    *BIOS_SWPMODE = 0x00;
    *BIOS_SYSTEM_MODE |= 0x80;
}