#include "app_temp.h"
#include "temp.h"
#include <stdio.h>

void init_app_temp(uint32_t (*tick_func)(void))
{
    temp_init(tick_func);
}

void get_temp_string(char* buffer)
{
    int32_t t = temp_read();
    int tempC = t / 10;
    int tempFrac = t % 10;
    sprintf(buffer, "%d.%d C", tempC, tempFrac);
}
