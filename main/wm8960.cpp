#include "wm8960.h"


void WM8960::send_I2C_command(uint8_t reg, uint16_t value){
    vTaskDelay(10/portTICK_PERIOD_MS);
    esp_err_t errAddr, errReg, errVal, espRc;
    
    uint8_t rByte = (reg << 1) | (value >> 8); // register + first bit
    uint8_t vByte = value  & 0x00FF; //last 8 bits 
    
    
    ESP_LOGI(TAG, "writing trough I2C (WM8960)... Register: %d  With data: %d",rByte>>1,value);

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    
    i2c_master_start(cmd);
    errAddr = i2c_master_write_byte(cmd,(CODEC_I2C_ADDR << 1) | I2C_MASTER_WRITE, true /* expect ack */);     // codec adress + write
    errReg = i2c_master_write_byte(cmd,rByte,true);
    errVal = i2c_master_write_byte(cmd,vByte,true);
    i2c_master_stop(cmd);
    espRc = i2c_master_cmd_begin(I2C_DRIVER_NUM, cmd, 100/portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);

    if(errAddr != ESP_OK){
        ESP_LOGW(TAG, "error in I2C writing address to command");
    } else if(errReg != ESP_OK){
        ESP_LOGW(TAG, "error in I2C writing register to command");
    } else if(errVal != ESP_OK){
        ESP_LOGW(TAG, "error in I2C writing value to command");
    } else if(espRc != ESP_OK){
        ESP_LOGW(TAG, "error in I2C writing the command to WM8960, .. %d",espRc);
         //ESP_ERROR_CHECK( espRc);
      
        //ESP_LOGW(TAG,esp_err_to_name(espRc));
    } else {
        //uncomment line below to have log feedback that there was no error.
        // ESP_LOGI(TAG, "no error in writing i2c");
    }
    

}
WM8960::WM8960(esp_audio_config *audioC, SDCard *sd_card, pca9535 *gpioHeader,esp_pin_config *pinconfig){
    SD = sd_card;                                   //might be needed to have status of SD card. not used now
    audioConfig = audioC;                           //disired confuguration of the recorder. call wm8960::update(); to apply changes
    gpio_header = gpioHeader;                       //external i2c gpio expander, needed for controlling some IC's
    pinout = pinconfig;                             //pinout of hardware. needed for connecting the i2s driver
    regCopy = CODEC_REGISTER_COPY_DEFAULT();        //see wm8960.h, contains a copy of all the registers

    audioBuffer1 = (uint8_t*)malloc(AUDIO_BUFFER_SIZE);
    
    

    
    
    initialSetupRegisters(); //configuration example, WILL USE VARIABLE SET BY MICPATH
    setupMicPath(); //sets the microphones in the correct position (connects the right microphones)
    setupI2S(); // init i2s driver
  

}

void WM8960::setupMicPath(){
     gpio_header->pinMode(pinout->mic_select_0, PCA_OUTPUT,false);    
     gpio_header->pinMode(pinout->mic_select_1,PCA_OUTPUT,false); 
     gpio_header->digitalWrite(pinout->mic_select_0,audioConfig->preOpApmLeftSel,false); //high = build in ,, low = extern (3.5mm)  mic0 = left (MIC1 on PCB)
     gpio_header->digitalWrite(pinout->mic_select_1,audioConfig->preOpApmRightSel,false); //high = build in ,, low = extern (5mm)   mic1 = right (MIC2 on PCB)
    
}
/*setup the i2s bus with the correct settings. 
the audio settings are stored in the struct audioConfig
the pins used for the i2s communication are stored in the struct pinout*/
void WM8960::setupI2S(){  //setup the i2s bus of the esp32  
  

    i2s_config = {
        .mode = static_cast<i2s_mode_t>(I2S_MODE_SLAVE | I2S_MODE_RX),
        .sample_rate = audioConfig->sample_rate, 
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,  //.bits_per_sample = static_cast<i2s_bits_per_sample_t>(audioConfig->bits_per_sample), always 16 bit, play with recording task for other values
        .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,  //default value, will be overwritten 
        .communication_format = static_cast<i2s_comm_format_t>(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB),
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1 , // high interrupt priority
        .dma_buf_count = I2S_BUFF_COUNT,
        .dma_buf_len = AUDIO_BUFFER_SIZE, // 1024
        .use_apll = false,  //codec has own clock
        .tx_desc_auto_clear = true,
        .fixed_mclk = 0
    };
    if(audioConfig->format == A_LAW || audioConfig->format == U_LAW){
        i2s_config.sample_rate = 16000; // will be made smaller in recording task
    }
    // if(audioConfig->num_channels ==1){   //          
    //     i2s_config.channel_format = I2S_CHANNEL_FMT_ONLY_RIGHT;
    // } else if(audioConfig->num_channels == 2){
    //     i2s_config.channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT;
    // }
     pin_config = {
        .bck_io_num = pinout->i2s_BCLK,
        .ws_io_num = pinout->i2s_WS,
        .data_out_num = pinout->i2s_DOUT,
        .data_in_num = pinout->i2s_DIN
    };
	 //install and start i2s driver
	i2s_driver_install((i2s_port_t)CODEC_I2S_NUM, &i2s_config, 0, NULL);   //install and start i2s driver
    i2s_set_pin((i2s_port_t)CODEC_I2S_NUM, &pin_config);

    // SET_PERI_REG_BITS(I2S_TIMING_REG(0), 0x1, 1, I2S_TX_DSYNC_SW_S);
    // SET_PERI_REG_BITS(I2S_CONF_REG(0), 0x1, 1, I2S_RX_SLAVE_MOD_S);

}
void WM8960::read(){                   //read from the dma buffers. make sure to call this function frequently to prevent a buffer overflow
    size_t numBytesread; 
    i2s_read((i2s_port_t)CODEC_I2S_NUM,(uint8_t*)audioBuffer1, AUDIO_BUFFER_SIZE,&numBytesread,portMAX_DELAY);
}
void WM8960::update(){
    //take info from esp_audio_config and update the registers that should be affected by this.. then write out all those registers
}
void WM8960::printCopyCodecRegisters(){
    ESP_LOGI(TAG, "Printing out all Codec Variables (local Copy)");
    printRegister(0, regCopy.R0_Codec_Left_Input_Volume);
    printRegister(1, regCopy.R1_Codec_Right_Input_Volume);
    printRegister(2, regCopy.R2_Codec_LOUT1_Volume);
    printRegister(3, regCopy.R3_Codec_ROUT1_Volume);
    printRegister(4, regCopy.R4_Codec_Clocking1);
    printRegister(5, regCopy.R5_Codec_ADC_DAC_Control1);
    printRegister(6, regCopy.R6_Codec_ADC_DAC_Control2);
    printRegister(7, regCopy.R7_Codec_Audio_Interface1);
    printRegister(8, regCopy.R8_Codec_Clocking2);
    printRegister(9, regCopy.R9_Codec_Audio_interface2);
    printRegister(10, regCopy.R10_Codec_Left_Dac_Volume);
    printRegister(11, regCopy.R11_Codec_Right_Dac_Volume);
    printRegister(15, regCopy.R15_Codec_Reset);
    printRegister(16, regCopy.R16_Codec_3D_Conrtol);
    printRegister(17, regCopy.R17_Codec_ALC1);
    printRegister(18, regCopy.R18_Codec_ALC2);
    printRegister(19, regCopy.R19_Codec_ALC3);
    printRegister(20, regCopy.R20_Codec_Noise_Gate);
    printRegister(21, regCopy.R21_Codec_Left_ADC_Volume);
    printRegister(22, regCopy.R22_Codec_Right_ADC_Volume);
    printRegister(23, regCopy.R23_Codec_Adittional_control1);
    printRegister(24, regCopy.R24_Codec_Adittional_control2);
    printRegister(25, regCopy.R25_Codec_Power_Manegement1);
    printRegister(26, regCopy.R26_Codec_Power_Manegement2);
    printRegister(27, regCopy.R27_Codec_Additional_control3);
    printRegister(28, regCopy.R28_Codec_Anti_Pop1);
    printRegister(29, regCopy.R29_Codec_Anti_Pop2);
    printRegister(32, regCopy.R32_Codec_ADCL_Signal_Path);
    printRegister(33, regCopy.R33_Codec_ADCR_Signal_Path);
    printRegister(34, regCopy.R34_Codec_Left_Out_Mix);
    printRegister(37, regCopy.R37_Codec_Right_Out_Mix);
    printRegister(38, regCopy.R38_Codec_Mono_Out_Mix1);
    printRegister(39, regCopy.R39_Codec_Mono_Out_Mix2);
    printRegister(40, regCopy.R40_Codec_LOUT1_Volume);
    printRegister(41, regCopy.R41_Codec_ROUT2_Volume);
    printRegister(42, regCopy.R42_Codec_MONOOUT_Volume);
    printRegister(43, regCopy.R43_Codec_Input_Boost_Mixer1);
    printRegister(44, regCopy.R44_Codec_Input_Boost_Mixer2);
    printRegister(45, regCopy.R45_Codec_Bypass1);
    printRegister(46, regCopy.R46_Cocec_Bypass2);
    printRegister(47, regCopy.R47_Codec_Power_Manegement3);
    printRegister(48, regCopy.R48_Codec_Additional_Control4);
    printRegister(49, regCopy.R49_Codec_Class_D_Control1);
    printRegister(51, regCopy.R51_Codec_Class_D_Control3);
    printRegister(52, regCopy.R52_Codec_PLL_N);
    printRegister(53, regCopy.R53_Codec_PLL_K_1);
    printRegister(54, regCopy.R54_Codec_PLL_K_2);
    printRegister(55, regCopy.R55_Codec_PLL_K_3);
    
    
  
    
}
void WM8960::printRegister(uint8_t index, uint16_t value){   // used for debugging the registers in the codec
    printf("register (HEX)%x, (DEC)%d,   with value: (HEX) %03X ,(BIN) 0B", index,index,value); 

    uint16_t valCopy = value;
   for (int i = 0; i < 16; i++) {
       if(i >= 7){
        printf("%d", (valCopy & 0x8000) >> 15);
       }
        valCopy <<= 1;
    }
    printf(" ,(DEC) %d",value);
    printf("\n");

}

void WM8960::initialSetupRegisters(){ //example config



    // // //enable microphone bias and enables power on ADC's
    // setRegister(regCopy.R25_Codec_Power_Manegement1,R25_MIC_BIAS|R25_VREF|R25_VMID_SELECT|R25_POWER_ADCL|R25_POWER_ADCR|R25_ENABLE_PGA_BOOST);  //micbias +vref+pgaBoost+adc
    // setRegister(regCopy.R47_Codec_Power_Manegement3,R47_ENABLE_PGA|R47_ENBALE_OUTPUT_MIXER);
    // //set the clock frequencies
    // setRegister(regCopy.R26_Codec_Power_Manegement2,R26_PLL_ENABLE);
    // setRegister(regCopy.R52_Codec_PLL_N,R52_PLLN|R52_SDM_FRACTIONAL_MODE|R52_PLL_PRESCALE_DIV_1);
    // setRegister(regCopy.R4_Codec_Clocking1,R4_SYS_CLOCK_DIV_2|R4_CLOCK_FROM_PLL);
    // setRegister(regCopy.R8_Codec_Clocking2,0b111000000); //speaker class D?
    // if(audioConfig->sample_rate == 16000){
    //     setBitsHigh(regCopy.R4_Codec_Clocking1,R4_DIVIDER_ADC_SAMPLE_16KHZ);
    //     setBitsHigh(regCopy.R8_Codec_Clocking2,R8_BITCLOCK_DIVIDER_24);
    // } else if(audioConfig->sample_rate == 48000){
    //     setBitsHigh(regCopy.R4_Codec_Clocking1,R4_DIVIDER_ADC_SAMPLE_48KHZ);
    //     setBitsHigh(regCopy.R8_Codec_Clocking2,R8_BITCLOCK_DIVIDER_8);
    // }

    // setRegister(regCopy.R53_Codec_PLL_K_1,R53_PLLK_23_16);
    // setRegister(regCopy.R54_Codec_PLL_K_2,R54_PLLK_15_8);
    // setRegister(regCopy.R55_Codec_PLL_K_3,R55_PLLK_7_0);
    // //sets the audio control interface
    // setRegister(regCopy.R7_Codec_Audio_Interface1,R7_MODE_MASTER/*|R7_INVERT_BCLK*/|R7_AUDIO_WORD_LENGTH_16|R7_FORMAT_I2S|R7_MSB_AFTER_FIRST_EDGE|R7_SWAP_CHANNELS);
    // setRegister(regCopy.R48_Codec_Additional_Control4,0b000000000);
    // //set audio input registers
    // setRegister(regCopy.R0_Codec_Left_Input_Volume,R0_UPDATE_SOUND|R0_DEFAULT_VOLUME);
    // setRegister(regCopy.R1_Codec_Right_Input_Volume,R1_UPDATE_SOUND|R1_DEFAULT_VOLUME);
    // setRegister(regCopy.R21_Codec_Left_ADC_Volume,R21_LEFT_ADC_VOLUME);
    // setRegister(regCopy.R22_Codec_Right_ADC_Volume,R22_RIGHT_ADC_VOLUME);
    // setRegister(regCopy.R32_Codec_ADCL_Signal_Path,R32_ADCL_SIGNAL_PATH_LIN3|R32_ADCL_LMIC_BOOST/*|R32_CONNECT_TO_BOOST*/);
    // setRegister(regCopy.R33_Codec_ADCR_Signal_Path,R33_ADCR_SIGNAL_PATH_RIN3|R33_ADCR_RMIC_BOOST/*|R33_CONNECT_TO_BOOST*/);
    // setRegister(regCopy.R43_Codec_Input_Boost_Mixer1,R43_LEFT_BOOSTER_GAIN);
    // setRegister(regCopy.R44_Codec_Input_Boost_Mixer2,R44_RIGHT_BOOSTER_GAIN);
    // //set ALC 
    // setRegister(regCopy.R17_Codec_ALC1,/*R17_ALC_ENABLE|*/R17_ALC_MAX_GAIN|R17_ALC_TARGET);
    // setRegister(regCopy.R18_Codec_ALC2,R18_ALC_MINIMUM_GAIN|R18_ALC_HOLD_TIME);
    // setRegister(regCopy.R19_Codec_ALC3,R19_ALC_MODE_ALC|R19_ALC_ATTACK|R19_ALC_DECAY);
    // setRegister(regCopy.R20_Codec_Noise_Gate,R20_NOISE_THRESHOLD_ENABLE|R20_NOISE_THRESHOLD);
    // setRegister(regCopy.R27_Codec_Additional_control3,R27_ALC_SAMPLE_RATE_16);
    
    // writeRegisters(); //sends registers to codec



    /*
    ///////////////NOTES///////////////
    - enable an play with ALC
    - play with noise gate. 
    - figure why 
    */

    /*
    the codec is configured to always record with 16 khz stero, unless required to record in 48 khz.
    the settings are detemined beforehand which allows this function only to be called when the device is starting up.
    */
   
    //http://www.sunnyqi.com/upLoad/product/month_1306/WM8960.pdf
    regCopy.R0_Codec_Left_Input_Volume =            0b101011111; // update volume(8), disable mute(7) & volume (5:0)   
    regCopy.R1_Codec_Right_Input_Volume =           0b101011111; // same as line above (max volume)
    regCopy.R2_Codec_LOUT1_Volume =                 0b111111111; // max output volume
    regCopy.R3_Codec_ROUT1_Volume =                 0b111111111; // max output volume
    regCopy.R4_Codec_Clocking1 =                    0b000000101; // clock divider of 2(2:1), and SYSCLK of PLL(0) (sample rate will be done at end of this config)
    regCopy.R5_Codec_ADC_DAC_Control1 =             0b000001000; // adc normal polarity(6:5), enable high pass filter(0), dac soft mute enable
    regCopy.R6_Codec_ADC_DAC_Control2 =             0b000000000; // all default, change output volume inmediately
    regCopy.R7_Codec_Audio_Interface1 =             0b001000010; // no channel swap(8), no bitclk invert(7), MASTER MODE(6), no LRCLK invert(4), 16 bit word length(3:2), I2S format(1:0)
    regCopy.R8_Codec_Clocking2 =                    0b111000000; // class d speaker output (default, not used)(8:6), bitclk will be done at enf of this config
    regCopy.R9_Codec_Audio_interface2 =             0b000000000; // adclrc/gpio1  = ADCLRC(6), no 8 bit word(5), no adc/dac companding(4:1),no loopback(0)
    regCopy.R10_Codec_Left_Dac_Volume =             0b111111111; // update output volume(8), volume control = 0db(7:0)
    regCopy.R11_Codec_Right_Dac_Volume =            0b111111111; // same as line above
    regCopy.R16_Codec_3D_Conrtol =                  0b000000000; // all default, output is not used
    regCopy.R17_Codec_ALC1 =                        0b001111011; // ALC off(8:7), max gain +30DB (6:4), target (3:0)(max)
    regCopy.R18_Codec_ALC2 =                        0b100000000; // ALC min gain = -17.25 (6:4), ALC hold time = 0ms(3:0)
    regCopy.R19_Codec_ALC3 =                        0b000110100; // ALC mode = ALC(8), decay = 192ms(7:4), attack = 24ms(3:0)
    regCopy.R20_Codec_Noise_Gate =                  0b000000001; // Noise threshold = -76.5dBfs(7:3), enable noise gate(0)
    regCopy.R21_Codec_Left_ADC_Volume =             0b111000000; // adc volume default(7:0)
    regCopy.R22_Codec_Right_ADC_Volume =            0b111000000; // same as line above
    regCopy.R23_Codec_Adittional_control1 =         0b111000000; // enable thermal shutdown(8), vmid = low current(7:6)  
    regCopy.R24_Codec_Adittional_control2 =         0b000000000; // all default, might solve conflicting pullup and down on data signals
    regCopy.R25_Codec_Power_Manegement1 =           0b011111110; // vmid  = 2*50Kohm (8:7), vref up(6), analog pga power(5:4), adc power(3:2),micbias up(1),master clock enable(0)
    regCopy.R26_Codec_Power_Manegement2 =           0b001100001; // loutput and routput, rest disabled(8:1), PLL enabled(0)
    regCopy.R27_Codec_Additional_control3 =         0b000000000; // no outputs, all default, ALC sample rate will be done at end of this config
    regCopy.R28_Codec_Anti_Pop1 =                   0b000000000; // micbias from vmid(8)
    regCopy.R29_Codec_Anti_Pop2 =                   0b000000000; // no resistor on HP for capacitors(6)
    regCopy.R32_Codec_ADCL_Signal_Path =            0b100111000; // Linput1 connected to PGA(8:6), pga boost +29Db (5:4), pga connected to boost mixer(3)
    regCopy.R33_Codec_ADCR_Signal_Path =            0b100111000; // same as line above
    regCopy.R34_Codec_Left_Out_Mix =                0b001010000; // default values, not used
    regCopy.R37_Codec_Right_Out_Mix =               0b001010000; // default values, not used
    regCopy.R38_Codec_Mono_Out_Mix1 =               0b000000000; // default values, not used
    regCopy.R39_Codec_Mono_Out_Mix2 =               0b000000000; // default values, not used
    regCopy.R40_Codec_LOUT1_Volume =                0b000000000; // default values, not used
    regCopy.R41_Codec_ROUT2_Volume =                0b000000000; // default values, not used
    regCopy.R42_Codec_MONOOUT_Volume =              0b001000000; // default values, not used
    regCopy.R43_Codec_Input_Boost_Mixer1 =          0b000000000; // all unput boost on in2 and in3 off, otherwise they will also be routed trough the moost mixer (robo voice)
    regCopy.R44_Codec_Input_Boost_Mixer2 =          0b000000000; // same as line above
    regCopy.R45_Codec_Bypass1 =                     0b011010000; // bypass enabled(7), rest is default
    regCopy.R46_Cocec_Bypass2 =                     0b011010000; // same as line above
    regCopy.R47_Codec_Power_Manegement3 =           0b000111100; // left and right PGA enabled(5:4), left&right output mixer enable control
    regCopy.R48_Codec_Additional_Control4 =         0b000000010; // temprature sensor enabled(1), bias voltage = 0.9*avvd(0)
    regCopy.R49_Codec_Class_D_Control1 =            0b000110111; // no speaker output(7:6), all is default
    regCopy.R51_Codec_Class_D_Control3 =            0b010000000; // default values, speaker not used
    regCopy.R52_Codec_PLL_N =                       0b000101000; 
    regCopy.R53_Codec_PLL_K_1 =                     0b000110001;
    regCopy.R54_Codec_PLL_K_2 =                     0b000100110;
    regCopy.R55_Codec_PLL_K_3 =                     0b011101000;

    /*adjusting settings according to the sample rate*/
    /*the mono/stereo selection will be done in the recording task. codec will always output stereo! 16khz, unless required to record in 48khz*/
    if(audioConfig->sample_rate == 48000){
        setBitsHigh(regCopy.R4_Codec_Clocking1,R4_DIVIDER_ADC_SAMPLE_48KHZ);
        setBitsHigh(regCopy.R8_Codec_Clocking2,R8_BITCLOCK_DIVIDER_8);
        setBitsHigh(regCopy.R27_Codec_Additional_control3,R27_ALC_SAMPLE_RATE_48);        
    } else {  // recording task will make a slower sample rate by capturing every 2 samples
        setBitsHigh(regCopy.R4_Codec_Clocking1,R4_DIVIDER_ADC_SAMPLE_16KHZ);
        setBitsHigh(regCopy.R8_Codec_Clocking2,R8_BITCLOCK_DIVIDER_24);
        setBitsHigh(regCopy.R27_Codec_Additional_control3,R27_ALC_SAMPLE_RATE_16);        
    }
    if(audioConfig->swapChannels){ // determined by configuring device settings in main.cpp. in case mono needs to be recorded with the second channel
        setBitsHigh(regCopy.R7_Codec_Audio_Interface1,R7_SWAP_CHANNELS); 
    }
    writeRegisters();

}

void WM8960::writeRegisters(){                                                                                     
    ESP_LOGI(TAG, "writing out all Codec registers form local Copy to Codec using i2c");
    send_I2C_command(0xF,0x00); //reset the codec

    send_I2C_command(0, regCopy.R0_Codec_Left_Input_Volume);
    send_I2C_command(1, regCopy.R1_Codec_Right_Input_Volume);
    send_I2C_command(2, regCopy.R2_Codec_LOUT1_Volume);
    send_I2C_command(3, regCopy.R3_Codec_ROUT1_Volume);
    send_I2C_command(4, regCopy.R4_Codec_Clocking1);
    send_I2C_command(5, regCopy.R5_Codec_ADC_DAC_Control1);
    send_I2C_command(6, regCopy.R6_Codec_ADC_DAC_Control2);
    send_I2C_command(7, regCopy.R7_Codec_Audio_Interface1);
    send_I2C_command(8, regCopy.R8_Codec_Clocking2);
    send_I2C_command(9, regCopy.R9_Codec_Audio_interface2);
    send_I2C_command(10, regCopy.R10_Codec_Left_Dac_Volume);
    send_I2C_command(11, regCopy.R11_Codec_Right_Dac_Volume);
   // send_I2C_command(15, regCopy.R15_Codec_Reset);  //DONT UNCOMMENT! ;-P
    send_I2C_command(16, regCopy.R16_Codec_3D_Conrtol);
    send_I2C_command(17, regCopy.R17_Codec_ALC1);
    send_I2C_command(18, regCopy.R18_Codec_ALC2);
    send_I2C_command(19, regCopy.R19_Codec_ALC3);
    send_I2C_command(20, regCopy.R20_Codec_Noise_Gate);
    send_I2C_command(21, regCopy.R21_Codec_Left_ADC_Volume);
    send_I2C_command(22, regCopy.R22_Codec_Right_ADC_Volume);
    send_I2C_command(23, regCopy.R23_Codec_Adittional_control1);
    send_I2C_command(24, regCopy.R24_Codec_Adittional_control2);
    send_I2C_command(25, regCopy.R25_Codec_Power_Manegement1);
    send_I2C_command(26, regCopy.R26_Codec_Power_Manegement2);
    send_I2C_command(27, regCopy.R27_Codec_Additional_control3);
    send_I2C_command(28, regCopy.R28_Codec_Anti_Pop1);
    send_I2C_command(29, regCopy.R29_Codec_Anti_Pop2);
    send_I2C_command(32, regCopy.R32_Codec_ADCL_Signal_Path);
    send_I2C_command(33, regCopy.R33_Codec_ADCR_Signal_Path);
    send_I2C_command(34, regCopy.R34_Codec_Left_Out_Mix);
    send_I2C_command(37, regCopy.R37_Codec_Right_Out_Mix);
    send_I2C_command(38, regCopy.R38_Codec_Mono_Out_Mix1);
    send_I2C_command(39, regCopy.R39_Codec_Mono_Out_Mix2);
    send_I2C_command(40, regCopy.R40_Codec_LOUT1_Volume);
    send_I2C_command(41, regCopy.R41_Codec_ROUT2_Volume);
    send_I2C_command(42, regCopy.R42_Codec_MONOOUT_Volume);
    send_I2C_command(43, regCopy.R43_Codec_Input_Boost_Mixer1);
    send_I2C_command(44, regCopy.R44_Codec_Input_Boost_Mixer2);
    send_I2C_command(45, regCopy.R45_Codec_Bypass1);
    send_I2C_command(46, regCopy.R46_Cocec_Bypass2);
    send_I2C_command(47, regCopy.R47_Codec_Power_Manegement3);
    send_I2C_command(48, regCopy.R48_Codec_Additional_Control4);
    send_I2C_command(49, regCopy.R49_Codec_Class_D_Control1);
    send_I2C_command(51, regCopy.R51_Codec_Class_D_Control3);
    send_I2C_command(52, regCopy.R52_Codec_PLL_N);
    send_I2C_command(53, regCopy.R53_Codec_PLL_K_1);
    send_I2C_command(54, regCopy.R54_Codec_PLL_K_2);
    send_I2C_command(55, regCopy.R55_Codec_PLL_K_3);
}
void WM8960::setRegister(uint16_t &reg, uint16_t value){
    reg = value;
}                                      
void WM8960::setBitsHigh(uint16_t &reg, uint16_t mask){
    reg = reg | mask;
}                                                
void WM8960::setBitsLow(uint16_t &reg, uint16_t mask){
    reg = reg & (~mask); //hope i got this right
}                                                        
