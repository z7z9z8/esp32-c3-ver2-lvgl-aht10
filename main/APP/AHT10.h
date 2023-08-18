#ifndef __AHT_H__
#define __AHT_H__

#include <i2cdev.h>
#include <stdbool.h>

#define AHT_I2C_ADDRESS_GND 0x38 //!< Device address when ADDR pin connected to GND
#define AHT_I2C_ADDRESS_VCC 0x39 //!< Device address when ADDR pin connected to VCC

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Device types
 */
typedef enum {
    AHT_TYPE_AHT1x = 0, //!< AHT10, AHT15
    AHT_TYPE_AHT20,     //!< AHT20
} aht_type_t;

/**
 * Device modes
 */
typedef enum {
    AHT_MODE_NORMAL = 0, //!< Normal mode
    AHT_MODE_CYCLE,      //!< Continuous measurements mode, undocumented
} aht_mode_t;

/**
 * Device descriptor
 */
typedef struct
{
    i2c_dev_t i2c_dev;
    aht_type_t type;
    aht_mode_t mode;
} aht_t;

/**
 * @brief Initialize device descriptor
 *
 * @param dev      Device descriptor
 * @param addr     Device I2C address
 * @param port     I2C port
 * @param sda_gpio SDA GPIO
 * @param scl_gpio SCL GPIO
 * @return `ESP_OK` on success
 */
esp_err_t aht_init_desc(aht_t *dev, uint8_t addr, i2c_port_t port, gpio_num_t sda_gpio, gpio_num_t scl_gpio);

/**
 * @brief Free device descriptor
 *
 * @param dev Device descriptor
 * @return `ESP_OK` on success
 */
esp_err_t aht_free_desc(aht_t *dev);

/**
 * @brief Init device
 *
 * @param dev Device descriptor
 * @return `ESP_OK` on success
 */
esp_err_t aht_init(aht_t *dev);

/**
 * @brief Soft reset device
 *
 * @param dev Device descriptor
 * @return `ESP_OK` on success
 */
esp_err_t aht_reset(aht_t *dev);

/**
 * @brief Get device status
 *
 * @param dev Device descriptor
 * @param[out] busy       Busy flag
 *                        - true: device currently measuring
 *                        - false: device in indle mode
 * @param[out] calibrated Calibration success flag
 *                        - true: sensor calibrated
 *                        - false: sensor not calibrated
 * @return `ESP_OK` on success
 */
esp_err_t aht_get_status(aht_t *dev, bool *busy, bool *calibrated);

/**
 * @brief Get temperature and relative humidity
 *
 * @param dev Device descriptor
 * @param[out] temperature Temperature, degrees Celsius
 * @param[out] humidity    Relative humidity, percents
 * @return `ESP_OK` on success
 */
esp_err_t aht_get_data(aht_t *dev, float *temperature, float *humidity);

#ifdef __cplusplus
}
#endif

/**@}*/

#endif /* __AHT_H__ */
