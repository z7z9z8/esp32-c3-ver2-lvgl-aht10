/* DHT11 temperature sensor library
   Usage:

void temper_task(void *p)
{
    DHT11_init(GPIO_NUM_5); //�ȳ�ʼ��
    struct dht11_reading dht; //����ṹ��
    while (1)
    {
        dht = DHT11_read(); //������
        printf("Temp=%d, Humi=%d\r\n", dht.temperature, dht.humidity);
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}

   Sam Johnston
   October 2016
   This example code is in the Public Domain (or CC0 licensed, at your option.)
   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#ifndef DHT11_H_
#define DHT11_H_

#include "driver/gpio.h"

#define DHT11_PIN     5   //��ͨ���궨�壬�޸�����

#define DHT11_CLR     gpio_set_level(DHT11_PIN, 0)
#define DHT11_SET     gpio_set_level(DHT11_PIN, 1)
#define DHT11_IN      gpio_set_direction(DHT11_PIN, GPIO_MODE_INPUT)
#define DHT11_OUT     gpio_set_direction(DHT11_PIN, GPIO_MODE_OUTPUT)

struct dht11_reading
{
    // bool status;
    uint8_t temperature;
    uint8_t humidity;
};

//���ų�ʼ��
void DHT11_init(gpio_num_t);

//�����ݣ����ؽṹ��
struct dht11_reading DHT11_read();


// uint8_t DHT11_ReadTemHum(uint8_t *buf);
uint8_t DHT11_ReadValue(void);
void DHT11_Start(void);
void DelayUs(  uint32_t nCount);
#endif
