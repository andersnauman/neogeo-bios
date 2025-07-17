volatile extern void _start(void);
volatile extern void vblank_handler(void);
volatile extern void hblank_handler(void);
volatile extern void default_handler(void);
volatile extern void cold_boot_handler(void);
volatile extern void bus_error_handler(void);
volatile extern void address_error_handler(void);
volatile extern void illegal_instruction_handler(void);
volatile extern void division_by_zero_handler(void);
volatile extern void chk_handler(void);
volatile extern void trapv_handler(void);

// 68k Vector Table - 256 bytes long

__attribute__((used, section(".vectors")))
void (* const vector_table[])(void) = {
    (void (*)(void)) 0x10F300,          // top of work ram on Neo Geo 0x10ff00
    _start,
    [0x08/4] = bus_error_handler,
    [0x0C/4] = address_error_handler,
    [0x10/4] = illegal_instruction_handler,
    [0x14/4] = division_by_zero_handler,
    [0x18/4] = chk_handler,
    [0x1C/4] = trapv_handler,
    [0x20/4 ... 0x5C/4] = default_handler,

    // --- Interrupt Vectors (IRQ Levels 1–7) ---
    // 0x060: Spurious Interrupt
    // 0x064: IRQ Level 1  V-Blank (cart) / Timer interrupt (CD).
    // 0x068: IRQ Level 2  Timer interrupt (cart) / V-Blank (CD).
    // 0x06C: IRQ Level 3  Cold boot (cart).
    // 0x070: IRQ Level 4  Unused
    // 0x074: IRQ Level 5  Unused
    // 0x078: IRQ Level 6  Unused
    // 0x07C: IRQ Level 7  Unused
    [0x60/4] = default_handler,
    [0x64/4] = vblank_handler,         // vblank-handler
    [0x68/4] = hblank_handler,         // hblank-handler
    [0x6C/4] = cold_boot_handler,
    [0x70/4] = default_handler,
    [0x74/4] = default_handler,
    [0x78/4] = default_handler,
    [0x7C/4] = default_handler,

    // TRAPs & User
    // [0x80/4] == 16bit checksum
    [0x82/4 ... 0xBF/4] = default_handler,

    // Unused / Reserved
    //[0xC0/4 ... 0xFC/4] = default_handler,

    // "NAUMAN" in reverse endian.
    (void (*)(void)) 0x20202020,
    (void (*)(void)) 0x4E205541,
    (void (*)(void)) 0x414D204E,
    (void (*)(void)) 0x20202020,

    //[0x100/4 ... 0x3FF/4] = default_handler,
};