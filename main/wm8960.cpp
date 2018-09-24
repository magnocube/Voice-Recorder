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
        //ESP_LOGW(TAG,esp_err_to_name(espRc));
    } else {
         ESP_LOGI(TAG, "no error in writing i2c");
    }
    

}
WM8960::WM8960(esp_audio_config *audioC, SDCard *sd_card){
    SD = sd_card;
    audioConfig = audioC; 
//set all registers
    micToHeadsetBypass(); //configuration example


}

void WM8960::micToHeadsetBypass(){
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
    
    //send_I2C_command(0x1A,0b000000001);  //NOTE... 9 bits //enable pll
    //send_I2C_command(0x04,0b000000001);  //NOTE... 9 bits //clock from pll
    //send_I2C_command(0x04,0b000000001);  //NOTE... 9 bits //pll power
}
