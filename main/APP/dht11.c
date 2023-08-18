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
// us��ʱ����������̫��
void DelayUs(uint32_t nCount)
{
    ets_delay_us(nCount);
}

void DHT11_Start(void)
{
    DHT11_OUT; //���ö˿ڷ���
    DHT11_CLR; //���Ͷ˿�
    DelayUs(19 * 1000);
    //   vTaskDelay(19 * portTICK_RATE_MS); //�������18ms;

    DHT11_SET;   //�ͷ�����
    DelayUs(30); //�����������������ߣ�������ʱ30uS;
    DHT11_IN;    //���ö˿ڷ���

    while (!gpio_get_level(DHT11_PIN))
        ; // DHT11�ȴ�80us�͵�ƽ��Ӧ�źŽ���
    while (gpio_get_level(DHT11_PIN))
        ; // DHT11   ����������80us
}

uint8_t DHT11_ReadValue(void)
{
    uint8_t i, sbuf = 0;
    for (i = 8; i > 0; i--)
    {
        sbuf <<= 1;
        while (!gpio_get_level(DHT11_PIN))
            ;
        DelayUs(30); // ��ʱ 30us �����������Ƿ��Ǹߵ�ƽ
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

//��һ�ֽڿ�����
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

//��ȡ����
/**
uint8_t DHT11_ReadTemHum(uint8_t *buf)
{
        DHT11_Start();
    uint8_t check;

#ifdef DHT11_CHECK
    now_data.humidity = DHT11_ReadValue();    // �¶ȸ�8λ
    readVOIDdata();                           //ʪ�ȵͰ�λ
    now_data.temperature = DHT11_ReadValue(); //ʪ�ȸ߰�λ
    readVOIDdata();                           //�¶ȵͰ�λ ��ֱ��ʵͣ�ʼ��Ψ0
    check = DHT11_ReadValue();                //У��λ
    if (check == now_data.humidity + now_data.temperature)
    {
        return 1;
    }
    else
    {
        return 0;
    }

#else
    now_data.humidity = DHT11_ReadValue(); // �¶ȸ�8λ
    readVOIDdata();
    now_data.temperature = DHT11_ReadValue(); //ʪ�ȸ߰�λ
    readVOIDdata();                           //�¶ȵͰ�λ ��ֱ��ʵͣ�ʼ��Ψ0
    readVOIDdata();                           //У��λ
    // now_data.status = TURE;
    return 1;
#endif
}
**/
//�����ݣ����ؽṹ��
struct dht11_reading DHT11_read()
{
    DHT11_Start();
    now_data.humidity = DHT11_ReadValue(); // �¶ȸ�8λ
    readVOIDdata();
    now_data.temperature = DHT11_ReadValue(); //ʪ�ȸ߰�λ
    readVOIDdata();                           //�¶ȵͰ�λ ��ֱ��ʵͣ�ʼ��Ψ0
    readVOIDdata();                           //У��λ
    return now_data;
}
