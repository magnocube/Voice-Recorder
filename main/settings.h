#ifndef SETTINGS_H
#define SETTINGS_H
#pragma once

// #include "SDCard.h"
// #include "wm8960.h"

//password and SSID of the voicerecorder... password might be changed to the mac of the device
#define EXAMPLE_ESP_WIFI_SSID      "Lan_Microphone"
#define EXAMPLE_ESP_WIFI_PASS      "12345678"  // replaced by mac adress
#define EXAMPLE_MAX_STA_CONN       5
#define ESP_INTR_FLAG_DEFAULT 0

#define SyncFileName "/sdcard/sync.cfg"

//i2c defines
#define CODEC_I2C_ADDR 0x1a
#define CODEC_I2S_NUM 0       //driver 0
#define AUDIO_BUFFER_SIZE 1024 //size of the buffers for reading the data from dma and storing it on the SD
#define I2S_BUFF_COUNT 10 //playing it safe
#define I2C_CLOCKSPEED 100000  //0.1mhz,,, 

#define I2C_DRIVER_NUM I2C_NUM_0
#define PCA_OUTPUT 0   //configuration register
#define PCA_INPUT 1    //configuration register
#define PCA_LOW 0
#define PCA_HIGH 1
#define PCA_I2C_ADDR 0x20 
#define WAV_HEADER_SIZE 44 
#define FILE_NAME_LENGTH 9
#define MAC_SIZE 6

#define MONO 1
#define STERIO 2
#define MIC_BUILD_IN 0
#define MIC_EXTERNAL_3_5_mm 1 
#define MIC_EXTERNAL_5_0_mm 2


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
    .i2c_clock = 17, \
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
// NOTE: these values will be overwritten in the main methond!.
// NOTE: channel2 will not be used when device is in mono mode.

/* 1=PCM, 257=Mu-Law, 258=A-Law, 259=ADPCM */
#define A_LAW 6 //258
#define U_LAW 7 //257
#define PCM 1

#define ESP_AUDIO_CONFIG_DEFAULT() {\
    .num_channels = STERIO, \
    .bits_per_sample = 16, \
    .sample_rate = 16000, \
    .channel1 = MIC_BUILD_IN, \
    .channel2 = MIC_BUILD_IN, \
    .format = PCM, \
    .swapChannels = false, \
    .preOpApmLeftSel = 1, \
    .preOpApmRightSel = 1, \
    .playbackLeftChannel = false, \
    .playbackRightChannel = false, \
}

#define ESP_SESSION_DATA_DEFAULT() {\
    .Ethernet_IP_Adress = (char*)malloc(15), \
    .Ethernet_Ip_received = false, \
    .is_in_TestModus = false, \
    .last_file_name = (char*)malloc(FILE_NAME_LENGTH +20), \
    .SD_Write_Protect_on = false, \
    .macAdress = (uint8_t*)malloc(MAC_SIZE), \
    .macAdressString = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, \
    .apresaIP = (char*)malloc(16), \
    .apresaPort = 2016, \
    .apresaNumFilesSync = 5, \
}

//TAG used for logging 
static const char* TAG = "Voice Recorder"; 



/*|||||||||_____________CORE SPECIFICATION_______________|||||||||*/
//every function called on a specific core will be executed on that core. core 1 (which should be handling the interface).
//should not handle recording and writing to the SD card (which should happen on core 0).


/*|||||||||_____________AUDIO SETTINGS_______________|||||||||*/
typedef struct{
    short num_channels;
    short bits_per_sample;
    int sample_rate;
    int channel1;
    int channel2;
    short format;
    bool swapChannels; // in case a other source than the input connected to channel 1 on the codec must be recorded
    bool preOpApmLeftSel;
    bool preOpApmRightSel;
    bool playbackLeftChannel;
    bool playbackRightChannel;
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
    int enable48V;      //phantom power 
} esp_pin_config;


/*|||||||||____________SESSION_DATA_______________|||||||||*/
/*this struct will be updated with all settings specific to this device
    -it will be used for the requested IP adress
    -it will be used for testing the device
*/
typedef struct{         
   char* Ethernet_IP_Adress;    //by default requested by DHCP, 
   bool Ethernet_Ip_received;   //indicates if the interface is connected
   bool is_in_TestModus;          //will check if the device is in test modus. the can be enabled by pressing the large button on startup with SD inside.
   char* last_file_name;        //used for storing the last generated file name
   bool SD_Write_Protect_on;
   uint8_t* macAdress;        //pure mac adress
   uint8_t macAdressString[MAC_SIZE*3];  //mac adress converted to string (used for wifi accespoint password)
   char* apresaIP;
   uint32_t apresaPort;
   uint32_t apresaNumFilesSync;
} esp_session_data;

#endif //SETTINGS_H
