

#include "bmp280_i2c_hal.h"

//Hardware Specific Components
#include "driver/i2c.h"

//I2C User Defines
//#define I2C_MASTER_SCL_IO           CONFIG_I2C_MASTER_SCL      /*!< GPIO number used for I2C master clock */
//#define I2C_MASTER_SDA_IO           CONFIG_I2C_MASTER_SDA      /*!< GPIO number used for I2C master data  */
#define I2C_MASTER_SCL_IO           5      /*!< GPIO number used for I2C master clock */
#define I2C_MASTER_SDA_IO           4      /*!< GPIO number used for I2C master data  */
#define I2C_MASTER_NUM              0                          /*!< I2C master i2c port number, the number of i2c peripheral interfaces available will depend on the chip */
#define I2C_MASTER_FREQ_HZ          100000                     /*!< I2C master clock frequency */
#define I2C_MASTER_TX_BUF_DISABLE   0                          /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE   0                          /*!< I2C master doesn't need buffer */
#define I2C_MASTER_TIMEOUT_MS       1000

int16_t bmp280_i2c_hal_init()
{
    int16_t err = BMP280_OK;

    //User implementation here

    int i2c_master_port = I2C_MASTER_NUM;

    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,    //Disable this if I2C lines have pull up resistor in place
        .scl_pullup_en = GPIO_PULLUP_ENABLE,    //Disable this if I2C lines have pull up resistor in place
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };

    i2c_param_config(i2c_master_port, &conf);

    err = i2c_driver_install(i2c_master_port, conf.mode, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0);

    return err == BMP280_OK ? BMP280_OK :  BMP280_ERR;
}

int16_t bmp280_i2c_hal_read(uint8_t address, uint8_t *reg, uint8_t *data, uint16_t count)
{
    int16_t err = BMP280_OK;

    //User implementation here

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, address << 1 | I2C_MASTER_WRITE, 1);
	i2c_master_write(cmd, reg, 1, true);
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, address << 1 | I2C_MASTER_READ, 1);
	i2c_master_read(cmd, data, count, I2C_MASTER_LAST_NACK);
	i2c_master_stop(cmd);
	err = i2c_master_cmd_begin(I2C_NUM_0, cmd, I2C_MASTER_TIMEOUT_MS / portTICK_RATE_MS);
	i2c_cmd_link_delete(cmd);

    return err == BMP280_OK ? BMP280_OK :  BMP280_ERR;
}

int16_t bmp280_i2c_hal_write(uint8_t address, uint8_t *data, uint16_t count)
{
    int16_t err = BMP280_OK;

    //User implementation here

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (address << 1) | I2C_MASTER_WRITE, 1);
    i2c_master_write(cmd, data, count, 1);
    i2c_master_stop(cmd);
    err = i2c_master_cmd_begin(I2C_NUM_0, cmd, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);

    return err == BMP280_OK ? BMP280_OK :  BMP280_ERR;
}

void bmp280_i2c_hal_ms_delay(uint32_t ms) {

    //User implementation here

    vTaskDelay(pdMS_TO_TICKS(ms));
}
