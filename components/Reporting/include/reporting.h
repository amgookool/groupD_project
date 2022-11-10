#ifndef REPORTING_H
#define REPORTING_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "esp_log.h"
#include "driver/uart.h"

#define UART_NUMBER UART_NUM_0

#define TX_BUFF_SIZE (1024)
#define RX_BUFF_SIZE (TX_BUFF_SIZE)

void serial_init();

void serial_task();



#endif