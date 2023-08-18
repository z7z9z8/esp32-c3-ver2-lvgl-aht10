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

//��������
#define TXD_PIN (GPIO_NUM_21)
#define RXD_PIN (GPIO_NUM_20)

//���û���ȥ��С
#define RX_BUF_SIZE 1024


//���ڳ�ʼ��
void uart_init();

//��ȡ����,�����ǿ��ٵĻ��������û�������uart_init()���涨�����Ϊ�̶��� gps_raw
uint16_t uart_get_data(char *gps_raw);
#endif
