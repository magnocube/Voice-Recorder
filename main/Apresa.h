#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>
#include "esp_log.h"
#include "stdlib.h"
#include "string.h"

#pragma once
#include <stdlib.h>




#define HOST_IP_ADDR "192.168.2.32" //must be placed in a variable from settings.txt
#define PORT 2016					 // ""


static const char *payload = "Message from ESP32 ";


class Apresa{    
    public:
        Apresa();
        void setup();                    //setup of the socket( for when connected to wifi/ethernet)
        void sendFile();                 //sends an actual file (blocking, dont call fron task that should not be blocked, use: "startSending")
        void startSending();               //to indicate that it can start sending the file (so an other thread can send it), non blocking for the thread calling it
        bool isSending();                  //to let other tasks know that there is an active connection, will be used by apresa thread to start the connection
        void setFileName(char* name);    //set the name of the file to send (might also happen internal in this class)

    private:
        void connectTCP();
        void disconnectTCP();
        char * fileName;                 // malloc in main
        bool isSendingAFile;

        int sock;
        int err;
    

};