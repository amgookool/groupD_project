#include "measurement.h"
#define ADDR_PIN ADS1115_ADDR_GND

// PWM period 1000us(1khz)
// 1us = 1hz
#define PWM_PERIOD (10000) // 10khz
#define PWM_DUTY_CYCLE 5000

#define MAX_VALUES 2000

static const char *TAG_MEASURE = "MEASUREMENT-MODE";
static const char *TAG_ADS1115 = "ADS1115";
static const char *TAG_LED = "LED-DRIVER";
static const char *TAG_BUZZER = "BUZZER";

uint16_t red_light_data[MAX_VALUES];
uint16_t ir_light_data[MAX_VALUES];
double SPO2_R_value;

int i = 0;
int j = 0;
int k = 0;

bool is_crest(uint16_t data_arr[], uint16_t number, int k, int i, int j)
{
    if (i >= 0 && data_arr[i] > number)
    {
        return false;
    }
    if (j < k && data_arr[j] > number)
    {
        return false;
    }
    return true;
}

bool is_trough(uint16_t data_arr[], uint16_t number, int k, int i, int j)
{
    if (i >= 0 && data_arr[i] < number)
    {
        return false;
    }
    if (j < k && data_arr[j] < number)
    {
        return false;
    }
    return true;
}

void calc_heart_rate_and_SPO2(uint16_t data_arr[], int k, double *pulse_rate, double *spo2_value)
{
    double crest = 0.0;
    double trough = 0.0;
    double bpm = 0.0;

    double trough_avg = 0.0;
    double crest_avg = 0.0;

    double spo2 = 0.0;
    double base = 0.0;
    double peak = 0.0;

    double min = data_arr[0];
    double max = data_arr[0];

    for (int i = 0; i < k; i++)
    {
        if (max > data_arr[i]) //&& arr[i]<500 && arr[i]>100)
        {
            max = data_arr[i];
        }

        if (min < data_arr[i]) //&& arr[i]<500 && arr[i]>100)
        {
            min = data_arr[i];
        }

        if (is_crest(data_arr, k, data_arr[i], i - 1, i + 1))
        {
            crest++;
            crest_avg = crest_avg + data_arr[i];
        }

        if (is_trough(data_arr, k, data_arr[i], i - 1, i + 1))
        {
            trough++;
            trough_avg = trough_avg + data_arr[i];
        }
    }
    bpm = (crest / 1000) * 60;
    peak = crest_avg / crest;
    base = trough_avg / trough;
    spo2 = peak / base;
    *pulse_rate = bpm;
    *spo2_value = spo2;
}

int SPO2_lookup_table(double R_value)
{
    if (R_value >= 0.400 && R_value < 0.440)
    {
        return 100;
    }
    if (R_value >= 0.450 && R_value < 0.490)
    {
        return 99;
    }
    if (R_value >= 0.500 && R_value < 0.550)
    {
        return 98;
    }
    if (R_value == 0.551 && R_value < 0.559)
    {
        return 97;
    }
    if (R_value >= 0.600 && R_value < 0.650)
    {
        return 96;
    }
    if (R_value >= 0.660 && R_value < 0.710)
    {
        return 95;
    }
    if (R_value >= 0.720 && R_value < 0.800)
    {
        return 94;
    }
    if (R_value >= 0.810 && R_value < 0.860)
    {
        return 93;
    }
    if (R_value >= 0.870 && R_value < 0.920)
    {
        return 92;
    }
    if (R_value >= 0.930 && R_value < 0.990)
    {
        return 91;
    }
    if (R_value >= 1.000 && R_value < 1.500)
    {
        return 90;
    }
    else
    {
        return 0;
    }
}

void ads1115_read_task(void *pvParam)
{
    uint16_t sensor_data;
    esp_err_t ret;
    double ir_light_spo2 = 0.0;
    double red_light_spo2 = 0.0;
    double spo2 = 0.0;
    double R_value = 0.0;

    while (1)
    {
        ret = ads1115_read_data(I2C_MASTER_NUM, ADS1115_CONV_REG, &sensor_data);
        if (ret == ESP_OK)
        {
            if (k % 2)
            {
                ir_light_data[i] = (uint16_t)sensor_data;
                i++;
                k++;
                // printf("IR Light: %d\n",(uint16_t)ir_light_data[i]);
            }
            else
            {
                red_light_data[j] = (uint16_t)sensor_data;
                j++;
                k++;
                // printf("Red Light: %d\n",(uint16_t)red_light_data[i]);
            }
            vTaskDelay(10 / portTICK_PERIOD_MS);

            if (i == MAX_VALUES)
            {
                double pulse_rate_val;
                calc_heart_rate_and_SPO2(red_light_data, sizeof(red_light_data), &pulse_rate_val, &red_light_spo2);
                calc_heart_rate_and_SPO2(ir_light_data, sizeof(ir_light_data), &pulse_rate_val, &ir_light_spo2);
                R_value = red_light_spo2 / ir_light_spo2;
                SPO2_R_value = R_value;
                int spo2_percent = SPO2_lookup_table(R_value);
                ESP_LOGI(TAG_MEASURE, "The BPM is: %d\n", (uint16_t)pulse_rate_val);
                ESP_LOGI(TAG_MEASURE, "The SPO2 Value is: %d\n", (int)spo2_percent);
                // vTaskDelete(NULL);
                vTaskDelay(5000 / portTICK_PERIOD_MS);
            }
            
        }
        else
        {
            ESP_LOGI(TAG_MEASURE, "Could not read ADS1115\n");
            vTaskDelete(NULL);
        }
    }
}

void led_driver_init(void *pvParam)
{
    // uint32_t duty = PWM_DUTY_CYCLE;
    // uint32_t pin = LED_DRIVER_PIN;
    // float phase = 0;
    // pwm_init(PWM_PERIOD, &duty, 3, &pin);
    // pwm_set_phase(3, phase);
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;           // disable interrupt
    io_conf.mode = GPIO_MODE_OUTPUT;                 // set as output mode
    io_conf.pin_bit_mask = (1ULL << LED_DRIVER_PIN); // bit mask of the pin that you want to set
    io_conf.pull_down_en = 0;                        // disable pull-down mode
    io_conf.pull_up_en = 0;                          // disable pull-up mode
    gpio_config(&io_conf);                           // configure GPIO with the given settings
}

void led_driver_function(void *pvParam)
{
    // pwm_start();
    // uint32_t period;
    // while (1)
    // {
    //     pwm_get_period(&period);
    //     ESP_LOGI(TAG_LED, "LED state: %d\n", period);
    //     // Visible LED is 660 nm
    //     // IR LED is 950 nm
    //     // if pin is high -> IR emitter is on
    //     // if pin is low -> Visible light emitter is on
    //     // active_wait(1780);
    //     // gpio_set_level(LED_DRIVER_PIN, 1);
    //     // vTaskDelay(1780 / portTICK_PERIOD_MS);
    //     // gpio_set_level(LED_DRIVER_PIN, 0);
    //     // // active_wait(1780);
    //     // vTaskDelay(420 / portTICK_PERIOD_MS);
    // }
    while (1)
    {
        gpio_set_level(LED_DRIVER_PIN, 1);
        vTaskDelay(10 / portTICK_RATE_MS);
        gpio_set_level(LED_DRIVER_PIN, 0);
        vTaskDelay(10 / portTICK_RATE_MS);
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

void sound_buzzer(void *pvParam)
{
    gpio_set_level(ALARM_BUZZER_PIN, 1);
    vTaskDelay(1000 / portMAX_DELAY);
    gpio_set_level(ALARM_BUZZER_PIN, 0);
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
