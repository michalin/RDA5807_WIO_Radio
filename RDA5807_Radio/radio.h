/*  RDA5807 based FM receiver
    Copyright (C) 2022  Doctor Volt
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
#ifndef RADIO_H
#define RADIO_H

#include <arduino.h>
#include <lvgl.h> //lv_arduino V3.0.1

#define BGCOLOR     LV_COLOR_PURPLE //Window background
#define BGCOLOR2    LV_COLOR_GRAY   //Widget background
#define FRQCOLOR    LV_COLOR_CYAN   //Frequency indicator
#define TEXTCOLOR   LV_COLOR_YELLOW //Text color
#define TEXTCOLOR2   LV_COLOR_LIME //Text color
#define BTNCOLOR    LV_COLOR_GREEN  //Button color
#define SLDCOLOR    LV_COLOR_YELLOW   //Slider indicator color
//#define BORDERCOLOR LV_COLOR_BLUE
#define SYMBCOLOR1  LV_COLOR_WHITE  //Button symbol color default
#define SYMBCOLOR2  LV_COLOR_ORANGE //Button symbol color when focused

//Assign buttons
#define NAV_NEXT    WIO_5S_RIGHT
#define NAV_PREV    WIO_5S_LEFT
#define SET_UP      WIO_5S_UP
#define SET_DOWN    WIO_5S_DOWN
#define ENTER       WIO_5S_PRESS

//Screen resolution
#define SCR_RES_X 320
#define SCR_RES_Y 240

extern lv_indev_t *wio_key_dev; 
extern lv_style_t txt_style, btnstyle;

typedef struct
{
  uint8_t test = 0xab; //Indicates that data in flash memory is valid
  uint16_t frequency = 8700; // Initial frequency
  uint8_t volume = 3;     // Initial volume
  uint8_t band = 0;       // 0: 87-108, 1: 76-91, 2: 76-108, 3: 65-76 4: 50-65
  uint8_t space = 0;
  bool mono = 1;
  bool bass_boost = 1;
  bool rds_on = 0;
  bool usa = 0;
} ConfigData;

extern ConfigData *p_Config;

//Forward declarations gui.cpp
void gui_init(lv_obj_t *screen);
void gui_show_frequency(uint16_t freqency); //Actual frequency in MHz / 100
void gui_show_status(uint16_t rssi, bool is_stereo);
void gui_show_rdsinfo(const char* rdsinfo, ...);

//Forward declarations menu.cpp
void menu_init(lv_obj_t *screen);

//Forward declarations radio.ino
void save_settings();
void rda_get_frequency();
void rda_seek(bool up);
void rda_set_next_frequency(uint16_t freq);
void rda_set_volume();
void rda_set_config();

#endif