#ifndef APP_MENU_H
#define APP_MENU_H

#include <stdint.h>

void menu_init(void);
void menu_process(uint8_t joy_state);
void menu_update_display(void);

#endif /* APP_MENU_H */
