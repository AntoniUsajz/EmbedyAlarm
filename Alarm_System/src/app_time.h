#ifndef APP_TIME_H
#define APP_TIME_H

#include <stdint.h>

void init_time(void);
void get_time_string(char* buffer);

uint8_t time_get_hour(void);
uint8_t time_get_minute(void);

void time_inc_hour(void);
void time_dec_hour(void);
void time_inc_minute(void);
void time_dec_minute(void);

#endif /* APP_TIME_H */
