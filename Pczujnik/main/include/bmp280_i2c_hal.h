

#ifndef MAIN_BMP280_I2C_HAL
#define MAIN_BMP280_I2C_HAL

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"

#define BMP280_ERR     -1
#define BMP280_OK      0x00

/**
 * @brief I2C init to be implemented by user based on hardware platform
 */
int16_t bmp280_i2c_hal_init();

/**
 * @brief I2C read to be implemented by user based on hardware platform
 */
int16_t bmp280_i2c_hal_read(uint8_t address, uint8_t *reg, uint8_t *data, uint16_t count);

/**
 * @brief I2C write to be implemented by user based on hardware platform
 */
int16_t bmp280_i2c_hal_write(uint8_t address, uint8_t *data, uint16_t count);

/**
 * @brief Milliseconds delay to be implemented by user based on hardware platform
 */
void bmp280_i2c_hal_ms_delay(uint32_t ms);

#ifdef __cplusplus
}
#endif

#endif /* MAIN_BMP280_I2C_HAL */
