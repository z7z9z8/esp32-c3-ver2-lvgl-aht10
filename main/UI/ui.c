#include <stdio.h>
#include "esp_system.h"
#include "esp_log.h"
#include "lvgl.h"
#include "INC/ui.h"
#include "INC/ui_fan.h"

static const char *TAG = "ui";

//这是一个组
static lv_group_t *group;
static lv_obj_t *main_show;

esp_err_t ui_init(void)
{
    //创建一个组
    group = lv_group_create();
    ESP_LOGI(TAG, "group create is OK");
    //检测输入设备的
    lv_indev_t *indev = lv_indev_get_next(NULL);
    ESP_LOGI(TAG, "get indev is OK");
    if (LV_INDEV_TYPE_ENCODER == lv_indev_get_type(indev)) {
        LV_LOG_USER("add group for encoder");
        lv_indev_set_group(indev, group);
        ESP_LOGI(TAG, "set indev in group is OK");
        lv_group_focus_freeze(group, false);
    }
    main_show=lv_scr_act();
    ESP_LOGI(TAG, "get active screen is ok");
    // ui_fan_init(main_show);
    //ui_calendar(main_show);
    //main_memu();
    //show_time();
    gps_ui();
    ESP_LOGI(TAG, "ui fan  is ok");
    return ESP_OK;
}

void ui_add_obj_to_encoder_group(lv_obj_t *obj)
{
    lv_group_add_obj(group, obj);
}

void ui_remove_all_objs_from_encoder_group(void)
{
    lv_group_remove_all_objs(group);
}

uint32_t ui_get_num_offset(uint32_t num, int32_t max, int32_t offset)
{
    if (num >= max) {
        LV_LOG_USER("[ERROR] num should less than max");
        return num;
    }

    uint32_t i;
    if (offset >= 0) {
        i = (num + offset) % max;
    } else {
        offset = max + (offset % max);
        i = (num + offset) % max;
    }

    return i;
}
