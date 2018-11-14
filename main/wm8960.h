
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
all the updates/settings are done to struct esp_audio_config. whenever a variable in the struct esp_audio_config is changed. you should call wm8960::update();
the update class wil first make a list of what ALL the registers should be. then it will update al the registers that have changed. 
(this is to have a local copy of all the registers, because the codec does not allow for reading the registers)
*/

#define R25_VMID_SELECT         0b010000000  //2* 50 Ohm
#define R25_VREF                0b001000000  //on
#define R25_MIC_BIAS            0b000000010  //on

 // copy of registers
typedef struct{      
    uint16_t R0_Codec_Left_Input_Volume;
    uint16_t R1_Codec_Right_Input_Volume;
    uint16_t R2_Codec_LOUT1_Volume;
    uint16_t R3_Codec_ROUT1_Volume;
    uint16_t R4_Codec_Clocking1;
    uint16_t R5_Codec_ADC_DAC_Control1;
    uint16_t R6_Codec_ADC_DAC_Control2;
    uint16_t R7_Codec_Audio_Interface1;
    uint16_t R8_Codec_Clocking2;
    uint16_t R9_Codec_Audio_interface2;
    uint16_t R10_Codec_Left_Dac_Volume;
    uint16_t R11_Codec_Right_Dac_Volume;
    uint16_t R15_Codec_Reset;
    uint16_t R16_Codec_3D_Conrtol;
    uint16_t R17_Codec_ALC1;
    uint16_t R18_Codec_ALC2;
    uint16_t R19_Codec_ALC3;
    uint16_t R20_Codec_Noise_Gate; 
    uint16_t R21_Codec_Left_ADC_Volume;
    uint16_t R22_Codec_Right_ADC_Volume;
    uint16_t R23_Codec_Adittional_control1;
    uint16_t R24_Codec_Adittional_control2;
    uint16_t R25_Codec_Power_Manegement1;
    uint16_t R26_Codec_Power_Manegement2;
    uint16_t R27_Codec_Additional_control3;
    uint16_t R28_Codec_Anti_Pop1;
    uint16_t R29_Codec_Anti_Pop2;
    uint16_t R32_Codec_ADCL_Signal_Path;
    uint16_t R33_Codec_ADCR_Signal_Path;
    uint16_t R34_Codec_Left_Out_Mix;
    uint16_t R37_Codec_Right_Out_Mix;
    uint16_t R38_Codec_Mono_Out_Mix1;
    uint16_t R39_Codec_Mono_Out_Mix2;
    uint16_t R40_Codec_LOUT1_Volume;
    uint16_t R41_Codec_ROUT2_Volume;
    uint16_t R42_Codec_MONOOUT_Volume;
    uint16_t R43_Codec_Input_Boost_Mixer1;
    uint16_t R44_Codec_Input_Boost_Mixer2;
    uint16_t R45_Codec_Bypass1;
    uint16_t R46_Cocec_Bypass2;
    uint16_t R47_Codec_Power_Manegement3;
    uint16_t R48_Codec_Additional_Control4;
    uint16_t R49_Codec_Class_D_Control1;
    uint16_t R51_Codec_Class_D_Control3;
    uint16_t R52_Codec_PLL_N;
    uint16_t R53_Codec_PLL_K_1;
    uint16_t R54_Codec_PLL_K_2;
    uint16_t R55_Codec_PLL_K_3;    
} codec_register_copy;

//default values of the registers (according to the datasheet)
#define CODEC_REGISTER_COPY_DEFAULT() {\
    .R0_Codec_Left_Input_Volume =           0b010010111, \
    .R1_Codec_Right_Input_Volume =          0b010010111, \
    .R2_Codec_LOUT1_Volume =                0b000000000, \
    .R3_Codec_ROUT1_Volume =                0b000000000, \
    .R4_Codec_Clocking1 =                   0b000000000, \
    .R5_Codec_ADC_DAC_Control1 =            0b000001000, \
    .R6_Codec_ADC_DAC_Control2 =            0b000000000, \
    .R7_Codec_Audio_Interface1 =            0b000001010, \
    .R8_Codec_Clocking2 =                   0b111000000, \
    .R9_Codec_Audio_interface2 =            0b000000000, \
    .R10_Codec_Left_Dac_Volume =            0b111111111, \
    .R11_Codec_Right_Dac_Volume =           0b111111111, \
    .R15_Codec_Reset =                      0b000000000, \
    .R16_Codec_3D_Conrtol =                 0b000000000, \
    .R17_Codec_ALC1 =                       0b001111011, \
    .R18_Codec_ALC2 =                       0b100000000, \
    .R19_Codec_ALC3 =                       0b000110010, \
    .R20_Codec_Noise_Gate =                 0b000000000, \
    .R21_Codec_Left_ADC_Volume =            0b011000011, \
    .R22_Codec_Right_ADC_Volume =           0b011000011, \
    .R23_Codec_Adittional_control1 =        0b111000000, \
    .R24_Codec_Adittional_control2 =        0b000000000, \
    .R25_Codec_Power_Manegement1 =          0b000000000, \
    .R26_Codec_Power_Manegement2 =          0b000000000, \
    .R27_Codec_Additional_control3 =        0b000000000, \
    .R28_Codec_Anti_Pop1 =                  0b000000000, \
    .R29_Codec_Anti_Pop2 =                  0b000000000, \
    .R32_Codec_ADCL_Signal_Path =           0b100000000, \
    .R33_Codec_ADCR_Signal_Path =           0b100000000, \
    .R34_Codec_Left_Out_Mix =               0b001010000, \
    .R37_Codec_Right_Out_Mix =              0b001010000, \
    .R38_Codec_Mono_Out_Mix1 =              0b000000000, \
    .R39_Codec_Mono_Out_Mix2 =              0b000000000, \
    .R40_Codec_LOUT1_Volume =               0b000000000, \
    .R41_Codec_ROUT2_Volume =               0b000000000, \
    .R42_Codec_MONOOUT_Volume =             0b001000000, \
    .R43_Codec_Input_Boost_Mixer1 =         0b000000000, \
    .R44_Codec_Input_Boost_Mixer2 =         0b000000000, \
    .R45_Codec_Bypass1 =                    0b001010000, \
    .R46_Cocec_Bypass2 =                    0b001010000, \
    .R47_Codec_Power_Manegement3 =          0b000000000, \
    .R48_Codec_Additional_Control4 =        0b000000010, \
    .R49_Codec_Class_D_Control1 =           0b000110111, \
    .R51_Codec_Class_D_Control3 =           0b010000000, \
    .R52_Codec_PLL_N =                      0b000000000, \
    .R53_Codec_PLL_K_1 =                    0b000110001, \
    .R54_Codec_PLL_K_2 =                    0b000100110, \
    .R55_Codec_PLL_K_3 =                    0b011101001, \
}


class WM8960{
    public:
        WM8960(esp_audio_config *audioC,SDCard *sd_card,pca9535 *gpioHeader,esp_pin_config *pinconfig);     //constructor, will do the setup of the chip
        void read();     //reads the i2s bus. (call this before accessing the audiobuffer1)                                                                                   // calls the read function from i2s driver. will take a number of bits and put it into the audiobuffer1
        void printCopyCodecRegisters();                                                                     
        uint8_t* audioBuffer1;                                                                              //buffer to hold the latest data from the i2s driver
        
    private:
        void setupI2S();                                                                                    //does the setup.. installs the driver, gets called by the constructor
        void update();                                                                                      //when parameters in the struct esp_audio_config change, call this function to update the corresponding registers
        void writeRegisters();                                                                              //write out all the registers (to ensure the codec has the same registers as the copy in the struct 'codec_register_copy')
        void setRegister(uint16_t &reg, uint16_t value);                                                      //overwrites the whole register
        void setBitsHigh(uint16_t &reg, uint16_t mask);                                                       //only sets the given bits high, ignores other bits
        void setBitsLow(uint16_t &reg, uint16_t mask);                                                        //only sets the given bits low, ignores other bits
        void send_I2C_command(uint8_t reg, uint16_t value);                                                 //send a value to a register
        void initialSetupRegisters();
        void printRegister(uint8_t index,uint16_t value);
        esp_audio_config *audioConfig;
        esp_pin_config *pinout;
        SDCard *SD;
        pca9535 *gpio_header;                                                                               //i2c gpio expansion module (pointer, is instantiated in main.cpp)
        i2s_config_t i2s_config;                                                                            //must be declared. otherwise it will fall from the stack after wm8960::setupI2S() has finished
        i2s_pin_config_t pin_config;                                                                        //same as comment above                                               
        codec_register_copy regCopy;                                                                        //struct which holds a copy of all the registers in the codec
        
};




#endif // WM8960_H