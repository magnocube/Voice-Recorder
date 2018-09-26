#ifndef SETTINGS_H
#define SETTINGS_H
#pragma once

// #include "SDCard.h"
// #include "wm8960.h"

//i2c defines
#define CODEC_I2C_ADDR 0x1a
#define CODEC_I2S_NUM 0
#define AUDIO_BUFFER_SIZE 512 //size of the buffers for reading the data from dma and storing it on the SD
#define I2C_CLOCKSPEED 10000  //0.01mhz

#define MONO 1
#define STERIO 2
#define PI 3.14159265
#define WAV_HEADER_SIZE 44 

//default pin configuration
#define ESP_PIN_CONFIG_DEFAULT() {\
    .phy_clock = 0, \
    .phy_clock_OE = 17, \
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
}

//default audio configuration
#define ESP_AUDIO_CONFIG_DEFAULT() {\
    .num_channels = STERIO, \
    .bits_per_sample = 16, \
    .sample_rate = 48000, \
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
} esp_pin_config;


#endif //SETTINGS_H
