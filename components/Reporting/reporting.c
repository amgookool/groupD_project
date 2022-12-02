#include "reporting.h"

static const char *TAG_SERIAL = "SERIAL";
static const char *TAG_STORAGE = "Storage";
static const char *TAG_REPORTING = "Reporting";

static QueueHandle_t serial_queue;

void serial_init(void *pvParam)
{
    ESP_LOGI(TAG_SERIAL, "Initializing UART configurations.\n");
    uart_config_t serial_configs = {
        .baud_rate = 74880,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE};

    uart_param_config(UART_NUMBER, &serial_configs);
    uart_driver_install(UART_NUMBER, BUF_SIZE * 2, BUF_SIZE * 2, 100, &serial_queue, 0);
}

void display_menu(void *pvParam)
{
    printf("----------------- MENU -----------------\n");
    printf("Please enter Integer to select setting\n\n");
    printf("1. Set frequency intervals for storing measurements\n");
    printf("2. Set maximum number of measurements to be stored\n");
    printf("3. Set the user being measured.\n");
    printf("4. View Stored measurements\n");
    printf("5. Enter Measurement Mode\n");
}

static void uart_event_task(void *pvParameters)
{
    uart_event_t event;
    uint8_t *dtmp = (uint8_t *)malloc(RD_BUF_SIZE);
    const char *char_option = (const char *)malloc(8);
    int len = 0;
    display_menu(NULL);

    for (;;)
    {
        // Waiting for UART event.
        if (xQueueReceive(serial_queue, (void *)&event, (portTickType)portMAX_DELAY))
        {
            bzero(dtmp, RD_BUF_SIZE);

            switch (event.type)
            {
            // Event of UART receiving data
            // We'd better handler data event fast, there would be much more data events than
            // other types of events. If we take too much time on data event, the queue might be full.
            case UART_DATA:
                // Read the data from serial connection
                uart_read_bytes(UART_NUMBER, dtmp, event.size, portMAX_DELAY);
                char_option = (const char *)dtmp;

                if (strcmp(char_option, "1") == 0)
                {
                    uart_flush_input(UART_NUMBER);
                    xQueueReset(serial_queue);
                    ESP_LOGI(TAG_SERIAL, "Enter Frequency Interval Value\n");
                    uart_read_bytes(UART_NUMBER, dtmp, BUF_SIZE, portMAX_DELAY);
                    ESP_LOGI(TAG_SERIAL, "The interval set is:");
                    uart_write_bytes(UART_NUMBER, (const char *)dtmp, sizeof(*dtmp));
                }
                else if (strcmp(char_option, "2") == 0)
                {
                    uart_flush_input(UART_NUMBER);
                    xQueueReset(serial_queue);
                    ESP_LOGI(TAG_SERIAL, "Enter Value for Maximum number of measurements\n");
                    int len = uart_read_bytes(UART_NUMBER, dtmp, BUF_SIZE, portMAX_DELAY);
                    ESP_LOGI(TAG_SERIAL, "The maximum number of measurements is:");
                    uart_write_bytes(UART_NUMBER, (const char *)dtmp, len);
                }
                else if (strcmp(char_option, "3") == 0)
                {
                    uart_flush_input(UART_NUMBER);
                    xQueueReset(serial_queue);
                    ESP_LOGI(TAG_SERIAL, "Enter Name of user being measured\n");
                    int len = uart_read_bytes(UART_NUMBER, dtmp, BUF_SIZE, portMAX_DELAY);
                    ESP_LOGI(TAG_SERIAL, "The User being measured is:");
                    uart_write_bytes(UART_NUMBER, (const char *)dtmp, len);
                }
                else if (strcmp(char_option, "4") == 0)
                {
                    uart_flush_input(UART_NUMBER);
                    xQueueReset(serial_queue);
                    ESP_LOGI(TAG_REPORTING, "Stored Measurements\n");
                }
                else if (strcmp(char_option, "5") == 0)
                {
                    free(dtmp);
                    dtmp = NULL;
                    vTaskDelete(NULL);
                }
                break;

            // Event of HW FIFO overflow detected
            case UART_FIFO_OVF:
                ESP_LOGI(TAG_SERIAL, "hw fifo overflow");
                // If fifo overflow happened, you should consider adding flow control for your application.
                // The ISR has already reset the rx FIFO,
                // As an example, we directly flush the rx buffer here in order to read more data.
                uart_flush_input(UART_NUMBER);
                xQueueReset(serial_queue);
                break;

            // Event of UART ring buffer full
            case UART_BUFFER_FULL:
                ESP_LOGI(TAG_SERIAL, "ring buffer full");
                // If buffer full happened, you should consider increasing your buffer size
                // As an example, we directly flush the rx buffer here in order to read more data.
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

            // Others
            default:
                ESP_LOGI(TAG_SERIAL, "uart event type: %d", event.type);
                break;
            }
        }
    }

    free(dtmp);
    dtmp = NULL;
    vTaskDelete(NULL);
}

void init_storage(void *pvParam)
{
    ESP_LOGI(TAG_STORAGE, "Initializing SPIFFS.\n");

    esp_err_t ret;
    size_t total_bytes = 0, used_bytes = 0;

    esp_vfs_spiffs_conf_t config = {
        .base_path = PATH_STORAGE,
        .partition_label = NULL, // default to "Storage"
        .max_files = MAX_NUM_FILES,
        .format_if_mount_failed = true};

    ret = esp_vfs_spiffs_register(&config);

    if (ret != ESP_OK)
    {
        if (ret == ESP_FAIL)
        {
            ESP_LOGE(TAG_STORAGE, "Failed to mount or format filesystem.\n");
        }
        else if (ret == ESP_ERR_NOT_FOUND)
        {
            ESP_LOGE(TAG_STORAGE, "Failed to find SPIFFS partition.\n");
        }
        else
        {
            ESP_LOGE(TAG_STORAGE, "Failed to initialize SPIFFS: (%s)\n", esp_err_to_name(ret));
        }
        return;
    }

    ret = esp_spiffs_info(NULL, &total_bytes, &used_bytes);

    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG_STORAGE, "Failed to get SPIFFS partition information: (%s)\n", esp_err_to_name(ret));
    }
    else
    {
        ESP_LOGI(TAG_STORAGE, "Successfully Initialized SPIFFs\n");
        ESP_LOGI(TAG_STORAGE, "Partition size: total: %d, used: %d\n", total_bytes, used_bytes);
    }
}

void create_file(const char *filename)
{
    struct stat st;
    // check if file doesn't exists
    if (stat(PATH_STORAGE + *filename, &st) == 1)
    {
        FILE *file;
        file = fopen(PATH_STORAGE + *filename, "w+");
        ESP_LOGI(TAG_STORAGE, "Successfully create file: %s\n", filename);
        fclose(file);
    }
    else
    {
        ESP_LOGI(TAG_STORAGE, "The file %s already exists.\n", filename);
    }
}


void read_file(const char *filename, int *read_value)
{
    FILE *file;
    struct stat st;
    // check if file destination exists
    if (stat(PATH_STORAGE + *filename, &st) == 0)
    {
        file = fopen(PATH_STORAGE + *filename, "r+");

        if (sizeof(*filename) == sizeof(FILE_MEASUREMENTS))
        {
            char name[20];
            int pr_val, spo_val;
            while (fscanf(file, "%s %d %d", name, &pr_val, &spo_val) != EOF)
            {
                printf("%s\t\t%d\t\t%d\n", name, pr_val, spo_val);
            }
            fclose(file);
        }
        else if (sizeof(*filename) == sizeof(FILE_MAX_MEASUREMENTS))
        {
            int num_measurements;
            fscanf(file, "%d", &num_measurements);
            ESP_LOGI(TAG_STORAGE, "Maximum Number of measurements is: %d\n", num_measurements);
            *read_value = num_measurements;
            fclose(file);
        }
        else if (sizeof(*filename) == sizeof(FILE_SET_INTERVALS))
        {
            int interval;
            fscanf(file, "%d", &interval);
            if (interval)
                printf("The interval is: %d\n", interval);
            *read_value = interval;
            fclose(file);
        }
    }
    else
    {
        ESP_LOGE(TAG_STORAGE, "The file does not exists. Use create file function to create the file.\n");
    }
}

/*This write file function will only work for the following files:
    - /max_measurements.txt
    - /set_intervals.txt
*/
void write_file(const char *filename, int *value)
{
    FILE *file;
    struct stat st;
    // check if file destination exists
    if (stat(PATH_STORAGE + *filename, &st) == 0)
    {
        file = fopen(PATH_STORAGE + *filename, "w+");
        fprintf(file, (const char *)*value);
        fclose(file);
    }
}

void write_measurements_file(const char *filename, const char *mode, char *name, int *pr_value, int *spo_value)
{
    FILE *file = fopen(filename, mode);
    // Name PulseRate OxygenSat
    fprintf(file, "%s  %d  %d", name, *pr_value, *spo_value);
    fclose(file);
}
