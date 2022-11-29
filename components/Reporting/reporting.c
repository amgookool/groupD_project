#include "reporting.h"

static const char *TAG_SERIAL = "SERIAL";
static const char *TAG_STORAGE = "Storage";

static QueueHandle_t serial_queue;

void serial_init(void *pvParam)
{
    ESP_LOGI(TAG_SERIAL,"Initializing UART configurations.\n");
    uart_config_t serial_configs = {
        .baud_rate = 74880,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE};

    uart_param_config(UART_NUMBER, &serial_configs);
    uart_driver_install(UART_NUMBER, RX_BUFF_SIZE * 2, TX_BUFF_SIZE * 2, 150, &serial_queue, 0);
}

/*
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
                uart_write_bytes(UART_NUMBER, (const char *)"---------------- Menu ------------------------------\n", serial_event.size);
                uart_write_bytes(UART_NUMBER, (const char *)"Please enter Integer to select setting\n", serial_event.size);
                uart_write_bytes(UART_NUMBER, (const char *)"1. Set frequency intervals for storing measurements\n", serial_event.size);
                uart_write_bytes(UART_NUMBER, (const char *)"2. Set maximum number of measurements to be stored\n", serial_event.size);
                uart_write_bytes(UART_NUMBER, (const char *)"3. Set the user being measured.\n", serial_event.size);
                uart_write_bytes(UART_NUMBER, (const char *)"4. View Stored measurements\n", serial_event.size);
                uart_write_bytes(UART_NUMBER, (const char *)"5. Enter Measurements Mode.\n", serial_event.size);

                uart_read_bytes(UART_NUMBER, dtmp, serial_event.size, portMAX_DELAY);
                ESP_LOGI(TAG_SERIAL, "The DATA REceived:\n");

                if ((int)dtmp == 1)
                {
                    ESP_LOGI(TAG_SERIAL, "Enter the value in seconds\n");
                    uart_read_bytes(UART_NUMBER, dtmp, serial_event.size, portMAX_DELAY);
                    int interval = (int)dtmp;
                    write_file(FILE_SET_INTERVALS, &interval);
                }
                else if ((int)dtmp == 2)
                {
                    ESP_LOGI(TAG_SERIAL, "Enter the value in seconds\n");
                    uart_read_bytes(UART_NUMBER, dtmp, serial_event.size, portMAX_DELAY);
                    int max_num = (int)dtmp;
                    write_file(FILE_MAX_MEASUREMENTS, &max_num);
                }
                else if ((int)dtmp == 3)
                {
                    ESP_LOGI(TAG_SERIAL, "Enter the name of the person you are measuring.");
                    uart_read_bytes(UART_NUMBER, dtmp, serial_event.size, portMAX_DELAY);
                    *user = (char *)dtmp;
                    ESP_LOGI(TAG_SERIAL, "The User is set to: %s", user);
                }
                else if ((int)dtmp == 4)
                {
                    ESP_LOGI(TAG_REPORTING, "Displaying Measurements.");
                    read_file(FILE_MEASUREMENTS);
                }

                uart_write_bytes(UART_NUMBER, (const char *)dtmp, serial_event.size);
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

*/

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

/*File Modes
fopen function allows manipulation of files by specifying a mode which are:
    r - Opens a file for reading. The file must exist.
    w - Creates an empty file for writing. If a file with the same name already exists, its content is erased and the file is considered as a new empty file.
    a - Appends to a file. Writing operations, append data at the end of the file. The file is created if it does not exist.
    r+ - Opens a file to update both reading and writing. The file must exist.
    w+ - Creates an empty file for both reading and writing.
    a+ - Opens a file for reading and appending.
*/
// FILE *get_file(const char *file_name, const char *mode_)
// {
//     FILE *file;
//     struct stat st;
//     // check if file destination exists
//     if (stat(PATH_STORAGE + *file_name, &st) == 0)
//     {
//         file = fopen(PATH_STORAGE + *file_name, *mode_);
//     }
//     return file;
// }

void read_file(const char *filename, int* read_value)
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
        fprintf(file, (const char*) *value);
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
