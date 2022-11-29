#include "measurement.h"
#define ADDR_PIN ADS1115_ADDR_GND

// PWM period 1000us(1khz)
// 1us = 1hz
#define PWM_PERIOD (440) // 2khz

static const char *TAG_MEASURE = "MEASUREMENT-MODE";
static const char *TAG_ADS1115 = "ADS1115";
static const char *TAG_LED = "LED-DRIVER";
static const char *TAG_BUZZER = "BUZZER";



void ads1115_read_task(void* pvParam)
{
    uint16_t sensor_data;
    sensor_data = 0x04;
    pvParam = &sensor_data;
    ESP_LOGI(TAG_ADS1115,"%d\n",(int)pvParam);

}



void led_driver_init(void *pvParam){
    uint32_t duty = 220;
    float phase = 0;
    pwm_init(PWM_PERIOD,duty,1,LED_DRIVER_PIN);
    pwm_set_phase(0x03,phase);
}


void led_driver_function(void *pvParam)
{
    int16_t count = 0;
    
    pwm_start();
    uint32_t period;
    while (1)
    {
        pwm_get_period(&period);
        ESP_LOGI(TAG_LED,"LED state: %d\n",period);
        // Visible LED is 660 nm
        // IR LED is 950 nm
        // if pin is high -> IR emitter is on
        // if pin is low -> Visible light emitter is on
        // active_wait(1780);
        // gpio_set_level(LED_DRIVER_PIN, 1);
        // vTaskDelay(1780 / portTICK_PERIOD_MS);
        // gpio_set_level(LED_DRIVER_PIN, 0);
        // // active_wait(1780);
        // vTaskDelay(420 / portTICK_PERIOD_MS);
    }
}


// init function for GPIO interface for buzzer
void gpio_pin2_init(void *pvParam)
{
    ESP_LOGI(TAG_BUZZER, "Configuring GPIO2 pin for buzzer.\n");
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;             // disable interrupt
    io_conf.mode = GPIO_MODE_OUTPUT;                   // set as output mode
    io_conf.pin_bit_mask = (1ULL << ALARM_BUZZER_PIN); // bit mask of the pin that you want to set
    io_conf.pull_down_en = 0;                          // disable pull-down mode
    io_conf.pull_up_en = 0;                            // disable pull-up mode
    gpio_config(&io_conf);                             // configure GPIO with the given settings
}

// Init function for alert pin
void gpio_pin0_init(void *pvParam)
{
    ESP_LOGI(TAG_ADS1115, "Configuring GPIO0 pin for alert on ADS1115.\n");
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_ANYEDGE;          // disable interrupt
    io_conf.mode = GPIO_MODE_INPUT;                 // set as output mode
    io_conf.pin_bit_mask = (1ULL << I2C_ALERT_PIN); // bit mask of the pin that you want to set
    io_conf.pull_down_en = 0;                       // disable pull-down mode
    io_conf.pull_up_en = 0;                         // disable pull-up mode
    gpio_config(&io_conf);                          // configure GPIO with the given settings
}

void active_wait(int wait_time_usecs)
{
    struct timeval present_tv; // Structure returned by gettimeofday function
    gettimeofday(&present_tv, NULL);

    int64_t start_time = (int64_t)present_tv.tv_sec * 1000000L + (int64_t)present_tv.tv_usec; // time since last reset in microseconds

    while (1)
    {
        gettimeofday(&present_tv, NULL);
        int64_t current_time = (int64_t)present_tv.tv_sec * 1000000L + (int64_t)present_tv.tv_usec;

        // check if 0.5 seconds have elapsed
        if ((current_time - start_time) >= wait_time_usecs)
        {
            break;
        }
    }
}

esp_err_t ads1115_i2c_init(i2c_port_t i2c_num, ADS1115_CONFIG_FIELDS *configs)
{
    ESP_LOGI(TAG_ADS1115, "Initializing ESP8266 for I2C protocol\n");
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

    ESP_LOGI(TAG_ADS1115, "Initializing ADS1115\n");
    vTaskDelay(200 / portTICK_RATE_MS);

    ads1115_16bit_config(configs);
    ESP_LOGI(TAG_ADS1115, "Configuration: %u\n", configs->configuration);

    // writing to configuration register
    ESP_ERROR_CHECK(ads1115_write_data(i2c_num, ADS1115_CONFIG_REG, configs->configuration));
    return ESP_OK;
}

esp_err_t ads1115_16bit_config(ADS1115_CONFIG_FIELDS *config)
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

    return ESP_OK;
}

esp_err_t ads1115_write_bytes(i2c_port_t i2c_num, uint8_t reg_addr, uint8_t *data, uint16_t data_len)
{
    esp_err_t ret;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);                                                 // start condition
    i2c_master_write_byte(cmd, (ADDR_PIN << 1) | WRITE_BIT, ACK_CHECK_EN); // address frame + read/write bit
    i2c_master_write_byte(cmd, reg_addr, ACK_CHECK_EN);                    // accessing register
    i2c_master_write(cmd, data, data_len, ACK_CHECK_EN);                   // writing to register
    i2c_master_stop(cmd);                                                  // stop condition
    ret = i2c_master_cmd_begin(i2c_num, cmd, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(cmd);
    return ret;
}

esp_err_t ads1115_write_data(i2c_port_t i2c_num, uint8_t reg_addr, uint16_t data)
{
    esp_err_t ret;
    uint8_t write_buff[2];

    write_buff[0] = (data >> 8) & 0xFF;
    write_buff[1] = (data >> 0) & 0xFF;

    ret = ads1115_write_bytes(i2c_num, reg_addr, write_buff, 2);

    return ret;
}

esp_err_t ads1115_read_bytes(i2c_port_t i2c_num, uint8_t reg_addr, uint8_t *data, uint16_t data_len)
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
        ESP_LOGE(TAG_ADS1115, "Could not read bytes from ADS1115!");
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

esp_err_t ads1115_read_data(i2c_port_t i2c_num, uint8_t reg_addr, uint16_t *data)
{
    esp_err_t ret;
    uint16_t sensor_data = 0;
    uint8_t read_buff[2];

    ret = ads1115_read_bytes(i2c_num, reg_addr, read_buff, 2);
    sensor_data = (read_buff[0] << 8) | read_buff[1];
    *data = sensor_data;
    return ret;
}
