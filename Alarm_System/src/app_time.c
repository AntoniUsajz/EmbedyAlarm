#include "app_time.h"
#include "lpc17xx_rtc.h"
#include <stdio.h>

void init_time(void)
{
    RTC_Init(LPC_RTC);
    RTC_ResetClockTickCounter(LPC_RTC);
    RTC_Cmd(LPC_RTC, ENABLE);

    RTC_TIME_Type currentTime;
    currentTime.HOUR = 12;
    currentTime.MIN = 0;
    currentTime.SEC = 0;
    RTC_SetFullTime(LPC_RTC, &currentTime);
}

void get_time_string(char* buffer)
{
    RTC_TIME_Type rtcTime;
    RTC_GetFullTime(LPC_RTC, &rtcTime);
    sprintf(buffer, "%02d:%02d:%02d", (int)rtcTime.HOUR, (int)rtcTime.MIN, (int)rtcTime.SEC);
}

uint8_t time_get_hour(void) {
    RTC_TIME_Type t;
    RTC_GetFullTime(LPC_RTC, &t);
    return t.HOUR;
}

uint8_t time_get_minute(void) {
    RTC_TIME_Type t;
    RTC_GetFullTime(LPC_RTC, &t);
    return t.MIN;
}

void time_inc_hour(void) {
    RTC_TIME_Type t;
    RTC_GetFullTime(LPC_RTC, &t);
    t.HOUR = (t.HOUR + 1) % 24;
    RTC_SetFullTime(LPC_RTC, &t);
}

void time_dec_hour(void) {
    RTC_TIME_Type t;
    RTC_GetFullTime(LPC_RTC, &t);
    t.HOUR = (t.HOUR == 0) ? 23 : t.HOUR - 1;
    RTC_SetFullTime(LPC_RTC, &t);
}

void time_inc_minute(void) {
    RTC_TIME_Type t;
    RTC_GetFullTime(LPC_RTC, &t);
    t.MIN = (t.MIN + 1) % 60;
    RTC_SetFullTime(LPC_RTC, &t);
}

void time_dec_minute(void) {
    RTC_TIME_Type t;
    RTC_GetFullTime(LPC_RTC, &t);
    t.MIN = (t.MIN == 0) ? 59 : t.MIN - 1;
    RTC_SetFullTime(LPC_RTC, &t);
}
