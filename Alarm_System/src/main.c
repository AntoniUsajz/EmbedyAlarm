#include "lpc17xx_pinsel.h"
#include "lpc17xx_ssp.h"
#include "lpc17xx_i2c.h"
#include "lpc17xx_timer.h"

#include "oled.h"
#include "app_time.h"
#include "app_temp.h"
#include "app_alarm.h"
#include "app_buzzer.h"
#include "joystick.h"
#include "app_menu.h"

static uint32_t msTicks = 0;

void SysTick_Handler(void) {
    msTicks++;
}

 uint32_t getTicks(void) {
    return msTicks;
}

// Konfiguracja pinów i magistrali I2C (wymagane m.in. dla czujnika temperatury)
static void init_i2c(void) {
    PINSEL_CFG_Type PinCfg;
    PinCfg.Funcnum = 2;
    PinCfg.Pinmode = 0;
    PinCfg.OpenDrain = 0;
    PinCfg.Portnum = 0;

    PinCfg.Pinnum = 10; // SDA2
    PINSEL_ConfigPin(&PinCfg);

    PinCfg.Pinnum = 11; // SCL2
    PINSEL_ConfigPin(&PinCfg);

    I2C_Init(LPC_I2C2, 100000);
    I2C_Cmd(LPC_I2C2, ENABLE);
}

static void init_ssp(void) {
    SSP_CFG_Type SSP_ConfigStruct;
    PINSEL_CFG_Type PinCfg;

    PinCfg.Funcnum = 2;
    PinCfg.OpenDrain = 0;
    PinCfg.Pinmode = 0;
    PinCfg.Portnum = 0;

    PinCfg.Pinnum = 7; // SCK
    PINSEL_ConfigPin(&PinCfg);
    PinCfg.Pinnum = 8; // MISO
    PINSEL_ConfigPin(&PinCfg);
    PinCfg.Pinnum = 9; // MOSI
    PINSEL_ConfigPin(&PinCfg);

    PinCfg.Funcnum = 0;
    PinCfg.Portnum = 2;
    PinCfg.Pinnum = 2; // SSEL
    PINSEL_ConfigPin(&PinCfg);

    SSP_ConfigStructInit(&SSP_ConfigStruct);
    SSP_Init(LPC_SSP1, &SSP_ConfigStruct);
    SSP_Cmd(LPC_SSP1, ENABLE);
}

int main(void) {
    // 1. Inicjalizacja magistrali sprzętowych
    init_i2c();
    init_ssp();

    // 2. Inicjalizacja układów peryferyjnych
    oled_init();
    joystick_init();
    buzzer_init();

    // 3. Inicjalizacja modułów aplikacji
    init_app_temp(&getTicks);
    init_time();


    // 4. Inicjalizacja interfejsu (OLED)
    menu_init();

    if (SysTick_Config(SystemCoreClock / 1000)) {
        while (1);
    }

    while(1) {
        // Odczyt wejścia i przetworzenie przez maszynę stanów
        uint8_t joy = joystick_read();
        menu_process(joy);

        // Odświeżenie zawartości ekranu
        menu_update_display();

        // Niezależna obsługa głośnika (uruchamiana tylko, gdy alarm jest aktywny)
        if (alarm_check()) {
            buzzer_play_alarm();
        }

        // Taktowanie pętli UI
        Timer0_Wait(100);
    }

    return 0;
}

void check_failed(uint8_t *file, uint32_t line) {
    while(1);
}
