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
    espRc = i2c_master_cmd_begin(I2C_NUM_0, cmd, 100/portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);

    if(errAddr != ESP_OK){
        ESP_LOGW(TAG, "error in I2C writing address to command");
    } else if(errReg != ESP_OK){
        ESP_LOGW(TAG, "error in I2C writing register to command");
    } else if(errVal != ESP_OK){
        ESP_LOGW(TAG, "error in I2C writing value to command");
    } else if(espRc != ESP_OK){
        ESP_LOGW(TAG, "error in I2C writing the command to WM8960" );
         //ESP_ERROR_CHECK( espRc);
      
        //ESP_LOGW(TAG,esp_err_to_name(espRc));
    } else {
        //uncomment line below to have log feedback that there was no error.
        // ESP_LOGI(TAG, "no error in writing i2c");
    }
    

}
WM8960::WM8960(esp_audio_config *audioC, SDCard *sd_card, pca9535 *gpioHeader,esp_pin_config *pinconfig){
    SD = sd_card;
    audioConfig = audioC; 
    gpio_header = gpioHeader;
    pinout = pinconfig;

    audioBuffer1 = (uint8_t*)malloc(AUDIO_BUFFER_SIZE);
    
    setupI2S(); // init i2s driver
    printf("value of i2s sample rate:   %d ", i2s_config.sample_rate);
    //set all registers
    micToHeadsetBypass(); //configuration example


}
void WM8960::setupI2S(){  //setup the i2s bus
  

    i2s_config = {
        .mode = static_cast<i2s_mode_t>(I2S_MODE_MASTER | I2S_MODE_RX),
        .sample_rate = audioConfig->sample_rate,
        .bits_per_sample = static_cast<i2s_bits_per_sample_t>(audioConfig->bits_per_sample),
        .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,  //default value, will be overwritten 
        .communication_format = static_cast<i2s_comm_format_t>(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB),
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1, // high interrupt priority
        .dma_buf_count = 3,
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

}

void WM8960::micToHeadsetBypass(){
    vTaskDelay(10000/portTICK_PERIOD_MS);
    send_I2C_command(0x19,0b10000000); // vmid 2*250kohm
    send_I2C_command(0x19,0b10000000); // vmid 2*250kohm
   
    // send_I2C_command(0x15,0x00); //reset ? 
    // send_I2C_command(0x15,0x00); //reset2 ? 
    // send_I2C_command(0x25,0x0F0);  
    // send_I2C_command(0x26,0x060);
    // send_I2C_command(0x32,0x000);
    // send_I2C_command(0x33,0x000);  
    // send_I2C_command(0x47,0x00c);  
    // send_I2C_command(0x34,0x080);
    // send_I2C_command(0x37,0x080);
    // send_I2C_command(0x02,0x179);
    // send_I2C_command(0x03,0x179);
    
    //send_I2C_command(0x1A,0b000000001);  //NOTE... 9 bits //enable pll
    //send_I2C_command(0x04,0b000000001);  //NOTE... 9 bits //clock from pll
    //send_I2C_command(0x04,0b000000001);  //NOTE... 9 bits //pll power
}
