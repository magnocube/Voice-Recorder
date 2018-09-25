#include "pca9535.h"



pca9535::pca9535(esp_pin_config *pinconfig){
    pinout = pinconfig;
    for(int i = 0; i< 16; i++){
        data[i] = false;
    }
    data[7] = true; 
    //data[10] = true; 
}
 uint16_t pca9535::getRawData(){
    uint16_t ret = 0;
    for(int i=0; i<16; i++){
         ret |= data[i] << i;
    }
    return ret;
}

