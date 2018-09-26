
#ifndef WM8960_H
#define WM8960_H
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
#include "SDCard.h"
#include <driver/i2s.h>
#include "pca9535.h"


/*
Brief working of the class...
all the updates are done to esp_audio_config. whenever a variable in the struct esp_audio_config is changed. you should call wm8960::update();
the update class wil first make a list of what ALL the registers should be. then it will update al the registers that have changed. 
(this is to have a local copy of all the registers, because the codec does not allow for reading the registers)
*/
class WM8960{
    public:
        WM8960(esp_audio_config *audioC,SDCard *sd_card,pca9535 *gpioHeader,esp_pin_config *pinconfig);        //constructor, will do the setup of the chip
        void read();
        uint8_t* audioBuffer1;
        
    private:
        void setupI2S();                                        //does the setup.. installs the driver, gets called by the constructor
        void update();                                          //when parameters in the struct esp_audio_config change, call this function to update the codec
        void send_I2C_command(uint8_t reg, uint16_t value);     //send a value to a register
        void micToHeadsetBypass();
        esp_audio_config *audioConfig;
        esp_pin_config *pinout;
        SDCard *SD;
        pca9535 *gpio_header; //i2c gpio expansion module (pinter, is instantiated in main.cpp)
        i2s_config_t i2s_config; //must be declared. otherwise it will fall from the stack after wm8960::setupI2S() has finished
        i2s_pin_config_t pin_config; // same as comment above
        
};




#endif // WM8960_H