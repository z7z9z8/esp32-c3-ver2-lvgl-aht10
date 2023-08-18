/*
 * @Author: Dawn
 * @Date: 2022-06-22 14:51:51
 * @LastEditors: Dawn
 * @description:
 * @LastEditTime: 2022-06-22 16:48:24
 * Keep On Going
 * Copyright (c) 2022 by Dawn, All Rights Reserved.
 */
#include "uart_init.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "esp_err.h"
#include "soc/soc_caps.h"

void uart_init()
{

    const uart_config_t uart_config = {
        .baud_rate = 9600,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };

    //´®¿ÚµÄÅäÖÃ
    ESP_ERROR_CHECK(uart_driver_install(UART_NUM_0, RX_BUF_SIZE * 2, 0, 0, NULL, 0));
    ESP_ERROR_CHECK(uart_param_config(UART_NUM_0, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(UART_NUM_0, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
}
uint16_t uart_get_data(char *gps_raw)
{
    uint16_t rxbyte = uart_read_bytes(UART_NUM_0, gps_raw, RX_BUF_SIZE, 1000 / portTICK_PERIOD_MS);
    return rxbyte;
}
