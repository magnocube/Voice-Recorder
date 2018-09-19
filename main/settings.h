#ifndef SETTINGS_H
#define SETTINGS_H
#pragma once


#define CODEC_I2C_ADDR 0x1a

#define I2C_CLOCKSPEED 10000  //0.01mhz


static const char* TAG = "Voice Recorder";  //default tag used for logging

typedef struct{         //|||||||||____________PIN CONFIGURATION_______________|||||||||
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

#endif //SETTINGS_H
