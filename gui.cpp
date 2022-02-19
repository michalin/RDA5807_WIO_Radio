/*  RDA5807 based FM receiver LVGL GUI functions
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

#include "radio.h"

#define GUI_L1 10
#define GUI_L2 GUI_L1 + 100
#define GUI_L3 GUI_L2 + 25
#define GUI_C1 10
#define GUI_C2 GUI_C1 + 40

//LV_FONT_DECLARE(lcd_100)     // Font for frequency display
LV_FONT_DECLARE(lcd_100bold) // Font for frequency display
//#define DEBUG
char dbgtext[32];
// uint16_t frequency = 10000;
lv_obj_t *spb_frq, *rssi_bar, *sli_volume, *rssi_val, *stereo_stat, *rds_stat, *tar_rdstext;

void spb_frequency_event_cb(lv_obj_t *sb, lv_event_t event)
{
    static uint8_t crsrpos;
    // Serial.println(event);
    if (event == LV_EVENT_KEY)
    {
        uint8_t key = lv_indev_get_key(wio_key_dev);
        if (key == LV_KEY_DOWN)
        {
            rda_set_next_frequency(lv_spinbox_get_value(sb));
        }
        else if (key == LV_KEY_UP)
        {
            rda_set_next_frequency(lv_spinbox_get_value(sb));
        }
        else if(key == LV_KEY_ENTER)
        {
            crsrpos++;
            if(crsrpos == 6)
                crsrpos = 0;
            else if(crsrpos == 3)
                crsrpos = 4; //Skip dec. point
            //Serial.println(crsrpos);
            lv_textarea_set_cursor_pos(sb, crsrpos);
        }
    } 
    else if (event == LV_EVENT_DEFOCUSED)
    {
        lv_textarea_set_cursor_hidden(spb_frq, true);
        if (lv_spinbox_get_value(sb) < 10000)
            lv_spinbox_set_digit_format(spb_frq, 4, 2);
    }
    else if (event == LV_EVENT_FOCUSED)
    {
        lv_spinbox_set_range(spb_frq, 5000,11500);
        lv_spinbox_set_digit_format(spb_frq, 5, 3);
        lv_textarea_set_cursor_hidden(spb_frq, false);
        crsrpos = 5;
        lv_textarea_set_cursor_pos(sb, crsrpos);
    }
    
}

void btn_prev_event_cb(lv_obj_t *btn, lv_event_t event)
{
    if (event == LV_EVENT_KEY)
    {
        rda_seek(0);
    }
}

void btn_next_event_cb(lv_obj_t *next, lv_event_t event)
{
    if (event == LV_EVENT_KEY)
    {
        rda_seek(1);
    }
}

void sli_volume_event_cb(lv_obj_t *vol, lv_event_t event)
{
    if (event == LV_EVENT_VALUE_CHANGED)
    {
        uint8_t key = lv_indev_get_key(wio_key_dev);
        p_Config->volume = lv_slider_get_value(vol);
        rda_set_volume();
    }
}

void btn_settings_event_cb(lv_obj_t *but, lv_event_t event)
{
    if (event == LV_BTN_STATE_RELEASED)
    {
        lv_obj_t *screen = lv_obj_create(NULL, NULL);
        menu_init(screen);
        lv_scr_load(screen);
    }
}

void gui_init(lv_obj_t *screen)
{
    lv_group_t *group = lv_group_create();
    lv_group_set_wrap(group, true);

    // Initialize styles
    lv_obj_set_style_local_bg_color(screen, LV_PAGE_PART_BG, LV_STATE_DEFAULT, BGCOLOR);
    lv_style_set_text_color(&txt_style, LV_STATE_DEFAULT, TEXTCOLOR);
    lv_style_set_bg_color(&btnstyle, LV_STATE_DEFAULT, BTNCOLOR);
    lv_style_set_border_color(&btnstyle, LV_STATE_DEFAULT, BTNCOLOR);
    lv_style_set_shadow_width(&btnstyle, LV_STATE_DEFAULT, 5);
    lv_style_set_shadow_ofs_x(&btnstyle, LV_STATE_DEFAULT, 3);
    lv_style_set_shadow_ofs_y(&btnstyle, LV_STATE_DEFAULT, 3);
    lv_style_set_value_color(&btnstyle, LV_STATE_DEFAULT, SYMBCOLOR1);
    lv_style_set_value_color(&btnstyle, LV_STATE_FOCUSED, SYMBCOLOR2);
    // Frequency selection
    // Scan down button
    lv_obj_t *btn_prev = lv_btn_create(screen, NULL);
    lv_theme_apply(btn_prev, LV_THEME_SPINBOX_BTN);
    lv_obj_add_style(btn_prev, LV_BTN_PART_MAIN, &btnstyle);
    lv_obj_set_style_local_value_str(btn_prev, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_SYMBOL_PREV);
    lv_obj_set_event_cb(btn_prev, btn_prev_event_cb);

    spb_frq = lv_spinbox_create(screen, NULL);
    lv_obj_set_style_local_text_font(spb_frq, LV_TEXTAREA_PART_BG, LV_STATE_DEFAULT, &lcd_100bold);
    lv_obj_set_style_local_bg_color(spb_frq, LV_TEXTAREA_PART_BG, LV_STATE_DEFAULT, LV_COLOR_BLACK);
    lv_obj_set_style_local_text_color(spb_frq, LV_TEXTAREA_PART_BG, LV_STATE_DEFAULT, FRQCOLOR);
    lv_obj_set_style_local_border_color(spb_frq, LV_TEXTAREA_PART_BG, LV_STATE_DEFAULT, BTNCOLOR);
    lv_obj_set_style_local_shadow_width(spb_frq, LV_STATE_DEFAULT, LV_STATE_DEFAULT, 5);
    lv_obj_set_style_local_shadow_ofs_x(spb_frq, LV_STATE_DEFAULT, LV_STATE_DEFAULT, 3);
    lv_obj_set_style_local_shadow_ofs_y(spb_frq, LV_STATE_DEFAULT, LV_STATE_DEFAULT, 3);
    lv_textarea_set_cursor_hidden(spb_frq, true);
    lv_textarea_set_text_align(spb_frq, LV_LABEL_ALIGN_CENTER);
    lv_spinbox_set_step(spb_frq, 1);
    lv_obj_set_width(spb_frq, 250);
    lv_obj_set_event_cb(spb_frq, spb_frequency_event_cb);
    gui_show_frequency(p_Config->frequency);

    // Scan up
    lv_obj_t *btn_next = lv_btn_create(screen, NULL);
    lv_theme_apply(btn_next, LV_THEME_SPINBOX_BTN);
    lv_obj_set_style_local_value_str(btn_next, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_SYMBOL_NEXT);
    lv_obj_add_style(btn_next, LV_BTN_PART_MAIN, &btnstyle);
    lv_obj_set_event_cb(btn_next, btn_next_event_cb);

    lv_coord_t h = lv_obj_get_height(spb_frq);
    lv_obj_set_size(btn_prev, 25, h);
    lv_obj_set_size(btn_next, 25, h);
    lv_obj_align(btn_prev, NULL, LV_ALIGN_IN_TOP_LEFT, GUI_C1, GUI_L1);
    lv_obj_align(spb_frq, btn_prev, LV_ALIGN_OUT_RIGHT_TOP, 0, 0);
    lv_obj_align(btn_next, spb_frq, LV_ALIGN_OUT_RIGHT_TOP, 0, 0);

    rssi_bar = lv_bar_create(screen, NULL);
    lv_obj_set_size(rssi_bar, lv_obj_get_width(spb_frq) - 10, 5);
    lv_bar_set_anim_time(rssi_bar, 1000);
    lv_bar_set_range(rssi_bar, 10, 70);
    lv_bar_set_value(rssi_bar, 60, false);
    lv_obj_set_style_local_bg_color(rssi_bar, LV_BAR_PART_BG, LV_STATE_DEFAULT, BGCOLOR2);
    // lv_obj_set_style_local_bg_opa(rssi_bar, LV_BAR_PART_BG, LV_STATE_DEFAULT, LV_OPA_0);
    lv_obj_set_style_local_bg_grad_dir(rssi_bar, LV_BAR_PART_INDIC, LV_STATE_DEFAULT, LV_GRAD_DIR_HOR);
    lv_obj_set_style_local_bg_color(rssi_bar, LV_BAR_PART_INDIC, LV_STATE_DEFAULT, LV_COLOR_RED);
    lv_obj_set_style_local_bg_grad_color(rssi_bar, LV_BAR_PART_INDIC, LV_STATE_DEFAULT, LV_COLOR_LIME);
    lv_obj_set_style_local_bg_blend_mode(rssi_bar, LV_BAR_PART_INDIC, LV_STATE_DEFAULT, LV_BLEND_MODE_ADDITIVE);
    lv_obj_set_style_local_bg_main_stop(rssi_bar, LV_BAR_PART_INDIC, LV_STATE_DEFAULT, 64);
    lv_obj_set_style_local_bg_grad_stop(rssi_bar, LV_BAR_PART_INDIC, LV_STATE_DEFAULT, 192);
    lv_obj_align(rssi_bar, spb_frq, LV_ALIGN_IN_BOTTOM_LEFT, 5, -1);

    // Status values
    stereo_stat = lv_label_create(screen, NULL);
    lv_obj_add_style(stereo_stat, LV_LABEL_PART_MAIN, &txt_style);
    lv_label_set_text(stereo_stat, "xxxxxx");
    lv_obj_align(stereo_stat, btn_prev, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 5);

    rssi_val = lv_label_create(screen, NULL);
    lv_obj_add_style(rssi_val, LV_LABEL_PART_MAIN, &txt_style);
    lv_label_set_text(stereo_stat, "00dBuV");
    lv_obj_align(rssi_val, spb_frq, LV_ALIGN_OUT_BOTTOM_RIGHT, 0, 5);

    // Volume slider
    lv_obj_t *vol_label = lv_label_create(screen, NULL);
    lv_obj_add_style(vol_label, LV_LABEL_PART_MAIN, &txt_style);
    lv_label_set_text(vol_label, "Vol");
    lv_obj_align(vol_label, NULL, LV_ALIGN_IN_TOP_LEFT, GUI_C1, GUI_L3);

    sli_volume = lv_slider_create(screen, NULL);
    lv_obj_set_size(sli_volume, 260, 10);
    lv_slider_set_range(sli_volume, 0, 15);
    lv_obj_set_event_cb(sli_volume, sli_volume_event_cb);
    lv_obj_set_style_local_bg_opa(sli_volume, LV_SLIDER_PART_KNOB, LV_STATE_DEFAULT, LV_OPA_0);
    lv_obj_set_style_local_bg_color(sli_volume, LV_SLIDER_PART_BG, LV_STATE_DEFAULT, BGCOLOR2);
    lv_obj_set_style_local_bg_color(sli_volume, LV_SLIDER_PART_INDIC, LV_STATE_DEFAULT, SLDCOLOR);
    lv_obj_align(sli_volume, NULL, LV_ALIGN_IN_TOP_LEFT, GUI_C2, GUI_L3);

    if (p_Config->rds_on)
    {
        tar_rdstext = lv_textarea_create(screen, NULL);
        lv_obj_set_height(tar_rdstext, 70);
        lv_obj_set_width(tar_rdstext, 280);
        lv_obj_set_style_local_text_color(tar_rdstext, LV_TEXTAREA_PART_BG, LV_STATE_DEFAULT, TEXTCOLOR2);
        lv_obj_set_style_local_bg_color(tar_rdstext, LV_TEXTAREA_PART_BG, LV_STATE_DEFAULT, BGCOLOR);
        lv_textarea_set_cursor_hidden(tar_rdstext, true);
        lv_textarea_set_sscrollbar_mode(tar_rdstext, LV_SCRLBAR_MODE_HIDE);
        lv_obj_set_style_local_border_opa(tar_rdstext, LV_TEXTAREA_PART_BG, LV_STATE_DEFAULT, LV_OPA_10);
        lv_obj_align(tar_rdstext, NULL, LV_ALIGN_IN_BOTTOM_LEFT, 0, 0);
    }

    // Settings button
    lv_obj_t *btn_set = lv_btn_create(screen, NULL);
    lv_obj_set_size(btn_set, 30, 30);
    lv_obj_align(btn_set, NULL, LV_ALIGN_IN_BOTTOM_RIGHT, -10, -10);
    lv_obj_add_style(btn_set, LV_BTN_PART_MAIN, &btnstyle);
    lv_obj_set_style_local_value_str(btn_set, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_SYMBOL_SETTINGS);
    lv_obj_set_event_cb(btn_set, btn_settings_event_cb);

    lv_group_add_obj(group, btn_set);
    lv_group_add_obj(group, btn_prev);
    lv_group_add_obj(group, spb_frq);
    lv_group_add_obj(group, btn_next);
    lv_group_add_obj(group, sli_volume);

    lv_indev_set_group(wio_key_dev, group);
    gui_show_frequency(p_Config->frequency);
    lv_slider_set_value(sli_volume, p_Config->volume, 2);
}

void gui_show_frequency(uint16_t freq)
{
    if (freq >= 10000)
    {
        lv_spinbox_set_digit_format(spb_frq, 5, 3);
        lv_spinbox_set_range(spb_frq, 5000, 11500);
    }
    else if(!lv_obj_is_focused(spb_frq))
        lv_spinbox_set_digit_format(spb_frq, 4, 2);

    lv_spinbox_set_value(spb_frq, freq);
    if (tar_rdstext)
        lv_textarea_set_text(tar_rdstext, "No RDS data"); // Clear RDS Data of last station
}

void gui_show_status(uint16_t rssi, bool is_stereo)
{
    lv_bar_set_value(rssi_bar, rssi, LV_ANIM_ON);
    char rssistr[16];
    sprintf(rssistr, "%ddBuV", rssi);
    lv_label_set_text(rssi_val, rssistr);
    p_Config->mono | !is_stereo ? lv_label_set_text(stereo_stat, "mono") : lv_label_set_text(stereo_stat, "stereo");
}

void gui_show_rdsinfo(const char *fmt, ...)
{
    va_list valist;
    va_start(valist, fmt);
    char txtbuf[128];
    vsprintf(txtbuf, fmt, valist);
    // lv_label_set_text(lab_rdstext, txtbuf);
    lv_textarea_set_text(tar_rdstext, txtbuf);
    lv_textarea_set_text(tar_rdstext, "Radio Doctor Volt FM\nQueen - Radio Ga Ga ");
    va_end(valist);
}
