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
#include "esp_spiffs.h" 
#include <math.h>
#include <driver/adc.h>
#include "esp_system.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_eth.h"
#include "rom/gpio.h"



#include "settings.h"
#include "wm8960.h"
#include "SDCard.h"
#include "pca9535.h"


#define MESSAGE "Hello TCP Client!!"
#define LISTENQ 2


#include "myPhy.h" //testing,,, this will become the ksz-PHY
#define DEFAULT_ETHERNET_PHY_CONFIG phy_KSZ8081_default_ethernet_config




// nvs_handle my_handle;

typedef struct{         //NOTE: keep the position of this struct below SDCARD.H and wm8960.h.    And above recording_task.h and wifi_ethernet_interface_task.h
    bool recording; 
    pca9535 *gpio_header;      
    SDCard *SD;
    WM8960 *codec;
    esp_audio_config *audio_config;
    esp_pin_config *pin_config;
    nvs_handle my_NVS_handle;
} esp_shared_buffer;

esp_shared_buffer sb;               //local variable of the struct above
esp_audio_config audioConfig;       //local variable of the audio config (used in the 'esp_shared_buffer' and can be found in 'settings.h')
esp_pin_config pinout;              //local variable of the pinout of the device (used in 'esp_shared_buffer' and can be found in 'settings.h')

 #include "recording_task.h"
 #include "wifi_ethernet_interface_task.h"




void setupI2C(esp_pin_config *pinconfig);
void setupSPIFFS();
void configureGPIOExpander();
void setupNVS();
void setupPeripherals(esp_pin_config *pinconfig);
// void recording_task(esp_shared_buffer *shared_buffer);
// void Wifi_ethernet_interface_task(esp_shared_buffer *shared_buffer);

void testSPIFFSRead();



#endif //MAIN_H