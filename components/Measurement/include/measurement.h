#ifndef MEASUREMENT_H
#define MEASUREMENT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sys/time.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"

#include "esp_log.h"
#include "esp_err.h"
#include "esp_system.h"

#include "esp8266/gpio_register.h"
#include "esp8266/pin_mux_register.h"

#include "driver/gpio.h"
#include "driver/i2c.h"
#include "driver/pwm.h"


#define I2C_MASTER_SCL_IO 2 // GPIO number for I2C master clock
#define I2C_MASTER_SDA_IO 0 // GPIO number for I2C master data
#define LED_DRIVER_PIN 3    // RXD pin as GPIO
#define ALARM_BUZZER_PIN 2  // GPIO pin for buzzer

#define I2C_ALERT_PIN   0   // ALERT pin GPIO pin
#define I2C_MASTER_NUM I2C_NUM_0    // I2C port number for master dev
#define I2C_MASTER_TX_BUF_DISABLE 0 // I2C master do not need buffer
#define I2C_MASTER_RX_BUF_DISABLE 0 // I2C master do not need buffer

#define WRITE_BIT I2C_MASTER_WRITE // I2C master write
#define READ_BIT I2C_MASTER_READ   // I2C master read
#define ACK_CHECK_EN 0x1           // I2C master will check ack from slave
#define ACK_CHECK_DIS 0x0          // I2C master will not check ack from slave
#define ACK_VAL 0x0                // I2C ack value
#define NACK_VAL 0x1               // I2C nack value
#define LAST_NACK_VAL 0x2          // I2C last_nack value

// Definitions for ADS1115 ADDR Pin possible address
#define ADS1115_ADDR_GND 0x48
#define ADS11115_ADDR_VDD 0x49
#define ADS1115_ADDR_SDA 0x4A
#define ADS1115_ADDR_SCL 0x4B

// Definitions for Conv, Config, Lo_Thresh, High_Thresh Register Addresses
#define ADS1115_CONV_REG 0x00
#define ADS1115_CONFIG_REG 0x01
#define ADS1115_LOTHRESH_REG 0x02
#define ADS1115_HITHRESH_REG 0x03

// Struct Definition for configuration bits for Configuration Register
typedef struct config_fields
{
    uint8_t OS;             // Operational Status (1-bit)
    uint8_t MUX;            // Input Multiplexer (3-bits)
    uint8_t PGA;            // Programmable Gain Amplifier (3-bits)
    uint8_t MODE;           // Mode (1-bit)
    uint8_t DR;             // Data Rate (3-bits)
    uint8_t COMP_MODE;      // Comparator Mode (1-bit)
    uint8_t COMP_POL;       // Comparator Polarity (1-bit)
    uint8_t COMP_LAT;       // Latching Comparator (1-bit)
    uint8_t COMP_QUE;       // Comparator Queue and Disable (2-bits)
    uint16_t configuration; // 16-bit configuration of all above fields
} ADS1115_CONFIG_FIELDS;

// ADS1115 Function Prototypes
esp_err_t ads1115_i2c_init(i2c_port_t i2c_num, ADS1115_CONFIG_FIELDS *configs);
esp_err_t ads1115_16bit_config(ADS1115_CONFIG_FIELDS *config);
esp_err_t ads1115_write_bytes(i2c_port_t i2c_num, uint8_t reg_addr, uint8_t *data, uint16_t data_len);
esp_err_t ads1115_write_data(i2c_port_t i2c_num, uint8_t reg_addr, uint16_t data);
esp_err_t ads1115_read_bytes(i2c_port_t i2c_num, uint8_t reg_addr, uint8_t *data, uint16_t data_len);
esp_err_t ads1115_read_data(i2c_port_t i2c_num, uint8_t reg_addr, uint16_t *data);

void ads1115_read_task(void* pvParam);

void gpio_pin0_init(void *pvParam);
void gpio_pin2_init(void* pvParam);
void led_driver_init(void *pvParam);
void led_driver_function(void *pvParam);
void active_wait(int wait_time_usecs);

#endif