#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "esp_freertos_hooks.h"
#include "freertos/semphr.h"
#include "esp_system.h"
#include <time.h>
#include "lv_port_indev.h" //编码器

#include "lvgl.h"
#include "lvgl_helpers.h"

#include "UI/INC/ui.h"
#include "APP/gps.h"
#include "APP/uart_init.h"
#include "APP/AHT10.h"
#include "APP/wifi.h"
#include "APP/SNTP.h"



#define LV_TICK_PERIOD_MS 1

char *gps_raw = NULL;
char *dest = NULL;

extern lv_obj_t *ui_jingdu;
extern lv_obj_t *ui_weidu;
extern lv_obj_t *ui_all;

static void lv_tick_task(void *arg);
static void guiTask();

/*获取时间*/
time_t now;
char strftime_buf[64];
struct tm timeinfo;

extern lv_obj_t *ui_tem;
extern lv_obj_t *ui_hum;
extern lv_obj_t *ui_time;

static void get_time()
{
    time(&now);

    // 将时区设置为中国标准时间
    setenv("TZ", "CST-8", 1);
    tzset();

    localtime_r(&now, &timeinfo);
    strftime(strftime_buf, sizeof(strftime_buf), "%H:%M:%S", &timeinfo);
    // strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
    ESP_LOGI("time", "The current date/time in Shanghai is: %s", strftime_buf);
}



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

static const char *TAG_TEMP = "aht-example";

void temp_hum_task(void *pvParameters)
{
    aht_t dev = {0};
    dev.mode = AHT_MODE_NORMAL;
    dev.type = AHT_TYPE_AHT1x;

    ESP_ERROR_CHECK(aht_init_desc(&dev, AHT_I2C_ADDRESS_GND, 0, 4, 5));
    ESP_ERROR_CHECK(aht_init(&dev));

    bool calibrated;
    ESP_ERROR_CHECK(aht_get_status(&dev, NULL, &calibrated));
    if (calibrated)
        ESP_LOGI(TAG_TEMP, "Sensor calibrated");
    else
        ESP_LOGW(TAG_TEMP, "Sensor not calibrated!");

    float temperature, humidity;

    esp_err_t res = aht_get_data(&dev, &temperature, &humidity);
    lv_label_set_text_fmt(ui_tem, "Temp:#0000ff %f#", temperature);
    lv_label_set_text_fmt(ui_hum, "Hum:#ff0000 %f#", humidity);

    int8_t update_count = 0;

    while (1)
    {
        update_count++;
        if (update_count >= 2)
        {
            res = aht_get_data(&dev, &temperature, &humidity);

            if (res == ESP_OK)
                ESP_LOGI(TAG_TEMP, "Temperature: %.1f°C, Humidity: %.2f%%", temperature, humidity);
            else
                ESP_LOGE(TAG_TEMP, "Error reading data: %d (%s)", res, esp_err_to_name(res));
            lv_label_set_text_fmt(ui_tem, "Temp:#0000ff %f#", temperature);
            lv_label_set_text_fmt(ui_hum, "Hum:#ff0000 %f#", humidity);
            update_count = 0;
        }

        get_time();

        lv_label_set_text_fmt(ui_time, "Time:#800080 %s#", strftime_buf);

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}




void app_main(void)
{
    // dest = (char *)malloc(16);
    // gps_raw = (char *)malloc(RX_BUF_SIZE + 1);
    // // uart_init();

    setenv("TZ", "CST-8", 1);
    tzset();

    lv_init();            // lvgl内核初始化
    guiTask();            // lvgl显示接口初始化
    lv_port_indev_init(); // 按键接口
    ui_init();


    wifi_init_sta();
    // 等待 Wi-Fi 连接成功

    obtain_time();

    // initialise_wifi();

    // timer_ntp();

    ESP_ERROR_CHECK(i2cdev_init());
    xTaskCreatePinnedToCore(temp_hum_task, TAG_TEMP, 1024 * 8, NULL, 5, NULL, 1);
    // xTaskCreatePinnedToCore(&task, "print_gps_data", 1024 * 4, NULL, 10, NULL, 1);

    while (true)
    {
        vTaskDelay(10 / portTICK_PERIOD_MS);
        lv_task_handler();
    }
}

SemaphoreHandle_t xGuiSemaphore;
static void guiTask()
{
    xGuiSemaphore = xSemaphoreCreateMutex();

    lv_init();

    // 初始化屏幕
    lvgl_driver_init();

    lv_color_t *buf1 = heap_caps_malloc(DISP_BUF_SIZE * sizeof(lv_color_t), MALLOC_CAP_DMA);
    assert(buf1 != NULL);

    lv_color_t *buf2 = heap_caps_malloc(DISP_BUF_SIZE * sizeof(lv_color_t), MALLOC_CAP_DMA);
    assert(buf2 != NULL);

    static lv_disp_buf_t disp_buf;

    uint32_t size_in_px = DISP_BUF_SIZE;

    lv_disp_buf_init(&disp_buf, buf1, buf2, size_in_px); // 初始化缓存区

    lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);

    disp_drv.flush_cb = disp_driver_flush;
    disp_drv.buffer = &disp_buf;

    lv_disp_drv_register(&disp_drv);

    /* Create and start a periodic timer interrupt to call lv_tick_inc */
    const esp_timer_create_args_t periodic_timer_args = {
        .callback = &lv_tick_task,
        .name = "periodic_gui"};
    esp_timer_handle_t periodic_timer;

    ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, LV_TICK_PERIOD_MS * 1000));
}

static void lv_tick_task(void *arg)
{
    (void)arg;
    lv_tick_inc(LV_TICK_PERIOD_MS);
}
