#ifndef _GRAPHIC_H
#define _GRAPHIC_H

#include <stdint.h>

#define PALETTE_ADDR                ((volatile uint16_t *) 0x400000) /* 0x400000-0x401FFF */
#define PALETTE_15                  ((volatile uint16_t *) 0x4001e2) /* Palette 15 */
#define PALETTE_OFFSCREEN_BORDER    ((volatile uint16_t *) 0x400004)
#define PALETTE_REFERENCE           ((volatile uint16_t *) 0x400000)
#define PALETTE_BACKGROUND          ((volatile uint16_t *) 0x401FFE)

void draw_fix_tile(uint8_t x, uint8_t y, uint16_t tile_number, uint8_t palette);
void set_palette(uint16_t palette, uint16_t color1);
void print_string(uint8_t start_x, uint8_t start_y, const char *str);

static const char _player_1_str[] = "PLAYER 1";
static const char _player_2_str[] = "PLAYER 2";
static const char _left_str[] = "LEFT";
static const char _right_str[] = "RIGHT";
static const char _up_str[] = "UP";
static const char _down_str[] = "DOWN";
static const char _start_str[] = "START";
static const char _select_str[] = "SELECT";
static const char _coin1_str[] = "COIN 1";
static const char _coin2_str[] = "COIN 2";
static const char _test_str[] = "TEST";
static const char _service_str[] = "SERVICE";
static const char _dips_str[] = "DIPS";

#endif // _GRAPHIC_H