        .syntax unified
        .cpu cortex-m4
        .thumb

        .global setup_leds
        .global turn_on_rgb
        .global turn_off_rgb
        .global set_single_color
        .global get_color_value

@ FRDM-K66F RGB LED pins (active-low, common anode)
@ Red   → PTC9  → GPIOC bit 9
@ Green → PTE6  → GPIOE bit 6
@ Blue  → PTA11 → GPIOA bit 11

        .equ GPIOA_PDOR, 0x400FF000
        .equ GPIOC_PDOR, 0x400FF080
        .equ GPIOE_PDOR, 0x400FF100

        .equ GPIOA_PDDR, 0x400FF014
        .equ GPIOC_PDDR, 0x400FF094
        .equ GPIOE_PDDR, 0x400FF114

        .text
        .align  2

setup_leds:
        push    {lr}

        /* Enable clocks for PORTA, PORTC, PORTE */
        ldr     r0, =0x40048038         /* SIM_SCGC5 */
        ldr     r1, [r0]
        orr     r1, r1, #(1<<9)         /* PORTA */
        orr     r1, r1, #(1<<11)        /* PORTC */
        orr     r1, r1, #(1<<13)        /* PORTE */
        str     r1, [r0]

        /* Set pins to GPIO (ALT1 = 0x100) */
        ldr     r0, =0x4004902C         /* PORTA PCR11 (Blue) */
        movw    r1, #0x100
        str     r1, [r0]
        ldr     r0, =0x4004B024         /* PORTC PCR9  (Red) */
        str     r1, [r0]
        ldr     r0, =0x4004D018         /* PORTE PCR6  (Green) */
        str     r1, [r0]

        /* Direction = output */
        ldr     r0, =GPIOA_PDDR
        ldr     r1, [r0]
        orr     r1, r1, #(1<<11)
        str     r1, [r0]

        ldr     r0, =GPIOC_PDDR
        ldr     r1, [r0]
        orr     r1, r1, #(1<<9)
        str     r1, [r0]

        ldr     r0, =GPIOE_PDDR
        ldr     r1, [r0]
        orr     r1, r1, #(1<<6)
        str     r1, [r0]

        /* All LEDs off (active-low → set bits high) */
        bl      turn_off_rgb
        pop     {pc}

turn_on_rgb:
        ldr     r0, =GPIOA_PDOR
        ldr     r1, [r0]
        bic     r1, r1, #(1<<11)        /* Blue ON (low) */
        str     r1, [r0]

        ldr     r0, =GPIOC_PDOR
        ldr     r1, [r0]
        bic     r1, r1, #(1<<9)         /* Red ON (low) */
        str     r1, [r0]

        ldr     r0, =GPIOE_PDOR
        ldr     r1, [r0]
        bic     r1, r1, #(1<<6)         /* Green ON (low) */
        str     r1, [r0]
        bx      lr

turn_off_rgb:
        ldr     r0, =GPIOA_PDOR
        ldr     r1, [r0]
        orr     r1, r1, #(1<<11)        /* Blue OFF (high) */
        str     r1, [r0]

        ldr     r0, =GPIOC_PDOR
        ldr     r1, [r0]
        orr     r1, r1, #(1<<9)         /* Red OFF (high) */
        str     r1, [r0]

        ldr     r0, =GPIOE_PDOR
        ldr     r1, [r0]
        orr     r1, r1, #(1<<6)         /* Green OFF (high) */
        str     r1, [r0]
        bx      lr

/* r0 = color mask: bit0=Red, bit1=Green, bit2=Blue */
set_single_color:
        push    {r3, r4, lr}            /* Save r3 for color mask */
        mov     r3, r0                  /* Save color mask before bl overwrites r0 */
        bl      turn_off_rgb

        tst     r3, #1                  /* Use saved r3 */
        beq     1f
        ldr     r4, =GPIOC_PDOR
        ldr     r1, [r4]
        bic     r1, r1, #(1<<9)         /* Red ON (low) */
        str     r1, [r4]
1:      tst     r3, #2
        beq     2f
        ldr     r4, =GPIOE_PDOR
        ldr     r1, [r4]
        bic     r1, r1, #(1<<6)         /* Green ON (low) */
        str     r1, [r4]
2:      tst     r3, #4
        beq     3f
        ldr     r4, =GPIOA_PDOR
        ldr     r1, [r4]
        bic     r1, r1, #(1<<11)        /* Blue ON (low) */
        str     r1, [r4]
3:
        pop     {r3, r4, pc}

/* r0 = index 0-6 → returns color byte in r0 */
get_color_value:
        cmp     r0, #7
        blo     1f
        movs    r0, #0
1:      ldr     r1, =color_table
        ldrb    r0, [r1, r0]
        bx      lr

        .align  2
color_table:
        .byte   0x01    /* 0 Red */
        .byte   0x03    /* 1 Yellow */
        .byte   0x02    /* 2 Green */
        .byte   0x06    /* 3 Cyan */
        .byte   0x04    /* 4 Blue */
        .byte   0x05    /* 5 Magenta */
        .byte   0x07    /* 6 White */

        .end
