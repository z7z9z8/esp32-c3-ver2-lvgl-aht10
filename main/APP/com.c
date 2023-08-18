#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_freertos_hooks.h"
#include "freertos/event_groups.h"
#include "freertos/semphr.h"
#include "esp_system.h"
#include <time.h>
#include "esp_attr.h"
#include "esp_sntp.h"
#include <driver/timer.h>

#include "com.h"

// dest = (char *)malloc(16);
// gps_raw = (char *)malloc(RX_BUF_SIZE + 1);
// // uart_init();

/*
UTC_TIME utctime;
// UTC时间转换为北京时间
void UTCToBeijing(unsigned int year, char month, char day, char hour, char minute, char second)
{
    hour += 8;
    if (1 == month || 3 == month || 5 == month || 7 == month || 8 == month || 10 == month || 12 == month) // 1,3,5,7,8,9,12月每月为31天
    {
        if (24 <= hour)
        {
            hour -= 24;
            day += 1; // 如果超过24小时，减去24小时，后再加上一天
            if (day > 31)
            {
                day -= 31;
                month += 1;
            } // 如果超过31一天，减去31天，后加上一个月
        }
    }
    else if (4 == month || 6 == month || 9 == month || 11 == month) // 4，6，9，11月每月为30天
    {
        if (24 <= hour)
        {
            hour -= 24;
            day += 1; // 如果超过24小时，减去24小时，后再加上一天
            if (30 < day)
            {
                day -= 30;
                month += 1;
            } // 如果超过30一天，减去30天，后加上一个月
        }
    }
    else // 剩下为2月，闰年为29天，平年为28天
    {
        if (24 <= hour)
        {
            hour -= 24;
            day += 1;
            if ((0 == year % 400) || (0 == year % 4 && 0 != year % 100)) // 判断是否为闰年，年号能被400整除或年号能被4整除，而不能被100整除为闰年
            {
                if (29 < day)
                {
                    day -= 29;
                    month += 1;
                }
            } // 为闰年
            else
            {
                if (28 < day)
                {
                    day -= 28;
                    month += 1;
                }
            } // 为平年
        }
    }

    if (12 < month)
    {
        month -= 12;
        year += 1;
    }

    utctime.year = year;
    utctime.month = month;
    utctime.day = day;

    utctime.hour = hour;
    utctime.min = minute;
    utctime.sec = second;
}


float change_gps_data(char *gps_data)
{
    uint16_t zhenshu = atoi(gps_data);
    float xiaoshu = atof(gps_data) - zhenshu;
    uint16_t lat_mm = zhenshu % 100;

    uint16_t lat_du = zhenshu / 100;
    float lat_fen = (lat_mm + xiaoshu) / 60;
    return lat_du + lat_fen;
}
void print_gps_data(void *pvParameters)
{
    // vTaskDelay(3000 / portTICK_PERIOD_MS);
    static GPRMC_DATA gps_all = {"0", 0, "0", 0, "0", 0, "0", "0"};
    static gps_time_t gps_time = {"0", "0", "0", "0"};

    uint16_t rxBytes = 9;

    time_t timer; // time_t就是long int 类型
    struct tm *tblock;

    int unm = 0;

    // while (rxBytes > 0)
    while (1)
    {
        ESP_LOGI("start", "reading");
        unm++;
        // lv_label_set_text_fmt(ui_weidu, "lat:#0000ff %f#", tem);
        // lv_label_set_text_fmt(ui_jingdu, "lon:#ff02ff %d#", unm);
        //  rxBytes = uart_get_data(gps_raw);
        //  gps_all = get_gps_moreData(gps_raw);

        // if (gps_all.is_Effective_positioning)
        // {
        //     gps_time = get_gps_time(gps_raw);
        // }

        // if (ui_all != NULL)
        // {

        //     UTCToBeijing(atoi(gps_time.year), atoi(gps_time.month), atoi(gps_time.day), atoi(gps_all.UTC_time) / 10000, atoi(gps_all.UTC_time) / 100 % 100, atoi(gps_all.UTC_time) % 100);
        //     lv_label_set_text_fmt(ui_all, "%d-%d-%d #ff0000 %d-%d-%d#", utctime.year, utctime.month, utctime.day, utctime.hour, utctime.min, utctime.sec);
        //     lv_label_set_text_fmt(ui_weidu, "lat:#0000ff %f#", change_gps_data(gps_all.lat));
        //     lv_label_set_text_fmt(ui_jingdu, "lon:#ff02ff %f#", change_gps_data(gps_all.lon));
        // }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
*/
