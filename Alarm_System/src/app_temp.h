#ifndef APP_TEMP_H
#define APP_TEMP_H

#include "lpc_types.h"

void init_app_temp(uint32_t (*tick_func)(void));
void get_temp_string(char* buffer);

#endif /* APP_TEMP_H */
