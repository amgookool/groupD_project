#include "main.h"

static const char *TAG = "MAIN";
static const char *TAG_SERIAL = "SERIAL";
static const char *TAG_REPORTING = "REPORTING";
static const char *TAG_STORAGE = "STORAGE";
static const char *TAG_MEASUREMENT = "MEASUREMENT";

// SemaphoreHandle_t state_switcher = NULL;
QueueHandle_t serial_queue;

TaskHandle_t led_driver_handle;


uint16_t ads1115_read_value;
char *user = "User";

void app_main()
{
    // state_switcher = xSemaphoreCreateMutex();

    // Setup for Measurement Mode
    ADS1115_CONFIG_FIELDS config_fields;
    config_fields.OS = 0x00;        // No effect
    config_fields.MUX = 0x04;       // AINp = A0 & AINn = GND
    config_fields.PGA = 0x01;       // Gain - 4.096 V
    config_fields.MODE = 0x00;      // Continuous-conversion mode
    config_fields.DR = 0x04;        // 128 SPS
    config_fields.COMP_MODE = 0x00; // Traditional comparator
    config_fields.COMP_POL = 0x00;  // Active low
    config_fields.COMP_LAT = 0x00;  // Non-latching comparator. Alert pin doesn't latch when asserted
    config_fields.COMP_QUE = 0x02;  // Assert after 4 conversions

    // Setup for Reporting Mode
    serial_init(NULL);
    init_storage(NULL);

    ESP_LOGI(TAG_STORAGE, "Creating Files to store data\n");
    create_file(FILE_MEASUREMENTS);
    create_file(FILE_MAX_MEASUREMENTS);
    create_file(FILE_SET_INTERVALS);
    create_file(FILE_SET_FORMAT);

    // if (state_switcher != NULL)
    // {
    // xTaskCreate(reporting_task, "Reporting-Task", 32768, NULL, HIGHEST_PRIORITY, NULL);
    // xTaskCreate(measurement_init_task, "Measurement-Initialization-Task", 2056, (void *)&config_fields, HIGHEST_PRIORITY, NULL);
    // }
    // else
    // {
    measurement_init_task((void *)&config_fields);
    
    xTaskCreate(led_driver_function, "LED-Task", 8192, NULL, 3, NULL);
    // xTaskCreate(ads1115_read_task, "Read_ADS1115-Task", 4096, (void *)&ads1115_read_value, 5, NULL);
    // }
}

// void reporting_task(void *pvParams)
// {
//     uart_event_t serial_event;
//     uint8_t *dtmp = (uint8_t *)malloc(RX_BUFF_SIZE);
//     int read_val;

//     for (;;)
//     {
//         if (xSemaphoreTake(state_switcher, (TickType_t)15) == pdTRUE)
//         {
//             if (xQueueReceive(serial_queue, (void *)&serial_event, (portTickType)portMAX_DELAY))
//             {
//                 bzero(dtmp, RX_BUFF_SIZE);
//                 ESP_LOGI(TAG_SERIAL, "Serial Event: %d", UART_NUMBER);

//                 switch (serial_event.type)
//                 {
//                 case UART_DATA:
//                     uart_write_bytes(UART_NUMBER, (const char *)"---------------- Menu ------------------------------\n", serial_event.size);
//                     uart_write_bytes(UART_NUMBER, (const char *)"Please enter Integer to select setting\n", serial_event.size);
//                     uart_write_bytes(UART_NUMBER, (const char *)"1. Set frequency intervals for storing measurements\n", serial_event.size);
//                     uart_write_bytes(UART_NUMBER, (const char *)"2. Set maximum number of measurements to be stored\n", serial_event.size);
//                     uart_write_bytes(UART_NUMBER, (const char *)"3. Set the user being measured.\n", serial_event.size);
//                     uart_write_bytes(UART_NUMBER, (const char *)"4. View Stored measurements\n", serial_event.size);
//                     uart_write_bytes(UART_NUMBER, (const char *)"5. Enter Measurements Mode.\n", serial_event.size);

//                     uart_read_bytes(UART_NUMBER, dtmp, serial_event.size, portMAX_DELAY);
//                     ESP_LOGI(TAG_SERIAL, "The DATA Received:");
//                     uart_write_bytes(UART_NUMBER, (const char *)dtmp, serial_event.size);
//                     if ((int)dtmp == 1)
//                     {
//                         ESP_LOGI(TAG_SERIAL, "Enter the value in seconds\n");
//                         uart_read_bytes(UART_NUMBER, dtmp, serial_event.size, portMAX_DELAY);
//                         int interval = (int)dtmp;
//                         write_file(FILE_SET_INTERVALS, &interval);
//                     }
//                     else if ((int)dtmp == 2)
//                     {
//                         ESP_LOGI(TAG_SERIAL, "Enter the value in seconds\n");
//                         uart_read_bytes(UART_NUMBER, dtmp, serial_event.size, portMAX_DELAY);
//                         int max_num = (int)dtmp;
//                         write_file(FILE_MAX_MEASUREMENTS, &max_num);
//                     }
//                     else if ((int)dtmp == 3)
//                     {
//                         ESP_LOGI(TAG_SERIAL, "Enter the name of the person you are measuring.");
//                         uart_read_bytes(UART_NUMBER, dtmp, serial_event.size, portMAX_DELAY);
//                         user = (char *)dtmp;
//                         ESP_LOGI(TAG_SERIAL, "The User is set to: %s", user);
//                     }
//                     else if ((int)dtmp == 4)
//                     {
//                         ESP_LOGI(TAG_REPORTING, "Displaying Measurements.");
//                         read_file(FILE_MEASUREMENTS, &read_val);
//                     }
//                     else if ((int)dtmp == 5)
//                     {
//                         ESP_LOGI(TAG, "Entering into Measurement Mode.\n");
//                         xSemaphoreGive(state_switcher);
//                         vTaskDelay(500 / portTICK_PERIOD_MS); // delay for 0.5 seconds
//                         vTaskDelete(NULL);
//                     }
//                     break;

//                 case UART_FIFO_OVF:
//                     ESP_LOGI(TAG_SERIAL, "HW FIFO Overflow\n");
//                     uart_flush_input(UART_NUMBER);
//                     xQueueReset(serial_queue);
//                     break;

//                 case UART_BUFFER_FULL:
//                     ESP_LOGI(TAG_SERIAL, "Buffer is Full");
//                     uart_flush_input(UART_NUMBER);
//                     xQueueReset(serial_queue);
//                     break;

//                 case UART_PARITY_ERR:
//                     ESP_LOGI(TAG_SERIAL, "uart parity error");
//                     break;
//                 // Event of UART frame error
//                 case UART_FRAME_ERR:
//                     ESP_LOGI(TAG_SERIAL, "uart frame error");
//                     break;

//                 default:
//                     ESP_LOGI(TAG_SERIAL, "UART Event Type: %d", serial_event.type);
//                     break;
//                 }
//             }
//         }
//     }
//     free(dtmp);
//     dtmp = NULL;
//     // vTaskDelete(NULL);
// }

void measurement_init_task(void *pvParam)
{
    ESP_LOGI(TAG_MEASUREMENT, "Initializing GPIO for LED driving circuit and I2C Protocol for ADS1115\n");

    ADS1115_CONFIG_FIELDS *config_fields;
    config_fields = (ADS1115_CONFIG_FIELDS *)pvParam;
    ads1115_i2c_init(I2C_MASTER_NUM, config_fields);

    ESP_LOGI(TAG_MEASUREMENT, "Configuring RXD pin as PWM.\n");

    led_driver_init(NULL);
    
    // gpio_config_t io_conf;
    // io_conf.intr_type = GPIO_INTR_DISABLE;           // disable interrupt
    // io_conf.mode = GPIO_MODE_OUTPUT;                 // set as output mode
    // io_conf.pin_bit_mask = (1ULL << LED_DRIVER_PIN); // bit mask of the pin that you want to set
    // io_conf.pull_down_en = 0;                        // disable pull-down mode
    // io_conf.pull_up_en = 0;                          // disable pull-up mode
    // gpio_config(&io_conf);                           // configure GPIO with the given settings
    // while (1)
    // {
    //     if (xSemaphoreTake(state_switcher, (TickType_t)15) == pdTRUE)
    //     {
    //         ESP_LOGI(TAG_MEASUREMENT, "Initializing Measurement Mode");
    //         ADS1115_CONFIG_FIELDS *config_fields;
    //         config_fields = (ADS1115_CONFIG_FIELDS *)pvParam;
    //         init_measurement_mode(config_fields);
    //         break;
    //     }
    //     else
    //     {
    //         xSemaphoreGive(state_switcher);
    //         continue;
    //     }
    // }
    // state_switcher = NULL;
    // vTaskDelete(NULL);
}