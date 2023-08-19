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
#include "APP/max30102.h"

/*心率模块*/
#define I2C_SDA                 (GPIO_NUM_4)
#define I2C_SCL                 (GPIO_NUM_5)
#define I2C_FRQ                     400000
#define I2C_PORT                   I2C_NUM_0
max30102_t max30102 = {};

esp_err_t i2c_master_init(i2c_port_t i2c_port){
    i2c_config_t conf = {};
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = I2C_SDA;
    conf.scl_io_num = I2C_SCL;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = I2C_FRQ;
    i2c_param_config(i2c_port, &conf);
    return i2c_driver_install(i2c_port, I2C_MODE_MASTER, 0, 0, 0);
}

void heart_test(void *pvParameters){

    ESP_ERROR_CHECK(i2c_master_init(I2C_PORT));
    //Init sensor at I2C_NUM_0
    ESP_ERROR_CHECK(max30102_init( &max30102,I2C_PORT));
    while(1){
        uint16_t RAWsensorDataRED[FIFO_A_FULL/2], RAWsensorDataIR[FIFO_A_FULL/2];
        max30102_read_fifo(I2C_NUM_0, RAWsensorDataRED,RAWsensorDataIR);
        printf("%u\n", RAWsensorDataRED[0]);
        // printf("%u\t\n", RAWsensorDataIR[0]);
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

#define LV_TICK_PERIOD_MS 1

static void lv_tick_task(void *arg);
static void guiTask();

/*获取时间*/
time_t now;
char strftime_buf[64];
struct tm timeinfo;
char *weekday;

extern lv_obj_t *ui_tem;
extern lv_obj_t *ui_hum;
extern lv_obj_t *ui_time;
extern lv_obj_t *ui_day;

static void get_time()
{
    time(&now);

    localtime_r(&now, &timeinfo);
    strftime(strftime_buf, sizeof(strftime_buf), "%Y-%m-%d %H:%M:%S", &timeinfo);

    // 获取当前星期并打印
    int week = timeinfo.tm_wday;

    switch (week)
    {
    case 0:
        weekday = "Sunday";
        break;
    case 1:
        weekday = "Monday";
        break;
    case 2:
        weekday = "Tuesday";
        break;
    case 3:
        weekday = "Wednesday";
        break;
    case 4:
        weekday = "Thursday";
        break;
    case 5:
        weekday = "Friday";
        break;
    case 6:
        weekday = "Saturday";
        break;
    default:
        weekday = "Invalid";
        break;
    }
    strcpy(strftime_buf + 19, weekday);

    ESP_LOGI("time", "The current date/time in Shanghai is: %s", strftime_buf);
}

static const char *TAG_TEMP = "aht-example";

void temp_hum_task(void *pvParameters)
{
    aht_t dev = {0};
    dev.mode = AHT_MODE_NORMAL;
    dev.type = AHT_TYPE_AHT1x;

    int8_t update_count = 0;

    while (1)
    {
        update_count++;
        if (update_count >= 2)
        {
            /*打开i2c开始读取温湿度信息*/
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

            if (res == ESP_OK)
                ESP_LOGI(TAG_TEMP, "Temperature: %.1f°C, Humidity: %.2f%%", temperature, humidity);
            else
                ESP_LOGE(TAG_TEMP, "Error reading data: %d (%s)", res, esp_err_to_name(res));
                lv_label_set_text_fmt(ui_tem, "Temp:#0000ff %f#", temperature);
                lv_label_set_text_fmt(ui_hum, "Hum:#ff0000 %f#", humidity);

            /*关闭i2c设备*/
            update_count = 0;
        }

        get_time();

        lv_label_set_text_fmt(ui_time, "#800080 %s#", strftime_buf);
        // lv_label_set_text_fmt(ui_day, "#800080 %s#", weekday);

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}



void app_main(void)
{

    // 设置时区 北京
    setenv("TZ", "CST-8", 1);
    tzset();

    lv_init();            // lvgl内核初始化
    guiTask();            // lvgl显示接口初始化
    lv_port_indev_init(); // 按键接口
    ui_init();

    // wifi_init_sta();

    // obtain_time();

    // ESP_ERROR_CHECK(i2cdev_init());
    // xTaskCreatePinnedToCore(temp_hum_task, TAG_TEMP, 1024 * 8, NULL, 5, NULL, 1);
    xTaskCreatePinnedToCore(heart_test, "heart_test", 1024 * 4, NULL, 10, NULL, 1);

    while (true)
    {
        vTaskDelay(10 / portTICK_PERIOD_MS);
        lv_task_handler();
    }
}

// SemaphoreHandle_t xGuiSemaphore;
static void guiTask()
{
    // xGuiSemaphore = xSemaphoreCreateMutex();

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
