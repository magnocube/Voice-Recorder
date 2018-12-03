
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
        pca9535(esp_pin_config * pinconfig, esp_session_data * sessionData);                                                    //constructor, will do the setup of the chip
        uint16_t getRawWriteData();  //the read(input) data of all pins
        uint16_t getRawConfigData();  //the config (input/output) data of all pins
        void pinMode(int pin, int m,bool flush);  
        void digitalWrite(int pin, int mode,bool flush);
        bool digitalRead(int pin,bool updateFirst);                                  
    private:
        esp_session_data *session_data;
        esp_pin_config *pinout;
        bool mode[16]; //input or output
        bool writeData[16]; //logic voltage of the io (for output)
        bool readData[16];  //logic input voltage (for input)
        void writeDataToI2C();
        void writeConfigToI2C();
        void readDataFromI2C();
};




#endif // PCA9535_H