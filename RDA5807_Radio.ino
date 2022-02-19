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

#include <TFT_eSPI.h>
#include <sfud.h> //Seeed Arduino SFUD 2.0.1
//#include <RDA5807.h
#include "RDA5807-lib.h"
#include "radio.h"

#define LVGL_TICK_PERIOD 5

TFT_eSPI tft = TFT_eSPI(); /* TFT instance */
static lv_disp_buf_t disp_buf;
static lv_color_t buf[LV_HOR_RES_MAX * 10];

lv_style_t txt_style, btnstyle;
lv_indev_t *wio_key_dev;
ConfigData *p_Config = new ConfigData();
RDA5807 rda;

const sfud_flash *flash;

#if USE_LV_LOG != 0
/* Serial debugging */
void my_print(lv_log_level_t level, const char *file, uint32_t line, const char *dsc)
{

  Serial.printf("%s@%d->%s\r\n", file, line, dsc);
  delay(100);
}
#endif
/* Interrupt driven periodic handler */
static void lv_tick_handler(void)
{
  lv_tick_inc(LVGL_TICK_PERIOD);
}

/* Display flushing */
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
  uint16_t c;

  tft.startWrite();                                                                            /* Start new TFT transaction */
  tft.setAddrWindow(area->x1, area->y1, (area->x2 - area->x1 + 1), (area->y2 - area->y1 + 1)); /* set the working window */
  for (int y = area->y1; y <= area->y2; y++)
  {
    for (int x = area->x1; x <= area->x2; x++)
    {
      c = color_p->full;
      tft.writeColor(c, 1);
      color_p++;
    }
  }
  tft.endWrite();            /* terminate TFT transaction */
  lv_disp_flush_ready(disp); /* tell lvgl that flushing is done */
}

/* Callback for processing the keys */
bool keys_read(lv_indev_drv_t *drv, lv_indev_data_t *data)
{
  //delay(100); //Debounce
  uint8_t last_key = 0;
  if (!digitalRead(WIO_5S_UP))
    last_key = LV_KEY_UP;
  else if (!digitalRead(WIO_5S_DOWN))
    last_key = LV_KEY_DOWN;
  else if (!digitalRead(WIO_5S_LEFT))
    last_key = LV_KEY_PREV;
  else if (!digitalRead(WIO_5S_RIGHT))
    last_key = LV_KEY_NEXT;
  else if (!digitalRead(WIO_5S_PRESS))
    last_key = LV_KEY_ENTER;
  if (last_key)
    data->state = LV_INDEV_STATE_PR;
  else
    data->state = LV_INDEV_STATE_REL;

  data->key = last_key; /*Get the last pressed or released key*/

  return false; /*No buffering now so no more data read*/
}

// Task that updates GUI (rssi rds, etc.)
void update_gui_task(lv_task_t *task)
{ 
  gui_show_status(rda.getRssi(), rda.isStereo());
}

void update_rds_task(lv_task_t *task)
{ 
  if(rda.hasRdsInfo())
  {
    gui_show_rdsinfo("%s\n%s", rda.getRdsText0A(), rda.getRdsText());
  }
}

void setup()
{
  Serial.begin(115200);
  lv_init();
#if USE_LV_LOG != 0
  lv_log_register_print(my_print); /* register print function for debugging */
#endif
  tft.init();         // TFT init
  tft.setRotation(3); // Landscape orientation

  lv_disp_buf_init(&disp_buf, buf, NULL, LV_HOR_RES_MAX * 10);
  /*Initialize the display*/
  lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);
  disp_drv.hor_res = SCR_RES_X;
  disp_drv.ver_res = SCR_RES_Y;
  disp_drv.flush_cb = my_disp_flush;
  disp_drv.buffer = &disp_buf;
  lv_disp_drv_register(&disp_drv);

  /*Initialize the keys*/
  pinMode(NAV_NEXT, INPUT_PULLUP);  //Next item
  pinMode(NAV_PREV, INPUT_PULLUP);  //Previous item
  pinMode(SET_UP, INPUT_PULLUP);    //Value up
  pinMode(SET_DOWN, INPUT_PULLUP);  //Value down
  pinMode(ENTER, INPUT_PULLUP);     //Enter
  lv_indev_drv_t key_drv;
  lv_indev_drv_init(&key_drv);
  key_drv.type = LV_INDEV_TYPE_KEYPAD;
  key_drv.read_cb = keys_read; // Callback that handles joystick and keys
  lv_indev_drv_register(&key_drv);
  wio_key_dev = lv_indev_drv_register(&key_drv);

  sfud_init();
#ifdef SFUD_USING_QSPI
  sfud_qspi_fast_read_enable(sfud_get_device(SFUD_W25Q32_DEVICE_INDEX), 2);
#endif
  flash = sfud_get_device_table() + 0;
  //sfud_erase(flash, 0, sizeof(ConfigData));
  sfud_err flash_status = sfud_read(flash, 0, 1, (uint8_t *)p_Config); //Read one byte to test if flash is availabe
  if(flash_status == SFUD_SUCCESS && p_Config->test == 0xab)  //and there is valid data
  {
    sfud_read(flash, 0, sizeof(ConfigData), (uint8_t *)p_Config);
  }
  p_Config->test = 0xab;

  lv_obj_t *screen = lv_obj_create(NULL, NULL);
  gui_init(screen);
  lv_scr_load(screen);

  if(flash_status != SFUD_SUCCESS)
  {
    lv_obj_t *error = lv_label_create(screen, NULL);
    lv_obj_set_style_local_text_color(error, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_RED);
    lv_label_set_text_fmt(error, "Flash memory not available.\nCannot save any settings. Code: %d", flash_status);
    lv_obj_align(error, NULL, LV_ALIGN_IN_BOTTOM_LEFT, 10, -10);
  }
  lv_task_t *update_gui = lv_task_create(update_gui_task, 2000, LV_TASK_PRIO_MID, NULL);
  lv_task_ready(update_gui);
  lv_task_t *update_rds = lv_task_create(update_rds_task, 500, LV_TASK_PRIO_MID, NULL);

  rda.setup();
  delay(1000);
  rda_set_config();
  rda.setFrequency(p_Config->frequency);
  rda.setVolume(p_Config->volume);
}

void loop()
{
  lv_tick_handler();
  lv_task_handler(); /* let the GUI do its work */
  delay(5);
}

void save_settings()
{
  sfud_erase_write(flash, 0, sizeof(ConfigData), (uint8_t *)p_Config);
}

// Radio control functions
void rda_get_frequency()
{
  gui_show_frequency(rda.getFrequency());
}

void rda_seek(bool up)
{
  rda.seek(0, up, rda_get_frequency);
  p_Config->frequency = rda.getRealFrequency(); 
  sfud_erase_write(flash, 0, sizeof(ConfigData), (uint8_t *)p_Config);
}

void rda_set_next_frequency(uint16_t freq)
{
  rda.setFrequency(freq);
  p_Config->frequency = rda.getRealFrequency(); 
  gui_show_frequency(p_Config->frequency);
  sfud_erase_write(flash, 0, sizeof(ConfigData), (uint8_t *)p_Config);
}

void rda_set_volume()
{
  rda.setVolume(p_Config->volume);
  sfud_erase_write(flash, 0, sizeof(ConfigData), (uint8_t *)p_Config);
}

void rda_set_config()
{
  rda.setMono(p_Config->mono);
  rda.setBass(p_Config->bass_boost);
  rda.setBand(p_Config->band);
  rda.setFmDeemphasis(!p_Config->usa);
  p_Config->usa ? rda.setRBDS(p_Config->rds_on) : rda.setRDS(p_Config->rds_on);
  rda.setSpace(p_Config->space);
  if(p_Config->frequency == 0)
    p_Config->frequency = rda.getRealFrequency();

  rda.setRdsFifo(true);
  //char dbg[64];
  //sprintf(dbg, "Mono: %d, Bass: %d, band: %d, space: %d, USA: %d", p_Config->mono, p_Config->bass_boost, p_Config->band, p_Config->space, p_Config->usa);
  //Serial.println(dbg);
}
