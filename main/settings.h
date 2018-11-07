#ifndef SETTINGS_H
#define SETTINGS_H
#pragma once

// #include "SDCard.h"
// #include "wm8960.h"

//password and SSID of the voicerecorder... passwordt might be changed to the mac of the device
#define EXAMPLE_ESP_WIFI_SSID      "voiceRecorder"
#define EXAMPLE_ESP_WIFI_PASS      "12345678"
#define EXAMPLE_MAX_STA_CONN       5
#define ESP_INTR_FLAG_DEFAULT 0

//i2c defines
#define CODEC_I2C_ADDR 0x1a
#define CODEC_I2S_NUM 0       //driver 0
#define AUDIO_BUFFER_SIZE 512 //size of the buffers for reading the data from dma and storing it on the SD
#define I2S_BUFF_COUNT 8 //playing it safe
#define I2C_CLOCKSPEED 400000  //0.4mhz,,, 

#define I2C_DRIVER_NUM I2C_NUM_0
#define MONO 1
#define STERIO 2
#define PCA_OUTPUT 0   //configuration register
#define PCA_INPUT 1    //configuration register
#define PCA_LOW 0
#define PCA_HIGH 1
#define PCA_I2C_ADDR 0x20 
#define WAV_HEADER_SIZE 44 



#define PIN_PHY_POWER 32    //can also be found in ESP_PIN_CONFIG_DEFAULT.  
#define PIN_SMI_MDC 23      //can also be found in ESP_PIN_CONFIG_DEFAULT.  
#define PIN_SMI_MDIO 18     //can also be found in ESP_PIN_CONFIG_DEFAULT.  

//default pin configuration
#define ESP_PIN_CONFIG_DEFAULT() {\
    .phy_clock = 0, \
    .phy_clock_OE = 32, \
    .sd_D0 = 2, \
    .sd_CLK = 14, \
    .sd_CMD = 15, \
    .i2s_BCLK = 4, \
    .i2s_DIN = 5, \
    .i2s_WS = 16, \
    .i2s_DOUT = 33, \
    .analogIn = 35, \
    .i2c_clock = 12, \
    .i2c_data = 13, \
    .MDIO = 18, \
    .MDC = 23, \
    .big_button = 34, \
    .led_red = 0, \
    .led_green = 1, \
    .led_blue = 2, \
    .led_yellow = 3, \
    .mic_select_0 = 4, \
    .mic_select_1 = 5, \
    .phy_reset = 6, \
    .ethernet_up_led = 7, \
    .sdPower = 8, \
    .sdDetect = 9, \
    .sdProtect = 10, \
    .enable48V = 11, \
}

//default audio configuration
//might be overwritten with spiffs data (to keep the changes from last run)
#define ESP_AUDIO_CONFIG_DEFAULT() {\
    .num_channels = STERIO, \
    .bits_per_sample = 16, \
    .sample_rate = 16000, \
}



//TAG used for logging
static const char* TAG = "Voice Recorder"; 


/*|||||||||_____________CORE COMMUNICATION_______________|||||||||*/
//every fonction called on a specific core will be executed on that core. core 1 (which should be handling the interface).
//should not handle recording and writing to the SD card. therefore this shared buffer will indicate what eacht task should do


/*|||||||||_____________AUDIO SETTINGS_______________|||||||||*/
typedef struct{
    short num_channels;
    short bits_per_sample;
    int sample_rate;
} esp_audio_config;


/*|||||||||____________PIN CONFIGURATION_______________|||||||||*/
typedef struct{         
    int phy_clock;      //clock input of Phy module 
    int phy_clock_OE;   //enable/disable Phy clock. (default on GPIO0 (boot problem))
    int sd_D0;          //sd card: data
    int sd_CLK;         //sd card: clock
    int sd_CMD;         //sd card: command
    int i2s_BCLK;       //codec: bitclock
    int i2s_DIN;        //codec: data in (in to microcontroller)
    int i2s_WS;         //codec: word select
    int i2s_DOUT;       //codec: data out (out from microcontroller)
    int analogIn;       //analog input. for detecting sound
    int i2c_clock;      //i2c bus: clock
    int i2c_data;       //i2c bus: data
    int MDIO;           //Phy: control IO: data
    int MDC;            //Phy: control IO: clock
    int big_button;     //large button on top of case
    int led_red;                                                        //NOTE: the following gpio are on the i2c expander,,, not on theESP32
    int led_green;
    int led_blue;
    int led_yellow;
    int mic_select_0;   //keep at default,, 3.5mm mic
    int mic_select_1;   //switch between build in and phantom mic
    int phy_reset;      //reset-pin of PHY
    int ethernet_up_led;//led on rj45 connector
    int sdPower;        //pullup and vcc to SD card
    int sdDetect;       //sd card in slot
    int sdProtect;      //wirte protection pin
    int enable48V;      //pahnom power 
} esp_pin_config;


#endif //SETTINGS_H
