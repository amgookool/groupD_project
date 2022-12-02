#include "main.h"
#include "timers.h"

static const char *TAG = "MAIN";
static const char *TAG_SERIAL = "SERIAL";
static const char *TAG_REPORTING = "REPORTING";
static const char *TAG_STORAGE = "STORAGE";
static const char *TAG_MEASUREMENT = "MEASUREMENT";

// SemaphoreHandle_t state_switcher = NULL;
QueueHandle_t serial_queue;

TaskHandle_t led_driver_handle;
TaskHandle_t ads1115_read_task_handler;

static TimerHandle_t ads1115_timer_handler = NULL;

void vTimerCallback(TimerHandle_t xTimer)
{
    ESP_LOGI(TAG_MEASUREMENT, "The Timer has expired!");
}

char *user = "User";

void app_main()
{
    // Setup for Reporting Mode
    ESP_LOGI(TAG, "Initializing System into Reporting State\n");
    serial_init(NULL);
    init_storage(NULL);

    ESP_LOGI(TAG_STORAGE, "Creating Files to store data\n");
    create_file(FILE_MEASUREMENTS);
    create_file(FILE_MAX_MEASUREMENTS);
    create_file(FILE_SET_INTERVALS);
    create_file(FILE_SET_FORMAT);

    // Task function for UART
    // xTaskCreate(uart_event_task,"Serial-Task",2048,NULL,HIGHEST_PRIORITY,NULL);

    // Setup for Measurement Mode
    ESP_LOGI(TAG, "Initializing System into Measurement State\n");
    
    ADS1115_CONFIG_FIELDS config_fields;
    // MUX Option
    // 0x04 - AINp = A0 & AINn = GND
    // 0x05 - AINp = A1 & AINn = GND
    config_fields.OS = 0x00;        // No effect
    config_fields.MUX = 0x04;       // AINp = A1 & AINn = GND
    config_fields.PGA = 0x01;       // Gain - 4.096 V
    config_fields.MODE = 0x00;      // Continuous-conversion mode
    config_fields.DR = 0x04;        // 128 SPS
    config_fields.COMP_MODE = 0x00; // Traditional comparator
    config_fields.COMP_POL = 0x00;  // Active low
    config_fields.COMP_LAT = 0x00;  // Non-latching comparator. Alert pin doesn't latch when asserted
    config_fields.COMP_QUE = 0x02;  // Assert after 4 conversions
    ads1115_i2c_init(I2C_MASTER_NUM, &config_fields);
    
    led_driver_init(NULL);

    xTaskCreate(led_driver_function, "LED-Driving-Task", 4096, NULL, 2, NULL);
    xTaskCreate(ads1115_read_task,"ADS1115-Task",8192,NULL,1,NULL);

}

// void measurement_init_task(void *pvParam)
// {
//     ESP_LOGI(TAG_MEASUREMENT, "Initializing GPIO for LED driving circuit and I2C Protocol for ADS1115\n");

//     ADS1115_CONFIG_FIELDS *config_fields;
//     config_fields = (ADS1115_CONFIG_FIELDS *)pvParam;
//     ads1115_i2c_init(I2C_MASTER_NUM, config_fields);

//     ESP_LOGI(TAG_MEASUREMENT, "Configuring RXD pin as PWM.\n");

//     led_driver_init(NULL);

//     // gpio_config_t io_conf;
//     // io_conf.intr_type = GPIO_INTR_DISABLE;           // disable interrupt
//     // io_conf.mode = GPIO_MODE_OUTPUT;                 // set as output mode
//     // io_conf.pin_bit_mask = (1ULL << LED_DRIVER_PIN); // bit mask of the pin that you want to set
//     // io_conf.pull_down_en = 0;                        // disable pull-down mode
//     // io_conf.pull_up_en = 0;                          // disable pull-up mode
//     // gpio_config(&io_conf);                           // configure GPIO with the given settings
//     // while (1)
//     // {
//     //     if (xSemaphoreTake(state_switcher, (TickType_t)15) == pdTRUE)
//     //     {
//     //         ESP_LOGI(TAG_MEASUREMENT, "Initializing Measurement Mode");
//     //         ADS1115_CONFIG_FIELDS *config_fields;
//     //         config_fields = (ADS1115_CONFIG_FIELDS *)pvParam;
//     //         init_measurement_mode(config_fields);
//     //         break;
//     //     }
//     //     else
//     //     {
//     //         xSemaphoreGive(state_switcher);
//     //         continue;
//     //     }
//     // }
//     // state_switcher = NULL;
//     // vTaskDelete(NULL);
// }