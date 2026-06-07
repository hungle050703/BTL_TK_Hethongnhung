/*
 * display.h
 *
 *  Created on: Oct 24, 2025
 *      Author: son
 */

#ifndef APP_DISPLAY_DISPLAY_APP_H_
#define APP_DISPLAY_DISPLAY_APP_H_

#include <stdint.h>


void display_app_init(void);
void display_app_menu_draw_current_screen(void);
void display_app_menu_update_main_info(float voltage, const char* ip, const char* time);
void display_app_menu_action_start_mode_a(void);
void display_app_menu_action_start_mode_b(void);
void display_app_menu_action_show_logs(void);
void display_app_menu_action_show_settings(void);
#endif /* APP_DISPLAY_DISPLAY_APP_H_ */
