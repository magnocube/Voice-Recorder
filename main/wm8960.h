
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



class WM8960{
    public:
        WM8960(esp_audio_config *audioC,SDCard *sd_card);                                                    //constructor, will do the setup of the chip
    private:
        void send_I2C_command(uint8_t reg, uint16_t value);     //send a value to a register
        void micToHeadsetBypass();
        esp_audio_config *audioConfig;
        SDCard *SD;
};




#endif // WM8960_H