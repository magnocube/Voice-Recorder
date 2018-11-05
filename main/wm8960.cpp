#include "wm8960.h"


void WM8960::send_I2C_command(uint8_t reg, uint16_t value){
    vTaskDelay(10/portTICK_PERIOD_MS);
    esp_err_t errAddr, errReg, errVal, espRc;
    
    uint8_t rByte = (reg << 1) | (value >> 8); // register + first bit
    uint8_t vByte = value  & 0x00FF; //last 8 bits 
    
    
    ESP_LOGI(TAG, "writing trough I2C (WM8960)... Register: %d  With data: %d",rByte>>1,vByte);

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
    
    setupI2S(); // init i2s driver
  

    micToHeadsetBypass(); //configuration example


}
void WM8960::setupI2S(){  //setup the i2s bus
  

    i2s_config = {
        .mode = static_cast<i2s_mode_t>(I2S_MODE_MASTER | I2S_MODE_RX),
        .sample_rate = audioConfig->sample_rate,
        .bits_per_sample = static_cast<i2s_bits_per_sample_t>(audioConfig->bits_per_sample),
        .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,  //default value, will be overwritten 
        .communication_format = static_cast<i2s_comm_format_t>(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB),
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1 , // high interrupt priority
        .dma_buf_count = I2S_BUFF_COUNT,
        .dma_buf_len = AUDIO_BUFFER_SIZE, // 512
        .use_apll = false,
        .fixed_mclk = 0
    };
    if(audioConfig->num_channels ==1){
        i2s_config.channel_format = I2S_CHANNEL_FMT_ONLY_LEFT;
    } else if(audioConfig->num_channels == 2){
        i2s_config.channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT;
    }
     pin_config = {
        .bck_io_num = pinout->i2s_BCLK,
        .ws_io_num = pinout->i2s_WS,
        .data_out_num = pinout->i2s_DOUT,
        .data_in_num = pinout->i2s_DIN
    };
	 //install and start i2s driver
	i2s_driver_install((i2s_port_t)CODEC_I2S_NUM, &i2s_config, 0, NULL);   //install and start i2s driver
    i2s_set_pin((i2s_port_t)CODEC_I2S_NUM, &pin_config);



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

    unsigned char *b = (unsigned char*) &value;
    int i, j;
    int size = 2;
    for (i=size-1;i>=0;i--)
    {
        for (j=8;j>=0;j--)
        {
            printf("%u", (b[i] >> j) & 1);
        }
    }
    printf(" ,(DEC) %d",value);
    printf("\n");

}

void WM8960::micToHeadsetBypass(){ //example config
    
    send_I2C_command(0x19,0b10000000); // vmid 2*250kohm
    send_I2C_command(0x19,0b10000000); // vmid 2*250kohm
    send_I2C_command(0x19,0b10000000); // vmid 2*250kohm
    send_I2C_command(0x19,0b10000000); // vmid 2*250kohm
    send_I2C_command(0x19,0b10000000); // vmid 2*250kohm
    send_I2C_command(0x19,0b10000000); // vmid 2*250kohm
    send_I2C_command(0x19,0b10000000); // vmid 2*250kohm
    send_I2C_command(0x19,0b10000000); // vmid 2*250kohm
    send_I2C_command(0x19,0b10000000); // vmid 2*250kohm
    send_I2C_command(0x19,0b10000000); // vmid 2*250kohm
   
    send_I2C_command(0x15,0x00); //reset ? 
    send_I2C_command(0x15,0x00); //reset2 ? 
    send_I2C_command(0x25,0x0F0);  
    send_I2C_command(0x26,0x060);
    send_I2C_command(0x32,0x000);
    send_I2C_command(0x33,0x000);  
    send_I2C_command(0x47,0x00c);  
    send_I2C_command(0x34,0x080);
    send_I2C_command(0x37,0x080);
    send_I2C_command(0x02,0x179);
    send_I2C_command(0x03,0x179);
    send_I2C_command(0x19,0b10000000); // vmid 2*250kohm
    
    //send_I2C_command(0x1A,0b000000001);  //NOTE... 9 bits //enable pll
    //send_I2C_command(0x04,0b000000001);  //NOTE... 9 bits //clock from pll
    //send_I2C_command(0x04,0b000000001);  //NOTE... 9 bits //pll power
}
