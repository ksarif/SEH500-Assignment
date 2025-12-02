.syntax unified
.cpu cortex-m4
.thumb

.global setup_leds
.global turn_on_rgb
.global turn_off_rgb

@ FRDM-K66F RGB LED pins (active-low)
@ Blue  → PTA11 (PORTA_PCR11, GPIOA_PDOR bit11)
@ Red   → PTC9  (PORTC_PCR9,  GPIOC_PDOR bit9)
@ Green → PTE6  (PORTE_PCR6,  GPIOE_PDOR bit6)

setup_leds:
    @ Enable clocks: PORTA(9), PORTC(11), PORTE(13) in SIM_SCGC5 (0x40048038)
    ldr     r0, =0x40048038
    ldr     r1, [r0]
    orr     r1, r1, #(1<<9)   @ PORTA
    orr     r1, r1, #(1<<11)  @ PORTC
    orr     r1, r1, #(1<<13)  @ PORTE
    str     r1, [r0]

    @ Pin mux to GPIO (0x100 = ALT1)
    ldr     r0, =0x4004902C    @ PORTA_PCR11 (Blue)
    movs    r1, #0x100
    str     r1, [r0]

    ldr     r0, =0x4004B024    @ PORTC_PCR9 (Red)
    str     r1, [r0]

    ldr     r0, =0x4004D018    @ PORTE_PCR6 (Green)
    str     r1, [r0]

    @ Set as outputs in PDDR
    ldr     r0, =0x400FF014    @ GPIOA_PDDR
    ldr     r1, [r0]
    orr     r1, r1, #(1<<11)
    str     r1, [r0]

    ldr     r0, =0x400FF094    @ GPIOC_PDDR
    ldr     r1, [r0]
    orr     r1, r1, #(1<<9)
    str     r1, [r0]

    ldr     r0, =0x400FF114    @ GPIOE_PDDR
    ldr     r1, [r0]
    orr     r1, r1, #(1<<6)
    str     r1, [r0]

    bx      lr

turn_on_rgb:                 @ Active-low: Clear bits = ON
    ldr     r0, =0x400FF000    @ GPIOA_PDOR
    ldr     r1, [r0]
    bic     r1, r1, #(1<<11)   @ PTA11=0 (Blue ON)
    str     r1, [r0]

    ldr     r0, =0x400FF080    @ GPIOC_PDOR
    ldr     r1, [r0]
    bic     r1, r1, #(1<<9)    @ PTC9=0 (Red ON)
    str     r1, [r0]

    ldr     r0, =0x400FF100    @ GPIOE_PDOR
    ldr     r1, [r0]
    bic     r1, r1, #(1<<6)    @ PTE6=0 (Green ON)
    str     r1, [r0]

    bx      lr

turn_off_rgb:                @ Active-high: Set bits = OFF
    ldr     r0, =0x400FF000
    ldr     r1, [r0]
    orr     r1, r1, #(1<<11)
    str     r1, [r0]

    ldr     r0, =0x400FF080
    ldr     r1, [r0]
    orr     r1, r1, #(1<<9)
    str     r1, [r0]

    ldr     r0, =0x400FF100
    ldr     r1, [r0]
    orr     r1, r1, #(1<<6)
    str     r1, [r0]

    bx      lr

.end
