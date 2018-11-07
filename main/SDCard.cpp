#include "SDCard.h"



SDCard::SDCard(esp_pin_config *pinC,esp_audio_config * audioC,pca9535 * gh){ 
    pinconfig = pinC;
    audioConfig = audioC;
    
    gpio_header = gh;

    setupSDConfig();
    isCardMounted = false;
    // mountCard(); //try to mount te card by default, it will also get mounted every time
    
}
                                               
bool SDCard::isCardInSlot(){
    //check for voltage on CardDetectPin
    int one = gpio_header->digitalRead(pinconfig->sdDetect,true);
	int two = gpio_header->digitalRead(pinconfig->sdProtect,false);
    if(one == false && two == false){
        return true;
    }else{
        return false;
    }
    // printf("pins:  %d   %d", pinconfig->sdDetect, pinconfig->sdProtect);
    // printf("readstatusses: %d ,  %d ",one, two);
}

void SDCard::releaseCard(){
    
}
esp_err_t SDCard::beginFile(){
   
    ESP_LOGI(TAG, "Opening file");
    file = fopen("/sdcard/sixtyS.wav", "w");
    ESP_LOGI(TAG, "file opened");
    return ESP_OK;
}
esp_err_t SDCard::addDataToFile(uint8_t* data,int length){
    fwrite(data,sizeof(uint8_t),length,file);
    return ESP_OK;
}   
void SDCard::endFile(){
       
    ESP_LOGI(TAG, "File written...");
    generateWavHeader();
    fclose(file);
    
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
    gpio_set_pull_mode((gpio_num_t)pinconfig->sd_CLK,GPIO_PULLUP_ONLY);
    gpio_set_pull_mode((gpio_num_t)pinconfig->sd_CMD,GPIO_PULLUP_ONLY);
    
}
void SDCard::printCardInfo(){
    sdmmc_card_print_info(stdout, card);
}
esp_err_t SDCard::mountCard(){
    gpio_header->digitalWrite(pinconfig->sdPower,PCA_LOW,true); //enable power
    //vTaskDelay(300/portTICK_PERIOD_MS);
    isCardMounted = false; //default false 
    
    esp_err_t ret = esp_vfs_fat_sdmmc_mount("/sdcard", &host, &slot_config, &mount_config, &card);

    if (ret != ESP_OK) {
        isCardMounted = false;
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount filesystem, consider a card format(can be done in code,,, but needs to be inplemented");
        } else {
            ESP_LOGE(TAG, "Failed to initialize the card: %d", ret);
        }
    } else{
        ESP_LOGI(TAG, "SD card mouted succesfully!");
        isCardMounted = true;
    }
    return ret;
}
bool SDCard::isMounted(){   //NOTE: there is also a public variable called "isCardMounted", 
                            //      this variable keeps track of the latest state od the SD since the last check 
                            //                    ESP_LOGW(TAG, "IsMount called, ");
    
    if(isCardInSlot()){   
    //ESP_LOGW(TAG, "card is in slot, ");
        if(isCardMounted == false){ //the card was not mounted before
        //ESP_LOGW(TAG, "ismounted = false ");
            mountCard();            //mount the card.. also updates the variable "isCardMounted"
            //ESP_LOGW(TAG, "MountCard called, ");
            if(isCardMounted){
                //ESP_LOGW(TAG, "Now true");
                
                gpio_header->digitalWrite(pinconfig->led_red,PCA_HIGH,true);
                return true;
            } else{
                // ESP_LOGW(TAG, "still false ");
                
                return false;
            }
        } else {  // card should be mounted
            //ESP_LOGI(TAG, "should be mounted");
            //ESP_LOGW(TAG, "should be mounted");
            return true;           
        }
    } else{                            // card is not in the cardholder
    //ESP_LOGW(TAG, "not in holder, ");
        if(isCardMounted == true) {  //the card was just removed from the cardholder
            vTaskDelay(20/portTICK_RATE_MS);
            if(!isCardInSlot()){     //do a double check!.. (can be removed later, trying to fix a bug with i2c)
                isCardMounted = false;
                ESP_LOGW(TAG, "SD card was removed from slot... possible data loss");
                printf(esp_err_to_name(esp_vfs_fat_sdmmc_unmount()));
                //ESP_LOGI(TAG, "Card unmounted");
                gpio_header->digitalWrite(pinconfig->sdPower,PCA_HIGH,false); //disable power
                gpio_header->digitalWrite(pinconfig->led_red,PCA_LOW,true);
                return false;
            } else{
                return true;
            }
        } else { //the card is out for a while
            // ESP_LOGI(TAG, "not mounted");
            //ESP_LOGW(TAG, "not mounted, ");
            isCardMounted = false;
            return false;
        }
    }
    
    return false;
}
bool SDCard::isWriteProtectOn(){
    //check for write protect pin
    return false;
}



void SDCard::generateWavHeader()
{
    wavHeader wavh;                //struct... can be found in SDCard.h 
    int size = ftell(file);       //size of the file at the moment of writing the wav header
    
    /*generating the WAV header based on the file size and audio quallity*/
    strncpy(wavh.ChunkID,"RIFF",4);
    wavh.ChunkSize = size - 8;
    strncpy(wavh.Format,"WAVE",4);
    strncpy(wavh.SubChunk1ID,"fmt ",4);
    wavh.SubChunk1Size = 16;
    wavh.AudioFormat = 1; 
    wavh.NumChannels = audioConfig->num_channels;
    wavh.SampleRate = audioConfig->sample_rate;
    wavh.ByteRate = audioConfig->sample_rate * audioConfig->num_channels * audioConfig->bits_per_sample / 8;
    wavh.BlockAlign = audioConfig->num_channels * audioConfig->bits_per_sample / 8;
    wavh.BitsPerSample = audioConfig->bits_per_sample;
    strncpy(wavh.SubChunk2ID,"data",4);
    wavh.SubChunk2Size = size-44;    
    
    /*jump to the start of the file and overwrite the header*/
    fseek(file,0,SEEK_SET);
    fwrite(&wavh,sizeof(char),WAV_HEADER_SIZE,file);
   
    printf("file written with: sample rate      : %d\n", wavh.SampleRate);
    printf("                 : num channels     : %d\n", wavh.NumChannels);
    printf("                 : byte rate        : %d\n", wavh.ByteRate);
    printf("                 : BlockAlign       : %d\n", wavh.BlockAlign);
    printf("                 : bits_per_sample  : %d\n", wavh.BitsPerSample);
    printf("Size of the written file it bytes: %d\n", size);


}