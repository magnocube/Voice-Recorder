#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>
#include "esp_log.h"
#include "stdlib.h"
#include "string.h"
#include "settings.h"
#include "driver/sdmmc_host.h"
#include "driver/sdspi_host.h"
#include "sdmmc_cmd.h"

#pragma once
#include <stdlib.h>
#include <stdio.h>




#define HOST_IP_ADDR "192.168.2.32" //must be placed in a variable from settings.txt
#define PORT 2016					 // ""





class Apresa{   
    
    public:
        Apresa(esp_session_data *sessionD);
        void setup();                    //setup of the socket( for when connected to wifi/ethernet)
        void sendFile();                 //sends an actual file (blocking, dont call fron task that should not be blocked, use: "startSending")
        void startSending();               //to indicate that it can start sending the file (so an other thread can send it), non blocking for the thread calling it
        bool isSending();                  //to let other tasks know that there is an active connection, will be used by apresa thread to start the connection
        void setFileName(int n);    //set the name of the file to send (might also happen internal in this class)
        void updateApresa();              //updates the local files to the apresa

    private:



        struct TunnelHeader{                        //Byte Index
            uint8_t sync[4];                        //0-3
            uint8_t version;                        //4
            uint8_t controlFlags;                   //5
            uint8_t last2MacBytes[2];               //6-7
            uint8_t fileLength[4];                  //8-11
            uint8_t reserved[4] = {0,0,0,0};                    //12-15
            uint8_t macAdress[6];                  //16-21
            uint8_t apresaChannelLicences[2];       //22-23
            uint8_t crVoipChannelLicences[2];       //24-25
            uint8_t recordingChannels[2];           //26-27
            uint8_t reserved2[4] = {0,0,0,0};                   //28-31
        };

        void connectTCP();
        void disconnectTCP();
        
        esp_session_data *sessionData;
        char * fileName;                 // malloc in main
        bool isSendingAFile;
        FILE* file; 
        int sock;
        int err;
    

};