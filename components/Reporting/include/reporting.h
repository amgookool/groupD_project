#ifndef REPORTING_H
#define REPORTING_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/unistd.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"

#include "esp_log.h"
#include "esp_err.h"
#include "esp_spiffs.h"
#include "driver/uart.h"

// Storage Definitions
#define MAX_NUM_FILES 5 
#define PATH_STORAGE "/spiffs"
#define FILE_MAX_MEASUREMENTS "/max_measurements.txt"
#define FILE_MEASUREMENTS "/measurements.txt"
#define FILE_SET_INTERVALS "/set_intervals.txt"
#define FILE_SET_FORMAT "/set_format.txt"

// Uart Definitions 
#define UART_NUMBER UART_NUM_0
#define BUF_SIZE (2048)
#define RD_BUF_SIZE (BUF_SIZE)

// UART Function Prototypes
void serial_init(void* pvParam);
void display_menu(void *pvParam);
static void uart_event_task(void *pvParameters);

// Storage Function Prototypes 
void init_storage(void *pvParam);
void create_file(const char *filename);
void read_file(const char *filename, int* read_value);
void write_file(const char* filename,int* value);
void write_measurements_file(const char *filename, const char *mode, char *name, int *pr_value, int *spo_value);

#endif