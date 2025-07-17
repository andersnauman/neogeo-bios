.section ".bioscalls", "ax"

    /* System */
#if defined(SYSTEM_AES)
    .byte 0x00          /* 0x400 */
#elif defined(SYSTEM_MVS)
    .byte 0x80          /* 0x400 */
#endif

    /* Region */
#if defined(REGION_JAPAN)
    .byte 0x00          /* 0x401 */
#elif defined(REGION_USA)
    .byte 0x01          /* 0x401 */
#elif defined(REGION_EUROPE)
    .byte 0x02          /* 0x401 */
#endif

    jmp _start          /* 0x402 */
    jmp _start          /* 0x408 */
    jmp _start          /* 0x40E */
    jmp _start          /* 0x414 */
    jmp _start          /* 0x41A */
    jmp _start          /* 0x420 */
    jmp _start          /* 0x426 */
    jmp _start          /* 0x42C */
    jmp _start          /* 0x432 */

    jmp vblank_handler  /* 0x438 */
    jmp hblank_handler  /* 0x43E */
    jmp system_return   /* 0x444 */
    jmp system_io       /* 0x44A */
    jmp credit_check    /* 0x450 */
    jmp credit_down     /* 0x456 */

#if defined(SYSTEM_AES)
    rts                 /* 0x45C */
    rts                 /* 0x462 */
#elif defined(SYSTEM_MVS)
    jmp read_calendar   /* 0x45C */
    jmp setup_calendar  /* 0x462 */
#endif

    jmp card            /* 0x468 */
    jmp card_error      /* 0x46E */
    jmp how_to_play     /* 0x474 */
    jmp _start          /* 0x47A */
    
    rts                 /* 0x480 */
    nop
    nop
    rts                 /* 0x486 */
    nop
    nop
    rts                 /* 0x48C */
    nop
    nop
    rts                 /* 0x492 */
    nop
    nop
    rts                 /* 0x498 */
    nop
    nop
    rts                 /* 0x49E */
    nop
    nop
    rts                 /* 0x4A4 */
    nop
    nop
    rts                 /* 0x4AA */
    nop
    nop
    rts                 /* 0x4B0 */
    nop
    nop
    rts                 /* 0x4B6 */
    nop
    nop
    rts                 /* 0x4BC */
    nop
    nop

    jmp fix_clear       /* 0x4C2 */
    jmp sprite_clear    /* 0x4C8 */
    jmp mess_out        /* 0x4CE */
    jmp controller_setup/* 0x4D4 */
    rts                 /* 0x4DA */
    nop
    nop

.section .note.GNU-stack,"",@progbits
