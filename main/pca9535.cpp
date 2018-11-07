#include "pca9535.h"



pca9535::pca9535(esp_pin_config *pinconfig){
    pinout = pinconfig;
    for(int i = 0; i< 16; i++){  //default settings
        writeData[i] = PCA_LOW;
        readData[i] = PCA_LOW;
        mode[i] = PCA_INPUT;         
    }
}
 uint16_t pca9535::getRawWriteData(){
    uint16_t ret = 0;
    for(int i=0; i<16; i++){
         ret |= writeData[i] << i;
    }
    return ret;
}
 uint16_t pca9535::getRawConfigData(){
    uint16_t ret = 0;
    for(int i=0; i<16; i++){
         ret |= mode[i] << i;
    }
    return ret;
}
void pca9535::pinMode(int pin, int m,bool flush){
    mode[pin] = m;
    if(flush){
        writeConfigToI2C();
    }
}
void pca9535::digitalWrite(int pin, int mode,bool flush){
    writeData[pin] = mode;
    if(flush){
        writeDataToI2C();
    }
}
bool pca9535::digitalRead(int pin, bool update){
    if(update){
        readDataFromI2C();
    }    
    return readData[pin];
}
void pca9535::writeDataToI2C(){
    uint16_t dataToSend = getRawWriteData();
    //ESP_LOGI(TAG, "writing trough I2C (PCA9535)... data: %d",dataToSend);

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd,(PCA_I2C_ADDR << 1) | I2C_MASTER_WRITE, true /* expect ack */);     // codec adress + write
    i2c_master_write_byte(cmd,2,true); // command byte 2
    i2c_master_write_byte(cmd,dataToSend,true);
    i2c_master_write_byte(cmd,dataToSend >> 8,true);
    i2c_master_stop(cmd);
    esp_err_t espRc = i2c_master_cmd_begin(I2C_DRIVER_NUM, cmd, 100/portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);

    if(espRc != ESP_OK){
        ESP_LOGW(TAG, "error in I2C writing the command to PCA9535");
    } 
    
}
void pca9535::readDataFromI2C(){
 // ESP_LOGI(TAG, "reading trough I2C (PCA9535)... ");
  uint8_t firstHalf;
  uint8_t secondHalf;

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd,(PCA_I2C_ADDR << 1) | I2C_MASTER_WRITE, true /* expect ack */);     // codec adress + write
    i2c_master_write_byte(cmd,0,true); // command byte 2

    i2c_master_start(cmd);
    i2c_master_write_byte(cmd,(PCA_I2C_ADDR << 1) | 0x01, true /* expect ack */); 
    i2c_master_read_byte(cmd,&firstHalf,I2C_MASTER_ACK);
    i2c_master_read_byte(cmd,&secondHalf,I2C_MASTER_NACK);   
    i2c_master_stop(cmd);

    esp_err_t espRc = i2c_master_cmd_begin(I2C_NUM_0, cmd, 10/portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);

    if(espRc != ESP_OK){
        ESP_LOGW(TAG, "error in I2C writing the command to PCA9535 (while reading), %d \n" , espRc);
        
    } else{
        // ESP_LOGI(TAG, "data while reading: 1e:  %d  and 2e:  %d",firstHalf,secondHalf);
      
        
        for(int i = 0; i < 8; i++){
            readData[i] = firstHalf >> i && 0x01;
            readData[i+8] = secondHalf >> i && 0x01;
            // printf(" %d ",readData[2*i]);
            // printf(" %d ",readData[(2*i)+1]);
        }
    }
}
void pca9535::writeConfigToI2C(){
    uint16_t dataToSend = getRawConfigData();
    ESP_LOGI(TAG, "writing trough I2C (PCA9535)... config: %d",dataToSend);

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd,(PCA_I2C_ADDR << 1) | I2C_MASTER_WRITE, true /* expect ack */);     // codec adress + write
    i2c_master_write_byte(cmd,6,true); // command byte 6
    i2c_master_write_byte(cmd,dataToSend,true);
    i2c_master_write_byte(cmd,dataToSend >> 8,true);
    i2c_master_stop(cmd);
    esp_err_t espRc = i2c_master_cmd_begin(I2C_NUM_0, cmd, 100/portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);

    if(espRc != ESP_OK){
        ESP_LOGW(TAG, "error in I2C writing the command to PCA9535");
    } 
    
}
