/*
 * @Author: Dawn
 * @Date: 2022-06-24 17:41:58
 * @LastEditors: Dawn
 * @description:
 * @LastEditTime: 2022-08-08 09:41:54
 * Keep On Going
 * Copyright (c) 2022 by Dawn, All Rights Reserved.
 */
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#include "dht11.h"

static struct dht11_reading now_data;

void DHT11_init(gpio_num_t gpio_num_dht11)
{
    gpio_pad_select_gpio(gpio_num_dht11);
}
// us延时函数，误差不能太大
void DelayUs(uint32_t nCount)
{
    ets_delay_us(nCount);
}

void DHT11_Start(void)
{
    DHT11_OUT; //设置端口方向
    DHT11_CLR; //拉低端口
    DelayUs(19 * 1000);
    //   vTaskDelay(19 * portTICK_RATE_MS); //持续最低18ms;

    DHT11_SET;   //释放总线
    DelayUs(30); //总线由上拉电阻拉高，主机延时30uS;
    DHT11_IN;    //设置端口方向

    while (!gpio_get_level(DHT11_PIN))
        ; // DHT11等待80us低电平响应信号结束
    while (gpio_get_level(DHT11_PIN))
        ; // DHT11   将总线拉高80us
}

uint8_t DHT11_ReadValue(void)
{
    uint8_t i, sbuf = 0;
    for (i = 8; i > 0; i--)
    {
        sbuf <<= 1;
        while (!gpio_get_level(DHT11_PIN))
            ;
        DelayUs(30); // 延时 30us 后检测数据线是否还是高电平
        if (gpio_get_level(DHT11_PIN))
        {
            sbuf |= 1;
        }
        else
        {
            sbuf |= 0;
        }
        while (gpio_get_level(DHT11_PIN))
            ;
    }
    return sbuf;
}

//读一字节空数据
void readVOIDdata()
{
    uint8_t i;
    for (i = 8; i > 0; i--)
    {
        while (!gpio_get_level(DHT11_PIN))
            ;
        DelayUs(30);
        while (gpio_get_level(DHT11_PIN))
            ;
    }
}

//读取数据
/**
uint8_t DHT11_ReadTemHum(uint8_t *buf)
{
        DHT11_Start();
    uint8_t check;

#ifdef DHT11_CHECK
    now_data.humidity = DHT11_ReadValue();    // 温度高8位
    readVOIDdata();                           //湿度低八位
    now_data.temperature = DHT11_ReadValue(); //湿度高八位
    readVOIDdata();                           //温度低八位 因分辨率低，始终唯0
    check = DHT11_ReadValue();                //校验位
    if (check == now_data.humidity + now_data.temperature)
    {
        return 1;
    }
    else
    {
        return 0;
    }

#else
    now_data.humidity = DHT11_ReadValue(); // 温度高8位
    readVOIDdata();
    now_data.temperature = DHT11_ReadValue(); //湿度高八位
    readVOIDdata();                           //温度低八位 因分辨率低，始终唯0
    readVOIDdata();                           //校验位
    // now_data.status = TURE;
    return 1;
#endif
}
**/
//读数据，返回结构体
struct dht11_reading DHT11_read()
{
    DHT11_Start();
    now_data.humidity = DHT11_ReadValue(); // 温度高8位
    readVOIDdata();
    now_data.temperature = DHT11_ReadValue(); //湿度高八位
    readVOIDdata();                           //温度低八位 因分辨率低，始终唯0
    readVOIDdata();                           //校验位
    return now_data;
}
