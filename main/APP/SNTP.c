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
timer_group_t timer_group = TIMER_GROUP_0;
timer_idx_t timer_id = TIMER_0;
#include "SNTP.h"
static const char *TAG_sntp = "time_sntp";

void time_sync_notification_cb(struct timeval *tv)
{
    ESP_LOGI(TAG_sntp, "Notification of a time synchronization event");
}
static void initialize_sntp(void)
{
    ESP_LOGI(TAG_sntp, "Initializing SNTP");
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "ntp.sjtu.edu.cn");
    sntp_set_time_sync_notification_cb(time_sync_notification_cb);
    sntp_init();
}

void obtain_time(void)
{
    // ESP_ERROR_CHECK(nvs_flash_init());
    // ESP_ERROR_CHECK(esp_netif_init());
    // ESP_ERROR_CHECK(esp_event_loop_create_default());

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
