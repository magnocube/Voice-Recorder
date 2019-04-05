#pragma once

class Apresa{    
    public:
        Apresa();
        void setup();                    //setup of the socket( for when connected to wifi/ethernet)
        void sendFile();                 //sends an actual file (blocking)
        bool startSending;               //to indicate that it can start sending the file (so an other thread can send it)
        void setFileName(char* name);    //set the name of the file to send (might also happen internal in this class)

    private:
        char * fileName;                 // malloc in main
    

};