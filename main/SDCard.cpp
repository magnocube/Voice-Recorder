#include "SDCard.h"



SDCard::SDCard(esp_pin_config *pinC){ 
    pinconfig = pinC;
    isCardMounted = false;
    setupSDConfig();
    if(isCardInSlot()){
        if(!isWriteProtectOn()){
            mountCard();
        }
        
    
    }
    
}
                                               
bool SDCard::isCardInSlot(){
    //check for voltage on CardDetectPin
    bool cardD = true;
    if(cardD){
        return true;
    }
    return false; 
}
esp_err_t SDCard::mountCard(){
    esp_err_t ret = esp_vfs_fat_sdmmc_mount("/sdcard", &host, &slot_config, &mount_config, &card);

    if (ret != ESP_OK) {
        isCardMounted = false;
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount filesystem, consider a card format(can be done in code,,, but needs to be inplemented");
        } else {
            ESP_LOGE(TAG, "Failed to initialize the card");
        }
    } else{
        ESP_LOGI(TAG, "SD card mouted succesfully!");
        isCardMounted = true;
    }
    return ret;
}
void SDCard::releaseCard(){
    
}
esp_err_t SDCard::beginFile(){
    ESP_LOGI(TAG, "Opening file");
    file = fopen("/sdcard/hello.txt", "w");
    fgetpos (file, &fileStartPosition);
    return ESP_OK;
}
esp_err_t SDCard::addDataToFile(char* data){
    fputs(data,file);
    return ESP_OK;
}   
void SDCard::endFile(){
    fsetpos (file, &fileStartPosition);
    fputs ("Header", file);
    fclose(file);
    ESP_LOGI(TAG, "File written..");
}
void SDCard::setupSDConfig(){
    ESP_LOGI(TAG, "initializing SDMMC peripheral");
    host = SDMMC_HOST_DEFAULT();
    slot_config = SDMMC_SLOT_CONFIG_DEFAULT();
    slot_config.width = 1;
    mount_config = {
        .format_if_mount_failed = true,
        .max_files = 5,
        .allocation_unit_size = 0 //default
    };

    gpio_set_pull_mode((gpio_num_t)pinconfig->sd_D0,GPIO_PULLUP_ONLY);
}
void SDCard::printCardInfo(){
    sdmmc_card_print_info(stdout, card);
}
bool SDCard::isMounted(){
    return isCardMounted;
}
bool SDCard::isWriteProtectOn(){
    //check for write protect pin
    return false;
}

