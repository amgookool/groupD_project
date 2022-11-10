#include "ads1115.h"
#define ADDR_PIN ADS1115_ADDR_GND

static const char *TAG = "ADS1115:";

static esp_err_t i2c_ads1115_init(i2c_port_t i2c_num, ADS1115_CONFIG_FIELDS* configs)
{
    ESP_LOGI(TAG, "Initializing ESP8266 for I2C protocol\n");
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

    ESP_LOGI(TAG,"Initializing ADS1115\n");
    vTaskDelay(200 / portTICK_RATE_MS);
    
    get_16bit_config(configs);
    ESP_LOGI(TAG,"Configuration: %u\n",configs->configuration);

    // writing to configuration register 
    ESP_ERROR_CHECK(ads1115_write_data(i2c_num,ADS1115_CONFIG_REG,configs->configuration));
    return ESP_OK;
}

static void get_16bit_config(ADS1115_CONFIG_FIELDS *config)
{
    uint16_t data;

    data = (config->OS << 3) | config->MUX;
    data = (data << 3) | config->PGA;
    data = (data << 1) | config->MODE;
    data = (data << 3) | config->DR;
    data = (data << 1) | config->COMP_MODE;
    data = (data << 1) | config->COMP_POL;
    data = (data << 1) | config->COMP_LAT;
    data = (data << 2) | config->COMP_QUE;
    config->configuration = data;
}

static esp_err_t ads1115_write_bytes(i2c_port_t i2c_num, uint8_t reg_addr, uint8_t *data, uint16_t data_len)
{
    esp_err_t ret;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);                                                         // start condition
    i2c_master_write_byte(cmd, (ADDR_PIN << 1) | WRITE_BIT, ACK_CHECK_EN); // address frame + read/write bit
    i2c_master_write_byte(cmd, reg_addr, ACK_CHECK_EN);                            // accessing register
    i2c_master_write(cmd, data, data_len, ACK_CHECK_EN);                           // writing to register
    i2c_master_stop(cmd);                                                          // stop condition
    ret = i2c_master_cmd_begin(i2c_num, cmd, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(cmd);
    return ret;
}

static esp_err_t ads1115_write_data(i2c_port_t i2c_num, uint8_t reg_addr, uint16_t data)
{
    esp_err_t ret;
    uint8_t write_buff[2];

    write_buff[0] = (data >> 8) & 0xFF;
    write_buff[1] = (data >> 0) & 0xFF;

    ret = ads1115_write_bytes(i2c_num, reg_addr, write_buff, 2);

    return ret;
}

static esp_err_t ads1115_read_bytes(i2c_port_t i2c_num, uint8_t reg_addr, uint8_t *data, uint16_t data_len)
{
    esp_err_t ret;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (ADDR_PIN << 1) | WRITE_BIT, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, reg_addr, ACK_CHECK_EN);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(i2c_num, cmd, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(cmd);
    if (ret != ESP_OK)
    {
        ESP_LOGI(TAG, "Could not read bytes from ADS1115!");
        return ret;
    }
    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (ADDR_PIN << 1) | READ_BIT, ACK_CHECK_EN);
    i2c_master_read(cmd, data, data_len, LAST_NACK_VAL);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(i2c_num, cmd, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(cmd);

    // for (int i = 0; i < data_len; i++)
    // {
    //     ESP_LOGI(TAG, "Byte:%d: %d", i, data[i]);
    // }
    return ret;
}

static esp_err_t ads1115_read_data(i2c_port_t i2c_num, uint8_t reg_addr, uint16_t *data)
{
    esp_err_t ret;
    uint16_t sensor_data = 0;
    uint8_t read_buff[2];

    ret = ads1115_read_bytes(i2c_num, reg_addr, read_buff, 2);
    sensor_data = (read_buff[0] << 8) | read_buff[1];
    *data = sensor_data;
    return ret;
}

void ads1115_task(void *params)
{
    ADS1115_CONFIG_FIELDS *config_fields;
    config_fields = (ADS1115_CONFIG_FIELDS *)params;

    uint16_t sensor_data;
    esp_err_t ret;
    
    ret = i2c_ads1115_init(I2C_MASTER_NUM, config_fields);
    if (ret == ESP_OK)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
        ESP_LOGI(TAG, "Succussfully Initialized ADS1115 and I2C Protocol\n");
    }
    else
    {
        i2c_driver_delete(I2C_MASTER_NUM);
        ESP_LOGI(TAG, "Error: %s", (char *)ret);
    }
    while (1)
    {
        ret = (ads1115_read_data(I2C_MASTER_NUM, ADS1115_CONV_REG, &sensor_data));
        if (ret == ESP_OK)
        {
            ESP_LOGI(TAG, "Successfully read ADS1115...\n");
            ESP_LOGI(TAG, "Sensor Data: %d\n", (int)sensor_data);
            
            double voltage = (double)sensor_data * 1.25e-4;
            ESP_LOGI(TAG, "Voltage: %d.%d V\n", (uint16_t)voltage, (uint16_t)(voltage * 100) % 100);
            
            vTaskDelay(2000 / portTICK_PERIOD_MS);
        }
        else
        {
            ESP_LOGI(TAG, "Could not read the ADS1115\nError: %s", (char *)ret);
            vTaskDelete(NULL);
        }
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}