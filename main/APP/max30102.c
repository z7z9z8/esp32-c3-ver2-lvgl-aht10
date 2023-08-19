/*
 */

#include <string.h>
#include <inttypes.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>
#include <driver/i2c.h>
#include <max30102.h>

/*i2c接口相关的函数*/
#define I2C_MASTER_TX_BUF_DISABLE 0
#define I2C_MASTER_RX_BUF_DISABLE 0
#define WRITE_BIT I2C_MASTER_WRITE
#define READ_BIT I2C_MASTER_READ
#define ACK_CHECK_EN 0x1
#define ACK_CHECK_DIS 0x0
#define ACK_VAL 0x0
#define NACK_VAL 0x1
#define LAST_NACK_VAL 0x2

#define I2C_ADDR_MAX30102 0x57 // max30102 i2c address
#define i2c_port 0
#define i2c_frequency 800000
#define i2c_gpio_sda 4
#define i2c_gpio_scl 5

static esp_err_t i2c_init()
{
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = i2c_gpio_sda,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num = i2c_gpio_scl,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = i2c_frequency};
    return i2c_param_config(i2c_port, &conf);
}

static int i2cdetect()
{
    i2c_init();
    i2c_driver_install(i2c_port, I2C_MODE_MASTER, I2C_MASTER_RX_BUF_DISABLE,
                       I2C_MASTER_TX_BUF_DISABLE, 0);
    uint8_t address;
    printf("     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f\r\n");
    for (int i = 0; i < 128; i += 16)
    {
        printf("%02x: ", i);
        for (int j = 0; j < 16; j++)
        {
            fflush(stdout);
            address = i + j;
            i2c_cmd_handle_t cmd = i2c_cmd_link_create();
            i2c_master_start(cmd);
            i2c_master_write_byte(cmd, (address << 1) | WRITE_BIT, ACK_CHECK_EN);
            i2c_master_stop(cmd);
            esp_err_t ret = i2c_master_cmd_begin(i2c_port, cmd, 50 / portTICK_RATE_MS);
            i2c_cmd_link_delete(cmd);
            if (ret == ESP_OK)
            {
                printf("%02x ", address);
            }
            else if (ret == ESP_ERR_TIMEOUT)
            {
                printf("UU ");
            }
            else
            {
                printf("-- ");
            }
        }
        printf("\r\n");
    }
    printf("\r\n\n");
    i2c_driver_delete(i2c_port);
    return 0;
}

static int i2c_read(uint8_t chip_addr, uint8_t data_addr, uint8_t *data_rd, size_t len)
{
    i2c_init();
    vTaskDelay(1);
    i2c_driver_install(i2c_port, I2C_MODE_MASTER, I2C_MASTER_RX_BUF_DISABLE,
                       I2C_MASTER_TX_BUF_DISABLE, 0);
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, chip_addr << 1 | WRITE_BIT, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, data_addr, ACK_CHECK_EN);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, chip_addr << 1 | READ_BIT, ACK_CHECK_EN);
    if (len > 1)
    {
        i2c_master_read(cmd, data_rd, len - 1, ACK_VAL);
    }
    i2c_master_read_byte(cmd, data_rd + len - 1, NACK_VAL);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(i2c_port, cmd, 100 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    if (ret == ESP_OK)
    {
        // for(int i=0; i<len; i++){ printf ("%d %x\n", i, data_rd[i]); }
    }
    else if (ret == ESP_ERR_TIMEOUT)
    {
        ESP_LOGW("", "Bus is busy");
    }
    else
    {
        ESP_LOGW("", "Read failed");
    }
    i2c_driver_delete(i2c_port);
    vTaskDelay(2);
    return 0;
}

static int i2c_write_block(int chip_addr, int data_addr, uint8_t *wr_data, int len)
{
    i2c_init();
    i2c_driver_install(i2c_port, I2C_MODE_MASTER, I2C_MASTER_RX_BUF_DISABLE,
                       I2C_MASTER_TX_BUF_DISABLE, 0);
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, chip_addr << 1 | WRITE_BIT, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, data_addr, ACK_CHECK_EN);
    for (int i = 0; i < len; i++)
    {
        i2c_master_write_byte(cmd, wr_data[i], ACK_CHECK_EN);
    }
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(i2c_port, cmd, 100 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    if (ret == ESP_OK)
    { // ESP_LOGI("", "Write OK addr %x  datablock\n", chip_addr);
    }
    else if (ret == ESP_ERR_TIMEOUT)
    {
        ESP_LOGW("", "Bus is busy");
    }
    else
    {
        ESP_LOGW("", "Write Failed");
    }
    i2c_driver_delete(i2c_port);
    return 0;
}

static int i2c_write(int chip_addr, int data_addr, int wr_data)
{
    i2c_init();
    i2c_driver_install(i2c_port, I2C_MODE_MASTER, I2C_MASTER_RX_BUF_DISABLE,
                       I2C_MASTER_TX_BUF_DISABLE, 0);
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, chip_addr << 1 | WRITE_BIT, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, data_addr, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, wr_data, ACK_CHECK_EN);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(i2c_port, cmd, 100 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    if (ret == ESP_OK)
    { // ESP_LOGI("", "Write OK addr %x  data %x\n", chip_addr, wr_data);
    }
    else if (ret == ESP_ERR_TIMEOUT)
    {
        ESP_LOGW("", "Bus is busy");
    }
    else
    {
        ESP_LOGW("", "Write Failed");
    }
    i2c_driver_delete(i2c_port);
    return 0;
}

/*max30102驱动*/
// 全局变量
char outStr[1500];
float meastime;
int countedsamples = 0;
int irpower = 0, rpower = 0, lirpower = 0, lrpower = 0;
int startstop = 0, raworbp = 0;
float heartrate = 99.2, pctspo2 = 99.2;

void max30102_init()
{
    uint8_t data;
    data = (0x2 << 5); // sample averaging 0=1,1=2,2=4,3=8,4=16,5+=32
    i2c_write(I2C_ADDR_MAX30102, 0x08, data);
    data = 0x03; // mode = red and ir samples
    i2c_write(I2C_ADDR_MAX30102, 0x09, data);
    data = (0x3 << 5) + (0x3 << 2) + 0x3; // first and last 0x3, middle smap rate 0=50,1=100,etc
    i2c_write(I2C_ADDR_MAX30102, 0x0a, data);
    data = 0xd0; // ir pulse power
    i2c_write(I2C_ADDR_MAX30102, 0x0c, data);
    data = 0xa0; // red pulse power
    i2c_write(I2C_ADDR_MAX30102, 0x0d, data);
}

extern SemaphoreHandle_t hr_mutex;
void max30102_task(void *pvParameters)
{
    max30102_init();
    hr_spo2_t *hr_spo2 = (hr_spo2_t *)pvParameters;

    int cnt, samp, tcnt = 0, cntr = 0;
    uint8_t rptr, wptr;   // 读指针 写指针
    uint8_t data;         // 数据
    uint8_t regdata[256]; // fifo的数据
    // int irmeas, redmeas;
    float firxv[5], firyv[5], fredxv[5], fredyv[5];
    float lastmeastime = 0; // 最后一次测量时间
    float hrarray[10], spo2array[10];
    int hrarraycnt = 0;
    while (1)
    {
        // 打开灯光
        if (lirpower != irpower)
        {
            data = (uint8_t)irpower;
            i2c_write(I2C_ADDR_MAX30102, 0x0d, data);
            lirpower = irpower;
        }
        if (lrpower != rpower)
        {
            data = (uint8_t)rpower;
            i2c_write(I2C_ADDR_MAX30102, 0x0c, data);
            lrpower = rpower;
        }

        // 读指针
        i2c_read(I2C_ADDR_MAX30102, 0x04, &wptr, 1);
        i2c_read(I2C_ADDR_MAX30102, 0x06, &rptr, 1);

        // 计算可读的样本
        samp = ((32 + wptr) - rptr) % 32;

        // 将可读的样本读出来 6*samp 一个样本6个Byte
        i2c_read(I2C_ADDR_MAX30102, 0x07, regdata, 6 * samp);
        // ESP_LOGI("samp","----  %d %d %d %d",  adc_read_ptr,samp,wptr,rptr);

        // 处理数据
        for (cnt = 0; cnt < samp; cnt++)
        {

            meastime = 0.01 * tcnt++;

            // 将 firxv 数组中的值向前移动一位，留出 firxv[4]
            firxv[0] = firxv[1];
            firxv[1] = firxv[2];
            firxv[2] = firxv[3];
            firxv[3] = firxv[4];
            //(1/3.48311) 系数 2048->587
            firxv[4] = (1 / 3.48311) * (256 * 256 * (regdata[6 * cnt + 3] % 4) + 256 * regdata[6 * cnt + 4] + regdata[6 * cnt + 5]);
            firyv[0] = firyv[1];
            firyv[1] = firyv[2];
            firyv[2] = firyv[3];
            firyv[3] = firyv[4];
            // FIR 滤波器
            firyv[4] = (firxv[0] + firxv[4]) - 2 * firxv[2] + (-0.1718123813 * firyv[0]) + (0.3686645260 * firyv[1]) + (-1.1718123813 * firyv[2]) + (1.9738037992 * firyv[3]);

            // 同上
            fredxv[0] = fredxv[1];
            fredxv[1] = fredxv[2];
            fredxv[2] = fredxv[3];
            fredxv[3] = fredxv[4];
            //          系数
            fredxv[4] = (1 / 3.48311) * (256 * 256 * (regdata[6 * cnt + 0] % 4) + 256 * regdata[6 * cnt + 1] + regdata[6 * cnt + 2]);
            fredyv[0] = fredyv[1];
            fredyv[1] = fredyv[2];
            fredyv[2] = fredyv[3];
            fredyv[3] = fredyv[4];
            fredyv[4] = (fredxv[0] + fredxv[4]) - 2 * fredxv[2] + (-0.1718123813 * fredyv[0]) + (0.3686645260 * fredyv[1]) + (-1.1718123813 * fredyv[2]) + (1.9738037992 * fredyv[3]);

            if (-1.0 * firyv[4] >= 100 && -1.0 * firyv[2] > -1 * firyv[0] && -1.0 * firyv[2] > -1 * firyv[4] && meastime - lastmeastime > 0.5)
            {
                hrarray[hrarraycnt % 5] = 60 / (meastime - lastmeastime);
                spo2array[hrarraycnt % 5] = 110 - 25 * ((fredyv[4] / fredxv[4]) / (firyv[4] / firxv[4]));
                if (spo2array[hrarraycnt % 5] > 100)
                    spo2array[hrarraycnt % 5] = 99.9;

                lastmeastime = meastime;
                hrarraycnt++;
                // 获取互斥量
                if (xSemaphoreTake(hr_mutex, portMAX_DELAY) == pdTRUE)
                {
                    hr_spo2->heartrate = (hrarray[0] + hrarray[1] + hrarray[2] + hrarray[3] + hrarray[4]) / 5;
                    if (heartrate < 40 || heartrate > 150)
                        heartrate = 0;
                    hr_spo2->pctspo2 = (spo2array[0] + spo2array[1] + spo2array[2] + spo2array[3] + spo2array[4]) / 5;
                    if (pctspo2 < 50 || pctspo2 > 101)
                        pctspo2 = 0;
                    // 释放互斥量
                    xSemaphoreGive(hr_mutex);
                }

                printf("%6.2f  %4.2f     hr= %5.1f     spo2= %5.1f\n", meastime, meastime - lastmeastime, hr_spo2->heartrate, hr_spo2->pctspo2);
            }

            // char tmp[32];
            // countedsamples++;
            // if (countedsamples < 100)
            // {
            //     if (raworbp == 0)
            //     {
            //         snprintf(tmp, sizeof tmp, "%5.1f,%5.1f,", -1 * fredyv[4], -1 * firyv[4]);
            //         strcat(outStr, tmp);
            //     }
            //     else
            //     {
            //         snprintf(tmp, sizeof tmp, "%5.1f,%5.1f,", fredxv[4], firxv[4]);
            //         strcat(outStr, tmp);
            //     }
            // }
        }
    }
}
