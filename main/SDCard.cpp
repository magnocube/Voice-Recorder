#include "SDCard.h"



SDCard::SDCard(esp_pin_config *pinC,esp_audio_config * audioC,pca9535 * gh, esp_session_data *sessionD){ 
    pinconfig = pinC;
    audioConfig = audioC;
    sessionData = sessionD;
    gpio_header = gh;

    setupSDConfig();
    isCardMounted = false;
    // mountCard(); //try to mount te card by default, it will also get mounted every time
    
}
                                               
bool SDCard::isCardInSlot(){
    //check for voltage on CardDetectPin
    int one = gpio_header->digitalRead(pinconfig->sdDetect,true);
	//int two = gpio_header->digitalRead(pinconfig->sdProtect,false);
    if(one == false/* && two == false*/){ //low
        return true;
    }else{
        return false;
    }
    // printf("pins:  %d   %d", pinconfig->sdDetect, pinconfig->sdProtect);
    // printf("readstatusses: %d ,  %d ",one, two);
}

void SDCard::releaseCard(){
    
}
esp_err_t SDCard::beginFile(char name[]){
   
    ESP_LOGI(TAG, "Opening file");
    file = fopen(name, "w");
    ESP_LOGI(TAG, "file opened");

    //printing zero's so the header will not be in the audio data
    // char * data = (char*)malloc(512);
    // memset (data,0,512);
    // fwrite(data,sizeof(uint8_t),512,file);
    // delete data;

    return ESP_OK;
}
esp_err_t SDCard::addDataToFile(uint8_t* data,int length){
    fwrite(data,sizeof(uint8_t),length,file);
    return ESP_OK;
}   

//write out the headers and close the file
void SDCard::endFile(){
       
    ESP_LOGI(TAG, "File written...");
    writeWavHeader();  // will also include the "note" field
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
    
    //external pull-ups are on the hardware, this is just an extra precaution
    gpio_set_pull_mode((gpio_num_t)pinconfig->sd_D0,GPIO_PULLUP_ONLY);
    gpio_set_pull_mode((gpio_num_t)pinconfig->sd_CLK,GPIO_PULLUP_ONLY);
    gpio_set_pull_mode((gpio_num_t)pinconfig->sd_CMD,GPIO_PULLUP_ONLY);
    
}
void SDCard::printCardInfo(){
    sdmmc_card_print_info(stdout, card);
}
esp_err_t SDCard::mountCard(){
    gpio_header->digitalWrite(pinconfig->sdPower,PCA_LOW,true); //enable power
    vTaskDelay(300/portTICK_PERIOD_MS); //give SD card some time
    isCardMounted = false; //default false 
    
    esp_err_t ret = esp_vfs_fat_sdmmc_mount("/sdcard", &host, &slot_config, &mount_config, &card);

    if (ret != ESP_OK) {
        //printf(esp_err_to_name(esp_vfs_fat_sdmmc_unmount()));
        isCardMounted = false;
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount filesystem, consider a card format(can be done in code,,, but needs to be inplemented");
        } else {
            ESP_LOGE(TAG, "Failed to initialize the card: %d", ret);
            gpio_header->digitalWrite(pinconfig->sdPower,PCA_HIGH,true); //disable power
            vTaskDelay(100/portTICK_PERIOD_MS); //halt the sd card to make sure all power is off (and it is not restarted withon 100 ms)
        }
    } else{
        ESP_LOGI(TAG, "SD card mouted succesfully!");
        isCardMounted = true;
        handleSDProtectBlink();
    }
    return ret;
}
void SDCard::handleSDProtectBlink(){
    if(gpio_header->digitalRead(pinconfig->sdProtect,false) == true){ //HIGH,,, so write protect is enabled
            sessionData->SD_Write_Protect_on = true;
            gpio_header->digitalWrite(pinconfig->led_red,!gpio_header->digitalRead(pinconfig->led_red,false),true); // turn led off
    } else {
            sessionData->SD_Write_Protect_on = false;
    }
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
                
                gpio_header->digitalWrite(pinconfig->led_red,PCA_HIGH,true); // turn led off
                
                
                return true;
            } else{
                // ESP_LOGW(TAG, "still false ");
                
                return false;
            }
        } else {  // card should be mounted
            //ESP_LOGI(TAG, "should be mounted");
            //ESP_LOGW(TAG, "should be mounted");
            handleSDProtectBlink(); 
            return true;           
        }
    } else{                            // card is not in the cardholder
    //ESP_LOGW(TAG, "not in holder, ");
        if(isCardMounted == true) {  //the card was just removed from the cardholder
            vTaskDelay(20/portTICK_RATE_MS);
                 //do a double check!.. (can be removed later, trying to fix a bug with i2c)
                isCardMounted = false;
                ESP_LOGW(TAG, "SD card was removed from slot... possible data loss");
                printf(esp_err_to_name(esp_vfs_fat_sdmmc_unmount()));
                //ESP_LOGI(TAG, "Card unmounted");
                gpio_header->digitalWrite(pinconfig->sdPower,PCA_HIGH,false); //disable power
                gpio_header->digitalWrite(pinconfig->led_red,PCA_LOW,true);
                return false;
            
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



void SDCard::writeWavHeader() // will also include the note header
{
   

    int size = ftell(file);                                                     //size of the file in bytes. Used for determining the header Size values

    /*wav header, contains 4 subchunks, see header for details*/
    WavHeader wavHeader;

    /*RIFF header*/
    strncpy(wavHeader.riffHeader.chunkID,"RIFF",4);
    wavHeader.riffHeader.chunkSize = size - 8; 
    strncpy(wavHeader.riffHeader.format,"WAVE",4);

    /*FMT subChunk*/
    strncpy(wavHeader.fmtSubChunk.subChunkID,"fmt ",4);
    wavHeader.fmtSubChunk.subChunkSize = 16;
    wavHeader.fmtSubChunk.audioFormat = 1;
    wavHeader.fmtSubChunk.numChannels = audioConfig->num_channels;
    wavHeader.fmtSubChunk.sampleRate = audioConfig->sample_rate;
    wavHeader.fmtSubChunk.byteRate = audioConfig->sample_rate * audioConfig->num_channels * audioConfig->bits_per_sample / 8;
    wavHeader.fmtSubChunk.blockAlign = audioConfig->num_channels * audioConfig->bits_per_sample / 8;
    wavHeader.fmtSubChunk.bitsPerSample = audioConfig->bits_per_sample;

    /*IDK,,, needs to be here apparently*/
    wavHeader.idk.unknownHeader[0] = 0x66;
    wavHeader.idk.unknownHeader[1] = 0x61;
    wavHeader.idk.unknownHeader[2] = 0x63;
    wavHeader.idk.unknownHeader[3] = 0x74;
    wavHeader.idk.unknownHeader[4] = 0x04;
    wavHeader.idk.unknownHeader[5] = 0x00;
    wavHeader.idk.unknownHeader[6] = 0x00;
    wavHeader.idk.unknownHeader[7] = 0x00;
    wavHeader.idk.unknownHeader[8] = 0x00;
    wavHeader.idk.unknownHeader[9] = 0x1D;
    wavHeader.idk.unknownHeader[10] = 0x08;
    wavHeader.idk.unknownHeader[11] = 0x00;
    wavHeader.idk.unknownHeader[12] = 0x4C;
    wavHeader.idk.unknownHeader[13] = 0x49;
    wavHeader.idk.unknownHeader[14] = 0x53;
    wavHeader.idk.unknownHeader[15] = 0x54;
    wavHeader.idk.unknownHeader[16] = 0xC0;
    wavHeader.idk.unknownHeader[17] = 0x01;
    wavHeader.idk.unknownHeader[18] = 0x00;
    wavHeader.idk.unknownHeader[19] = 0x00;
    wavHeader.idk.unknownHeader[20] = 0x61;
    wavHeader.idk.unknownHeader[21] = 0x64;
    wavHeader.idk.unknownHeader[22] = 0x74;
    wavHeader.idk.unknownHeader[23] = 0x6C;


    /*note subChunk*/
    strncpy(wavHeader.noteSubChunk.subChunkID,"note",4);
    wavHeader.noteSubChunk.pre_ff[0] = 0xff;
    wavHeader.noteSubChunk.pre_ff[1] = 0xff;
    wavHeader.noteSubChunk.pre_ff[2] = 0xff;
    wavHeader.noteSubChunk.pre_ff[3] = 0xff;
    wavHeader.noteSubChunk.subChunkSize = 436;           //needs to be checked!
    wavHeader.noteSubChunk.archived_recording = 'Z';
    wavHeader.noteSubChunk.recorded_call = 'C';
    wavHeader.noteSubChunk.year = 'J';              // 2019

    // wavHeader.noteSubChunk.month = '3';             // maart
    // wavHeader.noteSubChunk.day = '1';               //1
    // wavHeader.noteSubChunk.hour = 'N';              //23
    // wavHeader.noteSubChunk.minutesHigh = '5';       //50
    // wavHeader.noteSubChunk.minutesLow = '9';        //9  -> total of 59

    sprintf(&wavHeader.noteSubChunk.month, "%d", esp_random()%10);
    sprintf(&wavHeader.noteSubChunk.day, "%d", esp_random()%10);
    sprintf(&wavHeader.noteSubChunk.hour, "%d", esp_random()%10);
    sprintf(&wavHeader.noteSubChunk.minutesHigh, "%d", esp_random()%10);
    sprintf(&wavHeader.noteSubChunk.minutesLow, "%d", esp_random()%10);


    wavHeader.noteSubChunk.seconds = 'T';           // 58
    strncpy(wavHeader.noteSubChunk.additionalInformation,"______",6);
    wavHeader.noteSubChunk.format = '9';            //PCM 16 bit stereo
    strncpy(wavHeader.noteSubChunk.SoftwareID,"QP",2);
    wavHeader.noteSubChunk.recordingSource = 'A';
    wavHeader.noteSubChunk.inOrOutgoing ='I';
    strncpy(wavHeader.noteSubChunk.localNumber,"VVOICE__________",16);
    strncpy(wavHeader.noteSubChunk.remoteNumber,"________________",16);
    strncpy(wavHeader.noteSubChunk.connectedNumber,"________________",16);
    strncpy(wavHeader.noteSubChunk.macAdress,(char*)sessionData->macAdressString,12);
    strncpy(wavHeader.noteSubChunk.apresaChannelLicences,"001",3);
    strncpy(wavHeader.noteSubChunk.pcChannelLicences,"001",3);
    for(int i =0;i<(346);i++){
        wavHeader.noteSubChunk.rest_ff[i] = 0x00;
    }

    /*data subChunk*/
    strncpy(wavHeader.dataHeader.subChunkID,"data",4);
    wavHeader.dataHeader.subChunkSize = size-138;    //riff = 12.    fmt  = 24.      note = 94   .. dataheader = 8 (total 138)





    /*jump to the start of the file and overwrite the header*/
   
    fseek(file,0,SEEK_SET);
    fwrite(&wavHeader,sizeof(wavHeader),1,file);

    printf("file written with: sample rate      : %d\n", wavHeader.fmtSubChunk.sampleRate);
    printf("                 : num channels     : %d\n", wavHeader.fmtSubChunk.numChannels);
    printf("                 : byte rate        : %d\n", wavHeader.fmtSubChunk.byteRate);
    printf("                 : BlockAlign       : %d\n", wavHeader.fmtSubChunk.blockAlign);
    printf("                 : bits_per_sample  : %d\n", wavHeader.fmtSubChunk.bitsPerSample);
    printf("Size of the written file it bytes   : %d\n", size);
    
    

    


}
/*This function will generate a new file name based on the value of file counter. the file counter is stored in NVS, and needs to be incremented before use*/
void SDCard::generateNextFileName(){
    	
  

    nvs_handle my_NVS_handle;
    esp_err_t err;

    printf("Opening Non-Volatile Storage (NVS) handle... ");
    
    err = nvs_open("storage", NVS_READWRITE, &my_NVS_handle);   
    if (err != ESP_OK) {
        printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
    } else {
        printf("Done\n");

        // Read
        printf("Reading file counter from NVS ... ");
        int32_t file_counter = 0; // value will default to 0, if not set yet in NVS
        

        err = nvs_get_i32(my_NVS_handle, "file_counter", &file_counter);
        switch (err) {
            case ESP_OK:
                printf("Done\n");
                printf("file counter = %d\n", file_counter);
                break;
            case ESP_ERR_NVS_NOT_FOUND:
                printf("The file counter is not initialized yet, the counter will be put back to 0!\n");
                break;
            default :
                printf("Error (%s) reading!\n", esp_err_to_name(err));
        }

        // Write
                           
        printf("Updating file counter in NVS ... ");
        file_counter++;
        err = nvs_set_i32(my_NVS_handle, "file_counter", file_counter);
        printf((err != ESP_OK) ? "Failed!\n" : "Done\n");
                           
        // Commit written value.
        // After setting any values, nvs_commit() must be called to ensure changes are written
        // to flash storage. Implementations may write to storage at other times,
        // but this is not guaranteed.
       
            
        int32_t file_counter_copy = file_counter;
        int count = 0;
        while(file_counter_copy != 0)   // little loop in order to count the 0's in front of the file name.
        {
            file_counter_copy /= 10;
            ++count;
        } 
        
        
        char name[FILE_NAME_LENGTH +20] = "/sdcard/";   //FILE_NAME_LENGTH = the length of the file on the sd card.. the +20 is to add a path to it
        for(int i = 0; i< FILE_NAME_LENGTH - count; i++){ //write out the 0s before the file number
             strcat(name,"0");        
        } 
        char numbers[FILE_NAME_LENGTH+1];
                                
        itoa(file_counter,numbers,10);
      

        strcat(name,numbers);
        strcat(name,".wav");
        strcpy(sessionData->last_file_name,name);

        printf("generated name that will be used for the file: ");
        printf(name);
        printf("\n");




        printf("Committing updates in NVS ... ");
        err = nvs_commit(my_NVS_handle);
        printf((err != ESP_OK) ? "Failed!\n" : "Done\n");
        // Close
         nvs_close(my_NVS_handle); 
     

    }
    
    printf("\n");
}
