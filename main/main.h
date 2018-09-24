#ifndef MAIN_H
#define MAIN_H
#pragma once

#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include <driver/i2c.h>
#include <math.h>
#include <driver/adc.h>



#include "settings.h"
#include "wm8960.h"
#include "SDCard.h"

typedef struct{         //NOTE: keep the position of this struct below SDCARD.H and wm8960.h.    And above recording_task.h and wifi_ethernet_interface_task.h
    bool recording;      
    SDCard *SD;
    WM8960 *codec;
} esp_shared_buffer;
 #include "recording_task.h"
 #include "wifi_ethernet_interface_task.h"




void setupI2C(esp_pin_config *pinconfig);
void setupI2S(esp_pin_config *pinconfig);
void setupPeripherals(esp_pin_config *pinconfig);
// void recording_task(esp_shared_buffer *shared_buffer);
// void Wifi_ethernet_interface_task(esp_shared_buffer *shared_buffer);

esp_shared_buffer sb;
esp_audio_config audioConfig;
esp_pin_config pinout;
#endif //MAIN_H