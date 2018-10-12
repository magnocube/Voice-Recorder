#include "pca9535.h"



pca9535::pca9535(esp_pin_config *pinconfig){
    pinout = pinconfig;
    for(int i = 0; i< 16; i++){
        data[i] = false;
        mode[i] = true; // OUTPUT   TODO,,, make a define out of this
    }
    data[7] = true; 
     
}
 uint16_t pca9535::getRawData(){
    uint16_t ret = 0;
    for(int i=0; i<16; i++){
         ret |= data[i] << i;
    }
    return ret;
}
void pca9535::redOff(){
    ESP_LOGI(TAG, "turning red off ");

}
void pca9535::redOn(){
    ESP_LOGI(TAG, "turning red on ");
}
void pca9535::writeOutData(){
    
}

