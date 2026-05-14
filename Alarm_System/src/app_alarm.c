#include "app_alarm.h"
#include "lpc17xx_rtc.h"

static uint8_t alrm_h = 12;
static uint8_t alrm_m = 0;
static uint8_t alrm_s = 5;
static uint8_t is_active = 0;

static uint8_t alrm_vol = 3;   // Domyślnie środek skali
static uint8_t alrm_snd = 0;   // Domyślnie Oda do Radości

void alarm_init(uint8_t hour, uint8_t minute, uint8_t second) {
    alrm_h = hour; alrm_m = minute; alrm_s = second;
    is_active = 0;
}

uint8_t alarm_check(void) {
    RTC_TIME_Type rtcTime;
    if (is_active) return 1;
    RTC_GetFullTime(LPC_RTC, &rtcTime);
    if (rtcTime.HOUR == alrm_h && rtcTime.MIN == alrm_m && rtcTime.SEC == alrm_s) is_active = 1;
    return is_active;
}

void alarm_clear(void) { is_active = 0; }
uint8_t alarm_get_hour(void) { return alrm_h; }
uint8_t alarm_get_minute(void) { return alrm_m; }
void alarm_inc_hour(void) { alrm_h = (alrm_h + 1) % 24; }
void alarm_dec_hour(void) { alrm_h = (alrm_h == 0) ? 23 : alrm_h - 1; }
void alarm_inc_minute(void) { alrm_m = (alrm_m + 1) % 60; }
void alarm_dec_minute(void) { alrm_m = (alrm_m == 0) ? 59 : alrm_m - 1; }

uint8_t alarm_get_volume(void) { return alrm_vol; }
void alarm_set_volume(uint8_t vol) { if(vol <= 5) alrm_vol = vol; }
uint8_t alarm_get_sound(void) { return alrm_snd; }
void alarm_set_sound(uint8_t sound) { if(sound <= 2) alrm_snd = sound; }
