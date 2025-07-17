ENTRY(_start)

MEMORY {
    /* System ROM */
    ROM (rx) : ORIGIN = 0xC00000, LENGTH = 0x20000
    /* System RAM */
    /* 10F300 -> 10F500 reserved zone for 'how-to-play' */
    RAM (rw) : ORIGIN = 0x10F500, LENGTH = 0xB00
}

SECTIONS {
    .vectors 0xC00000 : {
        KEEP(*(.vectors))
    } > ROM

    .bioscalls 0xC00400 : {
        KEEP(*(.bioscalls))
    } > ROM

    .text 0xC00500 : {
        *(.text._start)
        *(.text*)
    } > ROM

    .rodata : {
        *(.rodata)
        *(.rodata.*)
    } > ROM

    .data : {
        *(.data.*)
    } > RAM AT > ROM

    .bss (NOLOAD) : {
        *(.bss.*)
    } > RAM

    /DISCARD/ : {
        *(.comment)
        *(.note*)
        *(.eh_frame)
    }
}