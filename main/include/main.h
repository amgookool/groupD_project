#ifndef MAIN_H
#define MAIN_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/unistd.h>

#include "esp_log.h"
#include "esp_err.h"
#include "esp_system.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"

#include "driver/i2c.h"
#include "driver/gpio.h"
#include "driver/uart.h"
#include "esp_spiffs.h"

#include "reporting.h"
#include "measurement.h"

#define HIGHEST_PRIORITY 10
#define LOWEST_PRIORITY 1

// void reporting_task(void *pvParams);
void measurement_init_task(void *pvParam);

#endif