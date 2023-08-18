#if 1

#include "lv_port_indev.h"
#include "iot_knob.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "sdkconfig.h"
#include "iot_knob.h"

#define TAG "encode"

static bool encoder_read(lv_indev_drv_t *indev_drv, lv_indev_data_t *data);

esp_err_t bsp_button_init(const bsp_button_t btn)
{
    const gpio_config_t button_io_config = {
        .pin_bit_mask = BIT64(btn),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE};
    return gpio_config(&button_io_config);
}

// 获取按键状态
bool bsp_button_get(const bsp_button_t btn)
{
    return !(bool)gpio_get_level(btn);
}
extern lv_obj_t *btn1;

static void knob_left_cb(void *arg, void *data)
{
    uint16_t count=iot_knob_get_count_value((knob_handle_t)arg);
    lv_obj_t *label=lv_obj_get_child(btn1,0);
    lv_label_set_text_fmt(label,"%d",count);

    printf("KNOB_LEFT Count is %d\n",count);
}

static void knob_right_cb(void *arg, void *data)
{
    uint16_t count=iot_knob_get_count_value((knob_handle_t)arg);
    lv_obj_t *label=lv_obj_get_child(btn1,0);
    lv_label_set_text_fmt(label,"%d",count);

    printf("KNOB_LEFT Count is %d\n",count);
}

lv_indev_t *lv_port_indev_init(void)
{
    static lv_indev_drv_t indev_drv;
    lv_indev_t *indev_encoder;

    knob_handle_t knob_handle;

    bsp_button_init(BSP_BTN_PRESS);

    knob_config_t *cfg = calloc(1, sizeof(knob_config_t));
    cfg->default_direction = 0;
    cfg->gpio_encoder_a = BSP_ENCODER_A;
    cfg->gpio_encoder_b = BSP_ENCODER_B;
    knob_handle = iot_knob_create(cfg);

    iot_knob_register_cb(knob_handle, KNOB_LEFT, knob_left_cb, NULL);
    iot_knob_register_cb(knob_handle, KNOB_RIGHT, knob_right_cb, NULL);

    /*Register a encoder input device*/
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_ENCODER;
    indev_drv.read_cb = encoder_read;
    indev_drv.user_data = knob_handle;
    indev_encoder = lv_indev_drv_register(&indev_drv);

    return indev_encoder;
}

static bool encoder_read(lv_indev_drv_t *indev_drv, lv_indev_data_t *data)
{
    static int32_t last_v = 0;

    assert(indev_drv);
    assert(indev_drv->user_data);
    knob_handle_t knob_handle = (knob_handle_t)indev_drv->user_data;

    int32_t invd = iot_knob_get_count_value(knob_handle);
    knob_event_t event = iot_knob_get_event(knob_handle);

    //-+2 原因是因为这个编码器顿挫感的原因
    if (((last_v-invd)>1)||((invd-last_v)>1))
    {
        last_v = invd;
        data->enc_diff = (KNOB_LEFT == event) ? (-1) : ((KNOB_RIGHT == event) ? (1) : (0));
    }
    else
    {
        data->enc_diff = 0;
    }
    data->state = (true == bsp_button_get(BSP_BTN_PRESS)) ? LV_BTN_STATE_PRESSED : LV_BTN_STATE_RELEASED;
    return false;
}
#else /* Enable this file at the top */
s
#endif
