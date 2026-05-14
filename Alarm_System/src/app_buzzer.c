#include "app_buzzer.h"
#include "lpc17xx_gpio.h"
#include "lpc17xx_pinsel.h"
#include "joystick.h"
#include "app_alarm.h"
#include "app_menu.h"

#define NOTE_PIN_HIGH() GPIO_SetValue(0, 1<<26)
#define NOTE_PIN_LOW()  GPIO_ClearValue(0, 1<<26)

#define VOL_CLK_PIN   (1<<27) // P0.27
#define VOL_UP_DN_PIN (1<<28) // P0.28

static uint32_t getNote(uint8_t ch) {
    switch(ch) {
        // Nuty diatoniczne (Niska oktawa)
        case 'C': return 3816; // C4
        case 'D': return 3401; // D4
        case 'E': return 3030; // E4
        case 'F': return 2865; // F4
        case 'G': return 2551; // G4
        case 'A': return 2272; // A4
        case 'B': return 2024; // B4

        // Półtony (Niskie)
        case 'H': return 3214; // Eb4
        case 'I': return 2145; // Bb4
        case 'J': return 2703; // Gb4

        // Nuty diatoniczne (Wysoka oktawa)
        case 'c': return 1912; // C5
        case 'd': return 1703; // D5
        case 'e': return 1517; // E5
        case 'f': return 1432; // F5
        case 'g': return 1275; // G5
        case 'a': return 1136; // A5
        case 'b': return 1012; // B5

        // Półtony (Wysokie)
        case 'h': return 1607; // Eb5
        case 'i': return 1073; // Bb5

        // Pauza
        case 'R': return 0;
    }
    return 0;
}

static const char *songs[] = {
    // 0: Oda do radości
    "e2,e2,f2,g2,g2,f2,e2,d2,c2,c2,d2,e2,e3.d1+d4_",

    // 1: Marsz Imperialny - poprawne proporcje i nuty
    "G3.G3.G3.H2+I1.G3.H2+I1.G6_d3.d3.d3.h2+I1.J3.H2+I1.G6_",

    // 2: Gwizdanie
    "e2,g2,e4_c2,d2,g4_e2,g2,e4_c2,d2,g4_"
};

static void delay_us_soft(uint32_t us) {
    for(volatile uint32_t i = 0; i < (us * 25); i++);
}

static void delay_ms_soft(uint32_t ms) {
    for(volatile uint32_t i = 0; i < (ms * 25000); i++);
}

void buzzer_set_volume(uint8_t vol) {
    GPIO_ClearValue(0, VOL_UP_DN_PIN);
    delay_us_soft(10);

    for (int i = 0; i < 16; i++) {
        GPIO_SetValue(0, VOL_CLK_PIN);
        delay_us_soft(100);
        GPIO_ClearValue(0, VOL_CLK_PIN);
        delay_us_soft(100);
    }

    if (vol > 0) {
        uint8_t steps = (vol == 5) ? 16 : (vol * 3);
        GPIO_SetValue(0, VOL_UP_DN_PIN);
        delay_us_soft(10);

        for (int i = 0; i < steps; i++) {
            GPIO_SetValue(0, VOL_CLK_PIN);
            delay_us_soft(100);
            GPIO_ClearValue(0, VOL_CLK_PIN);
            delay_us_soft(100);
        }
    }
}

static uint8_t playNote(uint32_t note, uint32_t durationMs) {
    uint32_t t = 0;
    if (note > 0) {
        while (t < (durationMs * 1000)) {
            if ((joystick_read() & JOYSTICK_CENTER) != 0) return 1;

            NOTE_PIN_HIGH();
            delay_us_soft(note / 2);
            NOTE_PIN_LOW();
            delay_us_soft(note / 2);
            t += note;
        }
    } else {
        uint32_t elapsed = 0;
        while (elapsed < durationMs) {
            if ((joystick_read() & JOYSTICK_CENTER) != 0) return 1;
            delay_ms_soft(1);
            elapsed++;
        }
    }
    return 0;
}

void buzzer_init(void) {
    // Inicjalizacja pinów dla LM4811
    GPIO_SetDir(2, 1<<13, 1);
    GPIO_ClearValue(2, 1<<13);

    GPIO_SetDir(0, VOL_CLK_PIN, 1);
    GPIO_SetDir(0, VOL_UP_DN_PIN, 1);

    // Inicjalizacja pinu dźwięku jako standardowe GPIO (P0.26)
    PINSEL_CFG_Type PinCfg;
    PinCfg.Funcnum = 0;
    PinCfg.OpenDrain = 0;
    PinCfg.Pinmode = 0;
    PinCfg.Portnum = 0;
    PinCfg.Pinnum = 26;
    PINSEL_ConfigPin(&PinCfg);

    GPIO_SetDir(0, 1<<26, 1);
    GPIO_ClearValue(0, 1<<26);

    buzzer_set_volume(alarm_get_volume());
}

void buzzer_play_alarm(void) {
    buzzer_set_volume(alarm_get_volume());
    uint8_t *song = (uint8_t*)songs[alarm_get_sound()];

    while(*song != '\0') {
        menu_update_display();

        uint32_t note = getNote(*song++);
        if (note == 0 && *(song-1) != 'R') break;

        uint32_t dur  = (*song++ - '0') * 100;
        uint8_t p_char = *song++;

        uint32_t pause = 5;
        if (p_char == '+') pause = 0;
        else if (p_char == '.') pause = 20;
        else if (p_char == '_') pause = 40;

        if (playNote(note, dur) == 1) { alarm_clear(); break; }

        uint32_t p_start = 0;
        while(p_start++ < pause) {
            delay_ms_soft(1);
            if ((joystick_read() & JOYSTICK_CENTER) != 0) { alarm_clear(); return; }
        }
    }
}
