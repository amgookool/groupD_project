#include "main.h"
#include "ads1115.h"
#include "reporting.h"

static const char *TAG = "MAIN";


void app_main()
{
    ADS1115_CONFIG_FIELDS config_fields;
    config_fields.OS = 0x00;        // No effect
    config_fields.MUX = 0x04;       // AINp = A0 & AINn = GND
    config_fields.PGA = 0x01;       // Gain - 4.096 V
    config_fields.MODE = 0x00;      // Continuous-conversion mode
    config_fields.DR = 0x04;        // 128 SPS
    config_fields.COMP_MODE = 0x00; // Traditional comparator
    config_fields.COMP_POL = 0x00;  // Active low
    config_fields.COMP_LAT = 0x00;  // Nonlatching comparator. Alert pin doesn't latch when asserted
    config_fields.COMP_QUE = 0x02;  // Assert after 4 cconversions

    ESP_LOGI(TAG,"Starting TASK\n");
    xTaskCreate(ads1115_task, "ads1115_task", 2048, (void *)&config_fields, 3, NULL);
}
