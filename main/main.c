#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
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
#include "freertos/event_groups.h"

#include <driver/timer.h>
timer_group_t timer_group = TIMER_GROUP_0;
timer_idx_t timer_id = TIMER_0;

#include "esp_wifi.h"
#include "esp_wpa2.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_smartconfig.h"

/*ntp*/
#include "esp_attr.h"
#include "esp_sntp.h"
static const char *TAG_sntp = "time_sntp";

static void obtain_time(void);
static void initialize_sntp(void);

void time_sync_notification_cb(struct timeval *tv)
{
    ESP_LOGI(TAG_sntp, "Notification of a time synchronization event");
}

static void obtain_time(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    initialize_sntp();

    // wait for time to be set
    time_t now = 0;
    struct tm timeinfo = {0};
    int retry = 0;
    const int retry_count = 10;
    while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET && ++retry < retry_count)
    {
        ESP_LOGI(TAG_sntp, "Waiting for system time to be set... (%d/%d)", retry, retry_count);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
    time(&now);
    localtime_r(&now, &timeinfo);
}
static void initialize_sntp(void)
{
    ESP_LOGI(TAG_sntp, "Initializing SNTP");
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "ntp.sjtu.edu.cn");
    sntp_set_time_sync_notification_cb(time_sync_notification_cb);
    sntp_init();
}

void IRAM_ATTR timer_isr(void *param)
{
    timer_set_counter_value(timer_group, timer_id, 0);
    // 中断处理程序的逻辑代码
    obtain_time();
}

void timer_ntp()
{
    timer_isr_register(timer_group, timer_id, timer_isr, NULL, ESP_INTR_FLAG_IRAM, NULL);
    timer_config_t config = {
        .alarm_en = TIMER_ALARM_DIS,
        .counter_en = TIMER_START,
        .intr_type = TIMER_INTR_LEVEL,
        .counter_dir = TIMER_COUNT_UP,
        .auto_reload = TIMER_AUTORELOAD_EN,
        .divider = 80 // 分频器，以80MHz时钟频率为基准，1分频得到1MHz
    };
    timer_init(timer_group, timer_id, &config);
    timer_set_counter_value(timer_group, timer_id, 0);
    timer_set_alarm_value(timer_group, timer_id, 5000000); // 定时器周期为1秒（1MHz * 1秒 = 1000000）
    timer_start(timer_group, timer_id);
}

#define LV_TICK_PERIOD_MS 1

char *gps_raw = NULL;
char *dest = NULL;

extern lv_obj_t *ui_jingdu;
extern lv_obj_t *ui_weidu;
// extern lv_obj_t *ui_time;
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

// struct tm
// {
//     int tm_sec;   /*代表目前秒数，正常范围为0-59，但允许至61秒 */
//     int tm_min;   /*代表目前分数，范围0-59*/
//     int tm_hour;  /* 从午夜算起的时数，范围为0-23 */
//     int tm_mday;  /* 目前月份的日数，范围01-31 */
//     int tm_mon;   /*代表目前月份，从一月算起，范围从0-11 */
//     int tm_year;  /*从1900 年算起至今的年数*/
//     int tm_wday;  /* 一星期的日数，从星期一算起，范围为0-6。*/
//     int tm_yday;  /* Days in year.[0-365] */
//     int tm_isdst; /*日光节约时间的旗标DST. [-1/0/1]*/
// };

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

/* FreeRTOS event group to signal when we are connected & ready to make a request */
static EventGroupHandle_t s_wifi_event_group;

/* The event group allows multiple bits for each event,
   but we only care about one event - are we connected
   to the AP with an IP? */
static const int CONNECTED_BIT = BIT0;
static const int ESPTOUCH_DONE_BIT = BIT1;
static const char *TAG = "smartconfig_example";

static void smartconfig_example_task(void *parm);

static void event_handler(void *arg, esp_event_base_t event_base,
                          int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        xTaskCreate(smartconfig_example_task, "smartconfig_example_task", 4096, NULL, 3, NULL);
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        esp_wifi_connect();
        xEventGroupClearBits(s_wifi_event_group, CONNECTED_BIT);
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        xEventGroupSetBits(s_wifi_event_group, CONNECTED_BIT);
    }
    else if (event_base == SC_EVENT && event_id == SC_EVENT_SCAN_DONE)
    {
        ESP_LOGI(TAG, "Scan done");
    }
    else if (event_base == SC_EVENT && event_id == SC_EVENT_FOUND_CHANNEL)
    {
        ESP_LOGI(TAG, "Found channel");
    }
    else if (event_base == SC_EVENT && event_id == SC_EVENT_GOT_SSID_PSWD)
    {
        ESP_LOGI(TAG, "Got SSID and password");

        smartconfig_event_got_ssid_pswd_t *evt = (smartconfig_event_got_ssid_pswd_t *)event_data;
        wifi_config_t wifi_config;
        uint8_t ssid[33] = {0};
        uint8_t password[65] = {0};
        uint8_t rvd_data[33] = {0};

        bzero(&wifi_config, sizeof(wifi_config_t));
        memcpy(wifi_config.sta.ssid, evt->ssid, sizeof(wifi_config.sta.ssid));
        memcpy(wifi_config.sta.password, evt->password, sizeof(wifi_config.sta.password));
        wifi_config.sta.bssid_set = evt->bssid_set;
        if (wifi_config.sta.bssid_set == true)
        {
            memcpy(wifi_config.sta.bssid, evt->bssid, sizeof(wifi_config.sta.bssid));
        }

        memcpy(ssid, evt->ssid, sizeof(evt->ssid));
        memcpy(password, evt->password, sizeof(evt->password));
        ESP_LOGI(TAG, "SSID:%s", ssid);
        ESP_LOGI(TAG, "PASSWORD:%s", password);
        if (evt->type == SC_TYPE_ESPTOUCH_V2)
        {
            ESP_ERROR_CHECK(esp_smartconfig_get_rvd_data(rvd_data, sizeof(rvd_data)));
            ESP_LOGI(TAG, "RVD_DATA:");
            for (int i = 0; i < 33; i++)
            {
                printf("%02x ", rvd_data[i]);
            }
            printf("\n");
        }

        ESP_ERROR_CHECK(esp_wifi_disconnect());
        ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
        esp_wifi_connect();
    }
    else if (event_base == SC_EVENT && event_id == SC_EVENT_SEND_ACK_DONE)
    {
        xEventGroupSetBits(s_wifi_event_group, ESPTOUCH_DONE_BIT);
    }
}

static void initialise_wifi(void)
{
    ESP_ERROR_CHECK(esp_netif_init());
    s_wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
    assert(sta_netif);

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(SC_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());
}

static void smartconfig_example_task(void *parm)
{
    EventBits_t uxBits;
    ESP_ERROR_CHECK(esp_smartconfig_set_type(SC_TYPE_ESPTOUCH));
    smartconfig_start_config_t cfg = SMARTCONFIG_START_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_smartconfig_start(&cfg));
    while (1)
    {
        uxBits = xEventGroupWaitBits(s_wifi_event_group, CONNECTED_BIT | ESPTOUCH_DONE_BIT, true, false, portMAX_DELAY);
        if (uxBits & CONNECTED_BIT)
        {
            ESP_LOGI(TAG, "WiFi Connected to ap");
        }
        if (uxBits & ESPTOUCH_DONE_BIT)
        {
            ESP_LOGI(TAG, "smartconfig over");
            esp_smartconfig_stop();
            vTaskDelete(NULL);
        }
    }
}


esp_event_handler_instance_t instance_any_id;

esp_err_t wifi_event_handler(void *ctx, system_event_t *event)
{
    switch (event->event_id) {
        case SYSTEM_EVENT_STA_START:
            ESP_LOGI(TAG, "Wi-Fi STA started");
            esp_wifi_connect();
            break;
        case SYSTEM_EVENT_STA_CONNECTED:
            ESP_LOGI(TAG, "Wi-Fi STA connected");
            break;
        case SYSTEM_EVENT_STA_GOT_IP:
            ESP_LOGI(TAG, "Wi-Fi connected, IP address: %s", ip4addr_ntoa(&event->event_info.got_ip.ip_info.ip));
            // 连接成功，执行您的逻辑代码
            break;
        // 其他Wi-Fi事件处理...
        default:
            break;
    }

    return ESP_OK;
}
void wifi()
{
    esp_netif_init();
    esp_wifi_init(&wifi_init_config_t());
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = "MERCURY_9A5E",         // 替换为您的Wi-Fi网络的SSID
            .password = "11111111", // 替换为您的Wi-Fi网络的密码
            .scan_method = WIFI_ALL_CHANNEL_SCAN,
            .sort_method = WIFI_CONNECT_AP_BY_SIGNAL,
            .threshold.rssi = -127,
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        },
    };

    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config);
    esp_wifi_start();


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
    esp_event_loop_create_default();
    esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, &instance_any_id);
    while (true) {
        if (wifi_is_connected()) {
            ESP_LOGI(TAG, "Wi-Fi is connected");
            // Wi-Fi连接成功，执行您的逻辑代码
            break;
        }
        vTaskDelay(pdMS_TO_TICKS(1000));  // 每秒检查一次连接状态
    }
    // obtain_time();
    // ESP_ERROR_CHECK( nvs_flash_init() );
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
