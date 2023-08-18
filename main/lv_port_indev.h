#if 1

#ifndef LV_PORT_INDEV_TEMPL_H
#define LV_PORT_INDEV_TEMPL_H

#include "lvgl.h"
#include "driver/gpio.h"


/* Buttons */
typedef enum {
    BSP_BTN_PRESS = GPIO_NUM_3,
} bsp_button_t;

//编码器
#define BSP_ENCODER_A         (GPIO_NUM_4)
#define BSP_ENCODER_B         (GPIO_NUM_5)

//编码器输入初始化
//返回一个输入设备的指针
lv_indev_t *lv_port_indev_init(void);

//确认按键初始化
esp_err_t bsp_button_init(const bsp_button_t btn);


#ifdef __cplusplus
extern "C" {
#endif


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*LV_PORT_INDEV_TEMPL_H*/

#endif /*Disable/Enable content*/
