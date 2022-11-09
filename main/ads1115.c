#include "include/ads1115.h"

static const char *TAG = "ADS1115";

static esp_err_t i2c_ads1115_init(i2c_port_t i2c_num, ADS1115_CONFIG_FIELDS *configs)
{
    vTaskDelay(200 / portTICK_RATE_MS);

    ESP_LOGI(TAG, "Initializing ESP8266 GPIO pins for I2C protocol");
    int i2c_master_port = I2C_MASTER_NUM;
    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = I2C_MASTER_SDA_IO;
    conf.sda_pullup_en = 1;
    conf.scl_io_num = I2C_MASTER_SCL_IO;
    conf.scl_pullup_en = 1;
    conf.clk_stretch_tick = 300; // 300 ticks, Clock stretch is about 300us
    ESP_ERROR_CHECK(i2c_driver_install(i2c_master_port, conf.mode));
    ESP_ERROR_CHECK(i2c_param_config(i2c_master_port, &conf));

    get_16bit_configs(&configs);

    ESP_LOGI(TAG, "The configuration field is: %d\n", (int)configs->configuration);

    ESP_ERROR_CHECK(ads1115_write_data(i2c_num, ADS1115_CONFIG_REG, configs->configuration));

    return ESP_OK;
}