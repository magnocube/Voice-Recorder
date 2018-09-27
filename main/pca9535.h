
#ifndef PCA9535_H
#define PCA9535_H
#pragma once

#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include <driver/i2c.h>
#include "esp_log.h"
#include "esp_err.h"

#include "settings.h" 




class pca9535{
    public:
        pca9535(esp_pin_config * pinconfig);                                                    //constructor, will do the setup of the chip
        uint16_t getRawData();
    private:
        esp_pin_config *pinout;
        bool mode[16]; //input or output
        bool data[16]; //logic voltage(mode does not matter)
};




#endif // PCA9535_H