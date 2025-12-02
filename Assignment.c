#include <stdio.h>
#include "board.h"
#include "peripherals.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "fsl_debug_console.h"
// Assembly LED functions
void setup_leds(void);
void turn_on_rgb(void);
void turn_off_rgb(void);
// Button and buzzer functions
static void button_init(void);
static uint8_t button_pressed(void);
static void buzzer_init(void);
static void buzzer_on(void);
static void buzzer_off(void);
// Simple demo tracking
volatile uint32_t seconds = 0, minutes = 0, hours = 0;
volatile uint32_t current_minute = 0;
volatile bool tick = false, alarm_active = false;
volatile uint32_t alarm_start_time = 0;

#define BUZZER_PIN 2
#define BUZZER_PORT GPIOC

void PIT_CHANNEL_0_IRQHANDLER(void)
{
    PIT_ClearStatusFlags(PIT, PIT_CHANNEL_0, kPIT_TimerFlag);

    seconds++;
    if (seconds >= 60) {
        seconds = 0;
        minutes++;

        //Trigger alarm every new minute
        if (minutes != current_minute) {
            current_minute = minutes;
            if (!alarm_active) {
                alarm_start_time = (hours * 3600) + (minutes * 60) + seconds;
                PRINTF("ALARM_START,Minute %d,%02d:%02d:%02d\r\n", current_minute, hours, minutes, seconds);

                turn_on_rgb();
                buzzer_on();
                alarm_active = true;
            }
        }
    }
    if (minutes >= 60) {
        minutes = 0;
        hours++;
    }
    if (hours >= 24) {
        hours = 0;
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

static void buzzer_init(void)
{
    SIM->SCGC5 |= SIM_SCGC5_PORTC_MASK;
    PORTC->PCR[BUZZER_PIN] = PORT_PCR_MUX(1);
    GPIOC->PDDR |= (1U << BUZZER_PIN);
    GPIOC->PDOR &= ~(1U << BUZZER_PIN);
}

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

    // Start at Minute 0
    current_minute = 0;
    hours = 0; minutes = 0; seconds = 0;

    // CSV Heading for the logger
    PRINTF("Event,Minute,Time,Duration_Seconds\r\n");
    PRINTF("SYSTEM_START,Minute %d,%02d:%02d:%02d,0\r\n", current_minute, hours, minutes, seconds);

    PRINTF("\n=== DEMO: MINUTE MEDICATION REMINDER ===\r\n");
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

                    // Log alarm stopped and medication taken
                    PRINTF("MEDICATION_TAKEN,Minute %d,%02d:%02d:%02d,%d\r\n",
                           current_minute, hours, minutes, seconds, alarm_duration);

                    turn_off_rgb();
                    buzzer_off();
                    alarm_active = false;

                    // Simple user feedback
                    PRINTF(">>> Medication logged for Minute %d! Next alarm: Minute %d\r\n\n",
                           current_minute, current_minute + 1);
                }
            }

            tick = false;
        }
        __asm("nop");
    }
    return 0;
}
