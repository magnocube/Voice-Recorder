#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include <driver/i2c.h>



#include "settings.h"
#include "wm8960.h"




void setupI2C(esp_pin_config *pinconfig);
void setupI2S(esp_pin_config *pinconfig);
void setupPeripherals(esp_pin_config *pinconfig);