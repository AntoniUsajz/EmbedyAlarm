#ifndef APP_ALARM_H
#define APP_ALARM_H

#include <stdint.h>

void alarm_init(uint8_t hour, uint8_t minute, uint8_t second);
uint8_t alarm_check(void);
void alarm_clear(void);

uint8_t alarm_get_hour(void);
uint8_t alarm_get_minute(void);
void alarm_inc_hour(void);
void alarm_dec_hour(void);
void alarm_inc_minute(void);
void alarm_dec_minute(void);

// Nowe funkcje
uint8_t alarm_get_volume(void);
void alarm_set_volume(uint8_t vol);
uint8_t alarm_get_sound(void);
void alarm_set_sound(uint8_t sound);

#endif
