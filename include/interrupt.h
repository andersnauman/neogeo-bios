#ifndef _INTERRUPT_H
#define _INTERRUPT_H

void __attribute__((interrupt)) vblank_handler(void);
void __attribute__((interrupt)) hblank_handler(void);
void __attribute__((interrupt)) default_handler(void);
void __attribute__((interrupt)) cold_boot_handler(void);
void __attribute__((interrupt)) bus_error_handler(void);
void __attribute__((interrupt)) address_error_handler(void);
void __attribute__((interrupt)) illegal_instruction_handler(void);
void __attribute__((interrupt)) division_by_zero_handler(void);
void __attribute__((interrupt)) chk_handler(void);
void __attribute__((interrupt)) trapv_handler(void);

#endif // _INTERRUPT_H