#include <stdio.h>
#include "board.h"
#include "peripherals.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "fsl_debug_console.h"

// Assembly LED functions
extern void setup_leds(void);
extern void turn_off_rgb(void);
extern uint8_t get_color_value(uint8_t index);
extern void set_single_color(uint8_t color_mask);

// Button and buzzer functions
static void button_init(void);
static uint8_t button_pressed(void);
static void buzzer_init(void);
static void buzzer_on(void);
static void buzzer_off(void);

volatile uint32_t seconds = 0, minutes = 0, hours = 0;
volatile uint32_t daily_alarm_count = 0;
volatile bool tick = false, alarm_active = false;
volatile uint32_t alarm_start_time = 0;
volatile uint8_t alarm_color_index = 0;
#define BUZZER_PIN 2
#define BUZZER_PORT GPIOC
// Color constants
#define COLOR_RED 0x01
#define COLOR_GREEN 0x02
#define COLOR_BLUE 0x04
#define COLOR_YELLOW 0x03 // Red + Green (0b011)
#define COLOR_CYAN 0x06 // Green + Blue (0b110)
#define COLOR_MAGENTA 0x05 // Red + Blue (0b101)
#define COLOR_WHITE 0x07 // All colors (0b111)
// Array of all colors in rainbow order
static const uint8_t rainbow_colors[] = {
    COLOR_RED, // 0
    COLOR_YELLOW, // 1 (Red + Green)
    COLOR_GREEN, // 2
    COLOR_CYAN, // 3 (Green + Blue)
    COLOR_BLUE, // 4
    COLOR_MAGENTA, // 5 (Red + Blue)
    COLOR_WHITE // 6 (All colors)
};
#define NUM_COLORS 7

void PIT_CHANNEL_0_IRQHANDLER(void)
{
    PIT_ClearStatusFlags(PIT, PIT_CHANNEL_0, kPIT_TimerFlag);
    seconds++;
    if (alarm_active) {
       
        uint8_t color = get_color_value(alarm_color_index);
        set_single_color(color);

        alarm_color_index = (alarm_color_index + 1) % NUM_COLORS;
    }
    //time tracker
    if (seconds >= 60) {
        seconds = 0;
        minutes++;
        if (minutes >= 60) {
            minutes = 0;
            hours++;
            if (hours >= 24) {
                hours = 0;
            }
        }
        //Code to make alarm go off at 8
        if (hours == 8 && minutes == 0 && seconds == 0) {
            if (!alarm_active) {
                alarm_start_time = (hours * 3600) + (minutes * 60) + seconds;
                PRINTF("ALARM_START,Daily %d,%02d:%02d:%02d\r\n", daily_alarm_count, hours, minutes, seconds);
                buzzer_on();
                alarm_active = true;
                alarm_color_index = 0; // Start with first color
                set_single_color(rainbow_colors[0]);
                daily_alarm_count++;
            }
        }
    }
    tick = true;
}

static void button_init(void)
{
    SIM->SCGC5 |= (1 << 12);
    PORTD->PCR[11] = 0x103;
    GPIOD->PDDR &= ~(1 << 11);
}
static uint8_t button_pressed(void)
{
    return !(GPIOD->PDIR & (1 << 11));
}
//Code for how to use external buzzer generated with AI in the interest of time.
static void buzzer_init(void)
{
    SIM->SCGC5 |= SIM_SCGC5_PORTC_MASK;
    PORTC->PCR[BUZZER_PIN] = PORT_PCR_MUX(1);
    GPIOC->PDDR |= (1U << BUZZER_PIN);
    GPIOC->PDOR &= ~(1U << BUZZER_PIN);
}
//Also generated with AI
static void buzzer_on(void) { GPIOC->PSOR = (1U << BUZZER_PIN); }
static void buzzer_off(void) { GPIOC->PCOR = (1U << BUZZER_PIN); }
int main(void)
{
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitBootPeripherals();
    BOARD_InitDebugConsole();
    setup_leds();
    button_init();
    buzzer_init();
    // Start a minute before 8am
    daily_alarm_count = 0;
    hours = 7; minutes = 59; seconds = 0;
    // CSV Heading for the logger
    PRINTF("Event,Day,Time,Duration_Seconds\r\n");
    PRINTF("SYSTEM_START,Day %d,%02d:%02d:%02d,0\r\n", daily_alarm_count, hours, minutes, seconds);
    PRINTF("\n=== Daily MEDICATION REMINDER ===\r\n");
    PRINTF("Alarm will sound every minute\r\n");
    PRINTF("Press SW2 to stop alarm and log medication\r\n\n");
    PIT_StartTimer(PIT_PERIPHERAL, PIT_CHANNEL_0);
    while (1)
    {
        if (tick)
        {
            if (alarm_active && button_pressed())
            {
                for(volatile int i = 0; i < 10000; i++); // Debounce
                if (button_pressed()) {
                    uint32_t alarm_end_time = (hours * 3600) + (minutes * 60) + seconds;
                    uint32_t alarm_duration = alarm_end_time - alarm_start_time;
                    PRINTF("MEDICATION_TAKEN,Day %d,%02d:%02d:%02d,%d\r\n",
                           daily_alarm_count - 1, hours, minutes, seconds, alarm_duration);
                    turn_off_rgb();
                    buzzer_off();
                    alarm_active = false;
            
                    PRINTF(">>> Medication logged for Day %d! Next alarm: Day %d\r\n\n",
                           daily_alarm_count - 1, daily_alarm_count);
                }
            }
            tick = false;
        }
        __asm("nop");
    }
    return 0;
}
