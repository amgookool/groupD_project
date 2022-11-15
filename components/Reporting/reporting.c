#include "reporting.h"

static const char *TAG_SERIAL = "SERIAL";
static const char *TAG_REPORTING = "REPORTING";

static QueueHandle_t serial_queue;

void serial_init()
{
    uart_config_t serial_configs = {
        .baud_rate = 74880,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE};

    uart_param_config(UART_NUMBER, &serial_configs);
    uart_driver_install(UART_NUMBER, RX_BUFF_SIZE * 2, TX_BUFF_SIZE * 2, 150, &serial_queue, 0);
}

void serial_task()
{
    uart_event_t serial_event;
    uint8_t *dtmp = (uint8_t *)malloc(RX_BUFF_SIZE);

    for (;;)
    {
        if (xQueueReceive(serial_queue, (void *)&serial_event, (portTickType)portMAX_DELAY))
        {
            bzero(dtmp, RX_BUFF_SIZE);
            ESP_LOGI(TAG_SERIAL, "Serial Event: %d", UART_NUMBER);

            switch (serial_event.type)
            {
            case UART_DATA:
                ESP_LOGI(TAG_REPORTING,"Please enter Integer to select setting\n");
                uart_write_bytes(UART_NUMBER, (const char *)"1. Stored measurements\n", serial_event.size);
                uart_write_bytes(UART_NUMBER, (const char *)"2. Set frequency intervals for storing measurements\n", serial_event.size);
                uart_write_bytes(UART_NUMBER, (const char*)"3. Set maximum number of measurements to be stored\n",serial_event.size);
                uart_write_bytes(UART_NUMBER, (const char*)"4. Set the storeage format of the measured data\n",serial_event.size);

                uart_read_bytes(UART_NUMBER, dtmp, serial_event.size, portMAX_DELAY);
                ESP_LOGI(TAG_SERIAL,"The DATA REceived:\n");
                uart_write_bytes(UART_NUMBER, (const char*)dtmp, serial_event.size);
                break;

            case UART_FIFO_OVF:
                ESP_LOGI(TAG_SERIAL, "HW FIFO Overflow\n");
                uart_flush_input(UART_NUMBER);
                xQueueReset(serial_queue);
                break;

            case UART_BUFFER_FULL:
                ESP_LOGI(TAG_SERIAL, "Buffer is Full");
                uart_flush_input(UART_NUMBER);
                xQueueReset(serial_queue);
                break;

            case UART_PARITY_ERR:
                ESP_LOGI(TAG_SERIAL, "uart parity error");
                break;
            // Event of UART frame error
            case UART_FRAME_ERR:
                ESP_LOGI(TAG_SERIAL, "uart frame error");
                break;

            default:
                ESP_LOGI(TAG_SERIAL, "UART Event Type: %d", serial_event.type);
                break;
            }
        }
    }
    free(dtmp);
    dtmp = NULL;
    vTaskDelete(NULL);
}