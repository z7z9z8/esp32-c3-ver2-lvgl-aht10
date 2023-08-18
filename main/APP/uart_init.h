/*
 * @Author: Dawn
 * @Date: 2022-06-22 14:51:35
 * @LastEditors: Dawn
 * @description:
 * @LastEditTime: 2022-06-22 16:48:30
 * Keep On Going
 * Copyright (c) 2022 by Dawn, All Rights Reserved.
 */
#ifndef UART_INIT_H
#define UART_INIT_H
#include <stdio.h>

//定义引脚
#define TXD_PIN (GPIO_NUM_21)
#define RXD_PIN (GPIO_NUM_20)

//设置缓冲去大小
#define RX_BUF_SIZE 1024


//串口初始化
void uart_init();

//获取数据,参数是开辟的缓冲区，该缓冲区在uart_init()里面定义过，为固定的 gps_raw
uint16_t uart_get_data(char *gps_raw);
#endif
