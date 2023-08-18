/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <stdio.h>
#include <time.h>
#include "lvgl.h"
#include "INC/ui.h"
#include "ui_fan.h"
#include "driver/gpio.h"

static const char *TAG = "Fan";

void ui_fan_init(lv_obj_t *parent)
{
    lv_obj_t *label1 = lv_label_create(parent, NULL);
    lv_label_set_long_mode(label1, LV_LABEL_LONG_BREAK); /*Break the long lines*/
    lv_label_set_recolor(label1, true);                  /*Enable re-coloring by commands in the text*/
    lv_label_set_align(label1, LV_LABEL_ALIGN_CENTER);   /*Center aligned lines*/
    lv_label_set_text(label1, "#0000ff Re-color# #ff00ff words# #ff0000 of a# label "
                              "and  wrap long text automatically.");
    lv_obj_set_width(label1, 150);
    lv_obj_align(label1, NULL, LV_ALIGN_CENTER, 0, -30);

    lv_obj_t *label2 = lv_label_create(lv_scr_act(), NULL);
    lv_label_set_long_mode(label2, LV_LABEL_LONG_SROLL_CIRC); /*Circular scroll*/
    lv_obj_set_width(label2, 150);
    lv_label_set_text(label2, "It is a circularly scrolling text. ");
    lv_obj_align(label2, NULL, LV_ALIGN_CENTER, 0, 5);

    ui_add_obj_to_encoder_group(label1);
    ui_add_obj_to_encoder_group(label2);
}
static void event_handler1(lv_obj_t *obj, lv_event_t event)
{
    if (event == LV_EVENT_VALUE_CHANGED)
    {
        lv_calendar_date_t *date = lv_calendar_get_pressed_date(obj);
        if (date)
        {
            printf("Clicked date: %02d.%02d.%d\n", date->day, date->month, date->year);
        }
    }
}
void ui_calendar(lv_obj_t *parent)
{
    lv_obj_t *calendar = lv_calendar_create(parent, NULL);
    lv_obj_set_size(calendar, 160, 128);
    lv_obj_align(calendar, NULL, LV_ALIGN_CENTER, 0, 0);

    lv_obj_set_event_cb(calendar, event_handler1);

    /*Make the date number smaller to be sure they fit into their area*/
    lv_obj_set_style_local_text_font(calendar, LV_CALENDAR_PART_DATE, LV_STATE_DEFAULT, lv_theme_get_font_small());

    /*Set today's date*/
    lv_calendar_date_t today;
    today.year = 2018;
    today.month = 10;
    today.day = 23;

    lv_calendar_set_today_date(calendar, &today);
    lv_calendar_set_showed_date(calendar, &today);

    /*Highlight a few days*/
    static lv_calendar_date_t highlighted_days[3]; /*Only its pointer will be saved so should be static*/
    highlighted_days[0].year = 2018;
    highlighted_days[0].month = 10;
    highlighted_days[0].day = 6;

    highlighted_days[1].year = 2018;
    highlighted_days[1].month = 10;
    highlighted_days[1].day = 11;

    highlighted_days[2].year = 2018;
    highlighted_days[2].month = 11;
    highlighted_days[2].day = 22;

    lv_calendar_set_highlighted_dates(calendar, highlighted_days, 3);
    ui_add_obj_to_encoder_group(calendar);
}
static void event_handler(lv_obj_t *obj, lv_event_t event)
{
    if (event == LV_EVENT_CLICKED)
    {
        show_time();
        printf("Clicked\n");
    }
    else if (event == LV_EVENT_VALUE_CHANGED)
    {
        main_memu();
        printf("Toggled\n");
    }
}

void main_memu(void)
{
    lv_obj_t *label;

    lv_obj_t *btn1 = lv_btn_create(lv_scr_act(), NULL);
    lv_obj_set_event_cb(btn1, event_handler);
    lv_obj_set_size(btn1, 50, 20);
    lv_obj_align(btn1, NULL, LV_ALIGN_CENTER, 0, -20);

    label = lv_label_create(btn1, NULL);
    lv_label_set_text(label, "Button");

    lv_obj_t *btn2 = lv_btn_create(lv_scr_act(), NULL);
    lv_obj_set_event_cb(btn2, event_handler);
    lv_obj_set_size(btn2, 50, 20);
    lv_obj_align(btn2, NULL, LV_ALIGN_CENTER, 0, 20);
    lv_btn_set_checkable(btn2, true);
    lv_btn_toggle(btn2);
    lv_btn_set_fit2(btn2, LV_FIT_NONE, LV_FIT_TIGHT);

    label = lv_label_create(btn2, NULL);
    lv_label_set_text(label, "Toggled");
    ui_add_obj_to_encoder_group(btn1);
    ui_add_obj_to_encoder_group(btn2);
}

uint16_t count=0;

static void but_back(lv_obj_t *obj, lv_event_t event)
{
    // if (event == LV_EVENT_CLICKED)
    // {
    //     printf("but_back is ok\n");
    // }
}

static void but_add(lv_obj_t *obj, lv_event_t event)
{
    if (event == LV_EVENT_CLICKED)
    {
        lv_obj_t *label=lv_obj_get_child(obj,0);
        lv_label_set_text_fmt(label,"%d",count);
        count++;

        printf("but_add is ok,count is %d\n",count);
    }
}
lv_obj_t *btn1;

void show_time(void)
{
    lv_obj_t *label;

    btn1 = lv_btn_create(lv_scr_act(), NULL);
    lv_obj_set_event_cb(btn1, but_add);
    lv_obj_set_size(btn1, 50, 20);
    lv_obj_align(btn1, NULL, LV_ALIGN_CENTER, 0, -20);

    label = lv_label_create(btn1, NULL);
    lv_label_set_text_fmt(label, "Add");

    lv_obj_t *btn2 = lv_btn_create(lv_scr_act(), NULL);
    lv_obj_set_event_cb(btn2, but_back);
    lv_obj_set_size(btn2, 50, 20);
    lv_obj_align(btn2, NULL, LV_ALIGN_CENTER, 0, 20);
    lv_btn_set_checkable(btn2, true);
    lv_btn_toggle(btn2);
    lv_btn_set_fit2(btn2, LV_FIT_NONE, LV_FIT_TIGHT);

    label = lv_label_create(btn2, NULL);
    lv_label_set_text(label, "Toggled");

    ui_add_obj_to_encoder_group(btn1);
    ui_add_obj_to_encoder_group(btn2);
}

lv_obj_t *ui_jingdu;
lv_obj_t *ui_weidu;
// lv_obj_t *ui_time;
lv_obj_t *ui_all;

void ui_Screen1_screen_init(void)
{

    ui_all = lv_label_create(lv_scr_act(),NULL);

    lv_obj_set_x(ui_all, 0);
    lv_obj_set_y(ui_all, 0);
    lv_label_set_text(ui_all, "                                  ");
    lv_label_set_recolor(ui_all,true);
    lv_label_set_long_mode(ui_all, LV_LABEL_LONG_BREAK);

    //经度
    ui_jingdu = lv_label_create(lv_scr_act(),NULL);

    lv_obj_set_x(ui_jingdu, 0);
    lv_obj_set_y(ui_jingdu, 20);
    lv_label_set_recolor(ui_jingdu,true);

    lv_label_set_text(ui_jingdu, "lon:");

    //纬度
    ui_weidu = lv_label_create(lv_scr_act(),NULL);
    lv_obj_set_x(ui_weidu, 0);
    lv_obj_set_y(ui_weidu, 40);
    lv_label_set_recolor(ui_weidu,true);


    lv_label_set_text(ui_weidu, "lat:");

    // //时间
    // ui_time = lv_label_create(lv_scr_act(),NULL);
    // lv_obj_set_x(ui_time, 0);
    // lv_obj_set_y(ui_time, 40);

    // lv_label_set_text(ui_time, "UTC:");


}

/*显示温度和湿度的界面*/
lv_obj_t *ui_tem;
lv_obj_t *ui_hum;
lv_obj_t *ui_time;

void ui_show_tem_hum_init(void)
{

    //温度
    ui_tem = lv_label_create(lv_scr_act(),NULL);

    lv_obj_set_x(ui_tem, 0);
    lv_obj_set_y(ui_tem, 0);
    lv_label_set_recolor(ui_tem,true);

    lv_label_set_text(ui_tem, "Temp:");

    //湿度
    ui_hum = lv_label_create(lv_scr_act(),NULL);
    lv_obj_set_x(ui_hum, 0);
    lv_obj_set_y(ui_hum, 20);
    lv_label_set_recolor(ui_hum,true);


    lv_label_set_text(ui_hum, "Hum:");

    //时间
    ui_time = lv_label_create(lv_scr_act(),NULL);
    lv_obj_set_x(ui_time, 0);
    lv_obj_set_y(ui_time, 40);
    lv_label_set_recolor(ui_time,true);

    lv_label_set_text(ui_time, "Time:");

}



void gps_ui(void)
{
    ui_show_tem_hum_init();
}
/*
static void event_handler(lv_obj_t *obj, lv_event_t event)
{
    if (event == LV_EVENT_CLICKED)
    {
        printf("Clicked\n");
    }
    else if (event == LV_EVENT_VALUE_CHANGED)
    {
        printf("Toggled\n");
    }
}

void lv_ex_btn_1(void)
{
    lv_group_t *group;
    group = lv_group_create();

    lv_obj_t *label;

    lv_obj_t *btn1 = lv_btn_create(lv_scr_act(), NULL);
    lv_obj_set_event_cb(btn1, event_handler);
    lv_obj_align(btn1, NULL, LV_ALIGN_CENTER, 0, -40);

    label = lv_label_create(btn1, NULL);
    lv_label_set_text(label, "Button");

    lv_obj_t *btn2 = lv_btn_create(lv_scr_act(), NULL);
    lv_obj_set_event_cb(btn2, event_handler);
    lv_obj_align(btn2, NULL, LV_ALIGN_CENTER, 0, 40);
    lv_btn_set_checkable(btn2, true);
    lv_btn_toggle(btn2);
    lv_btn_set_fit2(btn2, LV_FIT_NONE, LV_FIT_TIGHT);

    label = lv_label_create(btn2, NULL);
    lv_label_set_text(label, "Toggled");

    lv_group_add_obj(group, btn1);
    lv_group_add_obj(group, btn2);

    lv_indev_set_group(indev_encoder, group);
}
*/

/*****
void lv_ex_btn_2(void)
{
    lv_group_t *group;
    group = lv_group_create();

    static lv_anim_path_t path_overshoot;
    lv_anim_path_init(&path_overshoot);
    lv_anim_path_set_cb(&path_overshoot, lv_anim_path_overshoot);

    static lv_anim_path_t path_ease_out;
    lv_anim_path_init(&path_ease_out);
    lv_anim_path_set_cb(&path_ease_out, lv_anim_path_ease_out);

    static lv_anim_path_t path_ease_in_out;
    lv_anim_path_init(&path_ease_in_out);
    lv_anim_path_set_cb(&path_ease_in_out, lv_anim_path_ease_in_out);


    static lv_style_t style_gum;
    lv_style_init(&style_gum);
    lv_style_set_transform_width(&style_gum, LV_STATE_PRESSED, 10);
    lv_style_set_transform_height(&style_gum, LV_STATE_PRESSED, -10);
    lv_style_set_value_letter_space(&style_gum, LV_STATE_PRESSED, 5);
    lv_style_set_transition_path(&style_gum, LV_STATE_DEFAULT, &path_overshoot);
    lv_style_set_transition_path(&style_gum, LV_STATE_PRESSED, &path_ease_in_out);
    lv_style_set_transition_time(&style_gum, LV_STATE_DEFAULT, 250);
    lv_style_set_transition_delay(&style_gum, LV_STATE_DEFAULT, 100);
    lv_style_set_transition_prop_1(&style_gum, LV_STATE_DEFAULT, LV_STYLE_TRANSFORM_WIDTH);
    lv_style_set_transition_prop_2(&style_gum, LV_STATE_DEFAULT, LV_STYLE_TRANSFORM_HEIGHT);
    lv_style_set_transition_prop_3(&style_gum, LV_STATE_DEFAULT, LV_STYLE_VALUE_LETTER_SPACE);

    lv_obj_t *btn1 = lv_btn_create(lv_scr_act(), NULL);
    // lv_obj_set_size(btn1,40,20);
    lv_obj_align(btn1, NULL, LV_ALIGN_CENTER, 0, -30);
    lv_obj_add_style(btn1, LV_BTN_PART_MAIN, &style_gum);

    lv_obj_set_style_local_value_str(btn1, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, "Gum");

    static lv_style_t style_ripple;
    lv_style_init(&style_ripple);
    lv_style_set_transition_time(&style_ripple, LV_STATE_PRESSED, 300);
    lv_style_set_transition_time(&style_ripple, LV_STATE_DEFAULT, 0);
    lv_style_set_transition_delay(&style_ripple, LV_STATE_DEFAULT, 300);
    lv_style_set_bg_opa(&style_ripple, LV_STATE_DEFAULT, 0);
    lv_style_set_bg_opa(&style_ripple, LV_STATE_PRESSED, LV_OPA_80);
    lv_style_set_border_width(&style_ripple, LV_STATE_DEFAULT, 0);
    lv_style_set_outline_width(&style_ripple, LV_STATE_DEFAULT, 0);
    lv_style_set_transform_width(&style_ripple, LV_STATE_DEFAULT, -20);
    lv_style_set_transform_height(&style_ripple, LV_STATE_DEFAULT, -20);
    lv_style_set_transform_width(&style_ripple, LV_STATE_PRESSED, 0);
    lv_style_set_transform_height(&style_ripple, LV_STATE_PRESSED, 0);

    lv_style_set_transition_path(&style_ripple, LV_STATE_DEFAULT, &path_ease_out);
    lv_style_set_transition_prop_1(&style_ripple, LV_STATE_DEFAULT, LV_STYLE_BG_OPA);
    lv_style_set_transition_prop_2(&style_ripple, LV_STATE_DEFAULT, LV_STYLE_TRANSFORM_WIDTH);
    lv_style_set_transition_prop_3(&style_ripple, LV_STATE_DEFAULT, LV_STYLE_TRANSFORM_HEIGHT);

    lv_obj_t *btn3 = lv_btn_create(lv_scr_act(), NULL);
    // lv_obj_set_size(btn3,40,20);
    lv_obj_align(btn3, NULL, LV_ALIGN_CENTER, 0, 20);
    lv_obj_add_style(btn3, LV_BTN_PART_MAIN, &style_ripple);
    lv_obj_set_style_local_value_str(btn3, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, "Ripple");

    lv_group_add_obj(group, btn1);
    // lv_group_add_obj(group ,btn2);
    lv_group_add_obj(group, btn3);

    lv_indev_set_group(indev_encoder, group);
}
****/
