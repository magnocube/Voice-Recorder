#ifndef SDCard_H
#define SDCard_H
#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <sys/unistd.h>
#include "driver/gpio.h"
#include "driver/sdmmc_host.h"
#include "sdmmc_cmd.h"
#include "esp_vfs_fat.h"
#include "esp_log.h"
#include "esp_err.h"

#include "settings.h" 



class SDCard{
    public:
        SDCard(esp_pin_config *pinC);                                                  
        bool isCardInSlot();
        bool isWriteProtectOn();
        bool isMounted();
        esp_err_t mountCard();
        void releaseCard();
        esp_err_t beginFile();
        esp_err_t addDataToFile(char *data);
        void endFile();  
        void printCardInfo();  
    private:
        bool isCardMounted;
        void setupSDConfig();
        sdmmc_host_t host;
        sdmmc_slot_config_t slot_config;
        esp_pin_config *pinconfig;
        esp_vfs_fat_sdmmc_mount_config_t mount_config;
        sdmmc_card_t* card;
        FILE* file; 
        fpos_t fileStartPosition; //start to write header
        
};




#endif // SDCard_H