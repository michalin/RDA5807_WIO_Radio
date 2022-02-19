/*  RDA5807 based FM receiver config menu
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

#define MARGIN_X 10
#define MARGIN_Y 10

lv_obj_t *rds;

void dd_band_event_cb(lv_obj_t *obj, lv_event_t event)
{
  if (event == LV_EVENT_VALUE_CHANGED)
  {
    p_Config->band = lv_dropdown_get_selected(obj);
    p_Config->frequency = 0;
    rda_set_config();
  }
}

void dd_space_event_cb(lv_obj_t *obj, lv_event_t event)
{
  if (event == LV_EVENT_VALUE_CHANGED)
  {
    p_Config->space = lv_dropdown_get_selected(obj);
    rda_set_config();
  }
}

void cb_mono_event_cb(lv_obj_t *obj, lv_event_t event)
{
  if (event == LV_EVENT_VALUE_CHANGED)
  {
    p_Config->mono = lv_checkbox_is_checked(obj);
    rda_set_config();
  }
}

void cb_bass_event_cb(lv_obj_t *obj, lv_event_t event)
{
  if (event == LV_EVENT_VALUE_CHANGED)
  {
    p_Config->bass_boost = lv_checkbox_is_checked(obj);
    rda_set_config();
  }
}
void cb_rds_event_cb(lv_obj_t *obj, lv_event_t event)
{
  if (event == LV_EVENT_VALUE_CHANGED)
  {
    p_Config->rds_on = lv_checkbox_is_checked(obj);
    rda_set_config();
  }
}

void cb_usa_event_cb(lv_obj_t *obj, lv_event_t event)
{
  if (event == LV_EVENT_VALUE_CHANGED)
  {
    p_Config->usa = lv_checkbox_is_checked(obj);
    p_Config->usa ? lv_checkbox_set_text(rds, "RBDS") : lv_checkbox_set_text(rds, "RDS");
    rda_set_config();
  }
}

void btn_finish_event_cb(lv_obj_t *obj, lv_event_t event)
{
  if (event == LV_BTN_STATE_RELEASED)
  {
    lv_obj_t *screen = lv_obj_create(NULL, NULL);
    gui_init(screen);
    lv_scr_load(screen);
    save_settings();
  }
}

void menu_init(lv_obj_t *screen)
{
  lv_group_t *g = lv_group_create();
  lv_obj_t *lab_region = lv_label_create(screen, NULL);
  lv_label_set_text(lab_region, "Region");
  lv_obj_align(lab_region, screen, LV_ALIGN_IN_TOP_LEFT, MARGIN_X, MARGIN_Y);

  lv_obj_t *dd_band = lv_dropdown_create(screen, NULL);
  lv_obj_set_size(dd_band, 200, lv_obj_get_height(dd_band));
  lv_dropdown_set_options(dd_band, "87-108 MHz USA,EUR\n"
                                   "76-91 MHz Japan\n"
                                   "76-108 MHz World\n"
                                   "65-76 MHz Russia\n"
                                   "50-65 MHz Ham Radio");

  lv_obj_align(dd_band, lab_region, LV_ALIGN_OUT_RIGHT_MID, 5, 0);
  lv_obj_set_event_cb(dd_band, dd_band_event_cb);
  lv_dropdown_set_selected(dd_band, p_Config->band);

  lv_obj_t *cont = lv_cont_create(screen, NULL);
  lv_obj_set_auto_realign(cont, true);                         /*Auto realign when the size changes*/
  lv_obj_align(cont, NULL, LV_ALIGN_IN_BOTTOM_LEFT, MARGIN_X, -MARGIN_Y); /*This parametrs will be sued when realigned*/
  lv_cont_set_fit(cont, LV_FIT_TIGHT);
  lv_cont_set_layout(cont, LV_LAYOUT_COLUMN_LEFT);

  lv_obj_t *stereo = lv_checkbox_create(cont, NULL);
  lv_checkbox_set_text(stereo, "Mono");
  lv_obj_set_event_cb(stereo, cb_mono_event_cb);
  lv_checkbox_set_checked(stereo, p_Config->mono);

  lv_obj_t *bass = lv_checkbox_create(cont, NULL);
  lv_checkbox_set_text(bass, "Bass Boost");
  lv_obj_set_event_cb(bass, cb_bass_event_cb);
  lv_checkbox_set_checked(bass, p_Config->bass_boost);

  lv_obj_t *usa = lv_checkbox_create(cont, NULL);
  lv_checkbox_set_text(usa, "USA");
  lv_obj_set_event_cb(usa, cb_usa_event_cb);
  lv_checkbox_set_checked(usa, p_Config->usa);

  rds = lv_checkbox_create(cont, NULL);
  lv_checkbox_set_text(rds, "RDS");
  lv_obj_set_event_cb(rds, cb_rds_event_cb);
  lv_checkbox_set_checked(rds, p_Config->rds_on);

  lv_obj_t *dd_space = lv_dropdown_create(screen, NULL);
  lv_obj_set_size(dd_space, 100, lv_obj_get_height(dd_band));
  lv_dropdown_set_options(dd_space, "100 kHz\n"
                                   "200 kHz\n"
                                   "50 kHz\n"
                                   "25 kHz");

  lv_obj_align(dd_space, dd_band, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 5);
  lv_obj_set_event_cb(dd_space, dd_space_event_cb);
  lv_dropdown_set_selected(dd_space, p_Config->space);

  lv_obj_t *lab_space = lv_label_create(screen, NULL);
  lv_label_set_text(lab_space, "Space ");
  lv_obj_align(lab_space, dd_space, LV_ALIGN_OUT_LEFT_MID, 0, 0);

  lv_obj_t *btn_ok = lv_btn_create(screen, NULL);
  lv_obj_align(btn_ok, NULL, LV_ALIGN_IN_BOTTOM_RIGHT, -MARGIN_X, -MARGIN_Y);
  lv_obj_t *lab_btn = lv_label_create(btn_ok, NULL);
  lv_label_set_text(lab_btn, "Finish");
  lv_obj_set_event_cb(btn_ok, btn_finish_event_cb);
  lv_obj_set_style_local_border_color(btn_ok, LV_PAGE_PART_BG, LV_STATE_DEFAULT, LV_COLOR_SILVER);
  lv_obj_set_style_local_border_opa(btn_ok, LV_PAGE_PART_BG, LV_STATE_DEFAULT, LV_OPA_50);

  lv_group_add_obj(g, btn_ok);
  lv_group_add_obj(g, dd_band);
  lv_group_add_obj(g, dd_space);
  lv_group_add_obj(g, stereo);
  lv_group_add_obj(g, bass);
  lv_group_add_obj(g, usa);
  lv_group_add_obj(g, rds);

  lv_indev_set_group(wio_key_dev, g);
}
