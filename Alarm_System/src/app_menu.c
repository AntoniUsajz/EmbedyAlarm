#include "app_menu.h"
#include "joystick.h"
#include "oled.h"
#include "app_time.h"
#include "app_temp.h"
#include "app_alarm.h"
#include "lpc17xx_gpio.h"
#include "lpc17xx_pinsel.h"
#include "LPC17xx.h"
#include <stdio.h>

extern uint32_t getTicks(void);

typedef enum {
    STATE_MAIN,
    STATE_MENU,
    STATE_SET_TIME,
    STATE_ALARM_MENU,
    STATE_SET_ALARM_TIME,
    STATE_SET_VOLUME,
    STATE_SET_SOUND
} SystemState_t;

static SystemState_t current_state = STATE_MAIN;
static uint8_t menu_idx = 0;
static uint8_t sub_menu_idx = 0;
static uint8_t last_joy = 0;
static uint32_t last_clear_tick = 0;

static uint8_t blink_state = 0;
static uint32_t last_blink_tick = 0;

static uint8_t was_alarm_logic = 0;
static uint8_t was_alarm_disp = 0;

static uint8_t time_edit_focus = 0;
static uint8_t alarm_edit_focus = 0;

static uint32_t last_action_tick = 0;

void EINT0_IRQHandler(void) {
    LPC_SC->EXTINT |= (1 << 0);
}

void system_enter_deep_sleep(void) {
    oled_clearScreen(OLED_COLOR_WHITE);

    SCB->SCR |= (1 << 2);
    LPC_SC->PCON &= ~(0x03);

    __WFI();

    SCB->SCR &= ~(1 << 2);
    oled_clearScreen(OLED_COLOR_WHITE);

    current_state = STATE_MAIN;
}

void sleep_config_eint0(void) {
    PINSEL_CFG_Type PinCfg;

    PinCfg.Funcnum = 1;
    PinCfg.OpenDrain = 0;
    PinCfg.Pinmode = 0;
    PinCfg.Portnum = 2;
    PinCfg.Pinnum = 10;
    PINSEL_ConfigPin(&PinCfg);

    LPC_SC->EXTMODE |= (1 << 0);
    LPC_SC->EXTPOLAR &= ~(1 << 0);
    LPC_SC->EXTINT |= (1 << 0);

    NVIC_EnableIRQ(EINT0_IRQn);
}

void menu_init(void) {
    current_state = STATE_MAIN;
    was_alarm_logic = 0;
    was_alarm_disp = 0;
    last_action_tick = getTicks();

    GPIO_SetDir(0, 1<<24, 1);
    GPIO_ClearValue(0, 1<<24);

    sleep_config_eint0();

    oled_clearScreen(OLED_COLOR_WHITE);
}

void menu_process(uint8_t joy) {
    uint8_t is_alarm = alarm_check();

    uint8_t edge = joy & ~last_joy;
    last_joy = joy;

    if (edge != 0 || is_alarm) {
        last_action_tick = getTicks();
    }

    if (!is_alarm && (getTicks() - last_action_tick > 15000)) {
        system_enter_deep_sleep();
        last_action_tick = getTicks();
        return;
    }

    if (!is_alarm && was_alarm_logic) {
        last_clear_tick = getTicks();
    }
    was_alarm_logic = is_alarm;

    if (is_alarm) {
        if (edge & JOYSTICK_CENTER) {
            alarm_clear();
            last_clear_tick = getTicks();
        }
        return;
    }

    if (getTicks() - last_clear_tick < 1000) {
        return;
    }

    switch (current_state) {
        case STATE_MAIN:
            if (edge & JOYSTICK_CENTER) {
                current_state = STATE_MENU;
                menu_idx = 0;
                oled_clearScreen(OLED_COLOR_WHITE);
            }
            break;

        case STATE_MENU:
            if (edge & JOYSTICK_DOWN) menu_idx = (menu_idx + 1) % 3;
            else if (edge & JOYSTICK_UP) menu_idx = (menu_idx == 0) ? 2 : menu_idx - 1;
            else if (edge & JOYSTICK_CENTER) {
                if (menu_idx == 0) {
                    current_state = STATE_SET_TIME;
                    time_edit_focus = 0;
                    oled_clearScreen(OLED_COLOR_WHITE);
                }
                else if (menu_idx == 1) {
                    current_state = STATE_ALARM_MENU;
                    sub_menu_idx = 0;
                    oled_clearScreen(OLED_COLOR_WHITE);
                }
                else if (menu_idx == 2) {
                    current_state = STATE_MAIN;
                    oled_clearScreen(OLED_COLOR_WHITE);
                }
            }
            break;

        case STATE_ALARM_MENU:
            if (edge & JOYSTICK_DOWN) sub_menu_idx = (sub_menu_idx + 1) % 4;
            else if (edge & JOYSTICK_UP) sub_menu_idx = (sub_menu_idx == 0) ? 3 : sub_menu_idx - 1;
            else if (edge & JOYSTICK_CENTER) {
                if (sub_menu_idx == 0) {
                    current_state = STATE_SET_ALARM_TIME;
                    alarm_edit_focus = 0;
                    oled_clearScreen(OLED_COLOR_WHITE);
                }
                else if (sub_menu_idx == 1) {
                    current_state = STATE_SET_VOLUME;
                    oled_clearScreen(OLED_COLOR_WHITE);
                }
                else if (sub_menu_idx == 2) {
                    current_state = STATE_SET_SOUND;
                    oled_clearScreen(OLED_COLOR_WHITE);
                }
                else if (sub_menu_idx == 3) {
                    current_state = STATE_MENU;
                    oled_clearScreen(OLED_COLOR_WHITE);
                }
            }
            break;

        case STATE_SET_VOLUME:
            if (edge & JOYSTICK_UP) alarm_set_volume((alarm_get_volume() + 1) % 6);
            else if (edge & JOYSTICK_DOWN) alarm_set_volume((alarm_get_volume() == 0) ? 5 : alarm_get_volume() - 1);
            else if (edge & JOYSTICK_CENTER) {
                current_state = STATE_ALARM_MENU;
                oled_clearScreen(OLED_COLOR_WHITE);
            }
            break;

        case STATE_SET_SOUND:
            if (edge & JOYSTICK_UP) alarm_set_sound((alarm_get_sound() + 1) % 3);
            else if (edge & JOYSTICK_DOWN) alarm_set_sound((alarm_get_sound() == 0) ? 2 : alarm_get_sound() - 1);
            else if (edge & JOYSTICK_CENTER) {
                current_state = STATE_ALARM_MENU;
                oled_clearScreen(OLED_COLOR_WHITE);
            }
            break;

        case STATE_SET_TIME:
            if (edge & JOYSTICK_LEFT) time_edit_focus = 0;
            else if (edge & JOYSTICK_RIGHT) time_edit_focus = 1;
            else if (edge & JOYSTICK_UP) {
                if (time_edit_focus == 0) time_inc_hour();
                else time_inc_minute();
            }
            else if (edge & JOYSTICK_DOWN) {
                if (time_edit_focus == 0) time_dec_hour();
                else time_dec_minute();
            }
            else if (edge & JOYSTICK_CENTER) {
                current_state = STATE_MENU;
                oled_clearScreen(OLED_COLOR_WHITE);
            }
            break;

        case STATE_SET_ALARM_TIME:
            if (edge & JOYSTICK_LEFT) alarm_edit_focus = 0;
            else if (edge & JOYSTICK_RIGHT) alarm_edit_focus = 1;
            else if (edge & JOYSTICK_UP) {
                if (alarm_edit_focus == 0) alarm_inc_hour();
                else alarm_inc_minute();
            }
            else if (edge & JOYSTICK_DOWN) {
                if (alarm_edit_focus == 0) alarm_dec_hour();
                else alarm_dec_minute();
            }
            else if (edge & JOYSTICK_CENTER) {
                current_state = STATE_ALARM_MENU;
                oled_clearScreen(OLED_COLOR_WHITE);
            }
            break;
    }
}

void menu_update_display(void) {
    char buf[20];
    uint8_t is_alarm = alarm_check();
    uint32_t current_tick = getTicks();

    if (is_alarm && !was_alarm_disp) {
        oled_clearScreen(OLED_COLOR_BLACK);
        last_blink_tick = current_tick;
    } else if (!is_alarm && was_alarm_disp) {
        oled_clearScreen(OLED_COLOR_WHITE);
        GPIO_ClearValue(0, 1<<24);
    }
    was_alarm_disp = is_alarm;

    if (is_alarm) {
        if (current_tick - last_blink_tick > 100) {
            blink_state = !blink_state;
            last_blink_tick = current_tick;

            if (blink_state) {
                GPIO_SetValue(0, 1<<24);
            } else {
                GPIO_ClearValue(0, 1<<24);
            }
        }

        if (blink_state) {
             oled_putString(15, 25, (uint8_t*)" !!! ALARM !!! ", OLED_COLOR_WHITE, OLED_COLOR_BLACK);
        } else {
             oled_putString(15, 25, (uint8_t*)" !!! ALARM !!! ", OLED_COLOR_BLACK, OLED_COLOR_BLACK);
        }
        return;
    }

    if (current_tick - last_blink_tick > 300) {
         blink_state = !blink_state;
         last_blink_tick = current_tick;
    }

    if (current_state == STATE_MAIN) {
        oled_putString(1, 1,  (uint8_t*)"Temp : ", OLED_COLOR_BLACK, OLED_COLOR_WHITE);
        get_temp_string(buf);
        oled_fillRect(40, 1, 80, 9, OLED_COLOR_WHITE);
        oled_putString(40, 1, (uint8_t*)buf, OLED_COLOR_BLACK, OLED_COLOR_WHITE);

        oled_putString(1, 10, (uint8_t*)"Czas : ", OLED_COLOR_BLACK, OLED_COLOR_WHITE);
        get_time_string(buf);
        oled_fillRect(40, 10, 80, 18, OLED_COLOR_WHITE);
        oled_putString(40, 10, (uint8_t*)buf, OLED_COLOR_BLACK, OLED_COLOR_WHITE);
    }
    else if (current_state == STATE_MENU) {
        oled_putString(1, 1, (uint8_t*)"--- MENU ---", OLED_COLOR_BLACK, OLED_COLOR_WHITE);

        oled_putString(1, 15, (uint8_t*)"> Ustaw czas ", (menu_idx==0 && blink_state) ? OLED_COLOR_WHITE : OLED_COLOR_BLACK, OLED_COLOR_WHITE);
        oled_putString(1, 25, (uint8_t*)"> Ustaw alarm", (menu_idx==1 && blink_state) ? OLED_COLOR_WHITE : OLED_COLOR_BLACK, OLED_COLOR_WHITE);
        oled_putString(1, 35, (uint8_t*)"> Powrot     ", (menu_idx==2 && blink_state) ? OLED_COLOR_WHITE : OLED_COLOR_BLACK, OLED_COLOR_WHITE);
    }
    else if (current_state == STATE_ALARM_MENU) {
        oled_putString(1, 1, (uint8_t*)"-- ALARM CFG --", OLED_COLOR_BLACK, OLED_COLOR_WHITE);
        oled_putString(1, 15, (uint8_t*)"> Czas alarmu", (sub_menu_idx==0 && blink_state) ? OLED_COLOR_WHITE : OLED_COLOR_BLACK, OLED_COLOR_WHITE);
        oled_putString(1, 25, (uint8_t*)"> Glosnosc   ", (sub_menu_idx==1 && blink_state) ? OLED_COLOR_WHITE : OLED_COLOR_BLACK, OLED_COLOR_WHITE);
        oled_putString(1, 35, (uint8_t*)"> Dzwiek     ", (sub_menu_idx==2 && blink_state) ? OLED_COLOR_WHITE : OLED_COLOR_BLACK, OLED_COLOR_WHITE);
        oled_putString(1, 45, (uint8_t*)"> Powrot     ", (sub_menu_idx==3 && blink_state) ? OLED_COLOR_WHITE : OLED_COLOR_BLACK, OLED_COLOR_WHITE);
    }
    else if (current_state == STATE_SET_VOLUME) {
        oled_putString(1, 1, (uint8_t*)"GLOSNOSC [0-5]", OLED_COLOR_BLACK, OLED_COLOR_WHITE);
        sprintf(buf, "Poziom: %d", alarm_get_volume());
        oled_putString(10, 25, (uint8_t*)buf, OLED_COLOR_BLACK, OLED_COLOR_WHITE);
        oled_putString(1, 50, (uint8_t*)"[^/v]-Zmien", OLED_COLOR_BLACK, OLED_COLOR_WHITE);
    }
    else if (current_state == STATE_SET_SOUND) {
        const char* names[] = {"Oda do radosci", "Marsz Imper.", "Gwizdanie"};
        oled_putString(1, 1, (uint8_t*)"WYBIERZ DZWIEK", OLED_COLOR_BLACK, OLED_COLOR_WHITE);
        oled_putString(10, 25, (uint8_t*)names[alarm_get_sound()], OLED_COLOR_BLACK, OLED_COLOR_WHITE);
        oled_putString(1, 50, (uint8_t*)"[^/v]-Zmien", OLED_COLOR_BLACK, OLED_COLOR_WHITE);
    }
    else if (current_state == STATE_SET_TIME || current_state == STATE_SET_ALARM_TIME) {
        if (current_state == STATE_SET_TIME) {
            oled_putString(1, 1, (uint8_t*)"USTAW CZAS", OLED_COLOR_BLACK, OLED_COLOR_WHITE);
            sprintf(buf, "%02d", time_get_hour());
        } else {
            oled_putString(1, 1, (uint8_t*)"USTAW ALARM", OLED_COLOR_BLACK, OLED_COLOR_WHITE);
            sprintf(buf, "%02d", alarm_get_hour());
        }

        uint8_t focus = (current_state == STATE_SET_TIME) ? time_edit_focus : alarm_edit_focus;

        if (focus == 0 && blink_state) {
            oled_putString(30, 20, (uint8_t*)"  ", OLED_COLOR_WHITE, OLED_COLOR_WHITE);
        } else {
            oled_putString(30, 20, (uint8_t*)buf, OLED_COLOR_BLACK, OLED_COLOR_WHITE);
        }

        oled_putString(46, 20, (uint8_t*)":", OLED_COLOR_BLACK, OLED_COLOR_WHITE);

        if (current_state == STATE_SET_TIME) {
            sprintf(buf, "%02d", time_get_minute());
        } else {
            sprintf(buf, "%02d", alarm_get_minute());
        }

        if (focus == 1 && blink_state) {
            oled_putString(54, 20, (uint8_t*)"  ", OLED_COLOR_WHITE, OLED_COLOR_WHITE);
        } else {
            oled_putString(54, 20, (uint8_t*)buf, OLED_COLOR_BLACK, OLED_COLOR_WHITE);
        }

        oled_putString(1, 40, (uint8_t*)"[L/P]-Pole", OLED_COLOR_BLACK, OLED_COLOR_WHITE);
        oled_putString(1, 50, (uint8_t*)"[^/v]-Wartosc", OLED_COLOR_BLACK, OLED_COLOR_WHITE);
    }
}
