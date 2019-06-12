#ifndef SDCard_H
#define SDCard_H
#pragma once

/*
    This class will have 2 functions.
    The first function is to accept and handle a file System on the SD card.
    The second function is to write out a WAV header when a file is closed.
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/unistd.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/sdmmc_host.h"
#include "sdmmc_cmd.h"
#include "esp_vfs_fat.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "esp_err.h"
#include <string.h>

#include "settings.h" 
#include "pca9535.h"
#include "Apresa.h"
#include "RV-1805-C3.h"



class SDCard{
    public:
        SDCard(esp_pin_config *pinC, esp_audio_config *audioC,pca9535 * gh, esp_session_data *sessionD, Apresa *apresa_connection_ptr); //constructor, will try to mount the SD card on initialising                                           
        bool isCardInSlot();                                    //will check if the SD card is in the slot for writing
        bool isWriteProtectOn();                                //checks if the write protection is on. (card must be in slot for this to give a valid return)
        bool isMounted();                                       //returns if the SD card is mounted to the file System
        esp_err_t mountCard();                                  //will attempt to mount the SD card to the file system
        void releaseCard();                                     //will demount the SD card from the file System
        esp_err_t beginFile(char name[]);                                  //will make a file on the file system and sets the offset for the header
        void generateNextFileName();                            //generate new file name and pit it in session data
        esp_err_t addDataToFile(uint8_t *data,int length);      //will add data to the file. (can be time consuming!)
        void endFile();                                         //write out the headers tho the beginning of the file and close the file
        void printCardInfo();  
        bool isCardMounted;                                     //public variable used by the function: isMounted();                         //print out basic info of the card. might come in handy if some specific card wont work  
    private:       
        void setupSDConfig();                                   //setsup the SD peripheral in 1-line SD mode.
        void writeWavHeader();                                  //generates the WAV header. Should be called upon closing the file.
        void handleSDProtectBlink();                            //handle SD protection. will also blink the red led if protection is on.
        sdmmc_host_t host;                                      //used for: setupSDConfig();
        sdmmc_slot_config_t slot_config;                        //used for: setupSDConfig();
        esp_pin_config *pinconfig;                              //pingconfig of the device. Gets passed from the constructor
        esp_audio_config *audioConfig;                          //audioconfig of the device. will have up-to-date settings for audio settings 
        esp_session_data *sessionData;
        pca9535 *gpio_header;
        Apresa *apresa;
        esp_vfs_fat_sdmmc_mount_config_t mount_config;          //used for: setupSDConfig();
        sdmmc_card_t* card;                                     //used for: mountCard();
        FILE* file;                                             //the file that will be written too.. start by calling: beginFile();
        


        /*Riffheader is used for the format of a .WAV file*/
        /*
            order":
                    -RiffHeader
                    -FmtSubChunk
                    -NoteSubChunk (optional, used for identifying to external devices)
                    -data

        */
        struct RiffHeader{
            char chunkID[4];                                    /* "RIFF"                                  */
            int chunkSize;                                      /* file length in bytes - 8 bytes          */
            char format[4];                                     /* "WAVE"                                  */
        };
        struct FmtSubChunk{
            char subChunkID[4];                                /* "fmt "                                  */
            int subChunkSize;                                  /* size of FMT chunk in bytes (usually 16) */
            short audioFormat;                                  /* 1=PCM, 257=Mu-Law, 258=A-Law, 259=ADPCM */
            short numChannels;                                  /* 1=mono, 2=stereo                        */
            int sampleRate;                                     /* Sampling rate in samples per second     */
            int byteRate;                                       /* bytes per second = srate*bytes_per_samp */
            short blockAlign;                                   /* 2=16-bit mono, 4=16-bit stereo          */
            short bitsPerSample;                                /* Number of bits per sample               */
        };
        struct UnknownDataButNeeded{
            char unknownHeader[24];
        };
        struct NoteSubChunk{   
            char subChunkID[4];                                 /* "note"                                  */
            int subChunkSize;                                   /* size of note chunk in bytes (52+8) */      
            char pre_ff[4];                               
            char archived_recording;                            /*"Z" for a archived recording*/
            char recorded_call;                                 /*"C" for a recorded call*/
            char year;                                          /*Year*/
            char month;                                         
            char day;
            char hour;
            char minutesHigh;                                   /*minute divided by 10*/
            char minutesLow;                                    /*minute modulo 10*/
            char seconds;                                       /*"0"-"9" = 0-18.    "A"-"T" = 20-58*/
            char additionalInformation[6];                      
            char format; //see note below;
            char SoftwareID[2];
            char recordingSource;
            char inOrOutgoing;
            char localNumber[16];
            char remoteNumber[16];  
            char connectedNumber[16];   
            char macAdress[12];
            char apresaChannelLicences[3];
            char pcChannelLicences[3];    
            char rest_ff[346];
        };
        struct DataHeader{
            char subChunkID[4];                                /* "data"                                  */
            int subChunkSize;                                  /* data length in bytes                    */
        };


        /*
        
        WAV Codec Format
        "6"          = G.711 A-Law Mono  , Non-encrypted
        "7"          = G.711 A-Law Stereo , Non-encrypted
        "8"          = PCM 16-bit Mono , Non-encrypted
        "9"          = PCM 16-bit Stereo , Non-encrypted
        "F"          = G.711 A-Law Mono , Encrypted with ECB
        "G"          = G.711 A-Law Stereo, Encrypted with ECB
        "H"          = PCM 16-bit Mono . Encrypted with ECB
        "I"          = PCM 16-bit Stereo ,Encrypted with ECB
        "V"          = G.711 A-Law Mono , Encrypted with CBC
        "W"          = G.711 A-Law Stereo. Encrypted with CBC
        "X"          = PCM 16-bit Mono . Encrypted with CBC
        "Y"          = PCM 16-bit Stereo . Encrypted with CBC
        
        */

        struct WavHeader{
            RiffHeader riffHeader;
            FmtSubChunk fmtSubChunk;
            UnknownDataButNeeded idk;
            NoteSubChunk noteSubChunk;
            DataHeader dataHeader;
        };


        // typedef struct {                                        //the struct to have a clear overview of the WAV header. this struct will be configured and written to the file in: SDCard::endFile();
        //     char ChunkID[4];                                    /* "RIFF"                                  */
        //     int ChunkSize;                                      /* file length in bytes - 8 bytes          */
        //     char Format[4];                                     /* "WAVE"                                  */
        //     char SubChunk1ID[4];                                /* "fmt "                                  */
        //     int SubChunk1Size;                                  /* size of FMT chunk in bytes (usually 16) */
        //     short AudioFormat;                                  /* 1=PCM, 257=Mu-Law, 258=A-Law, 259=ADPCM */
        //     short NumChannels;                                  /* 1=mono, 2=stereo                        */
        //     int SampleRate;                                     /* Sampling rate in samples per second     */
        //     int ByteRate;                                       /* bytes per second = srate*bytes_per_samp */
        //     short BlockAlign;                                   /* 2=16-bit mono, 4=16-bit stereo          */
        //     short BitsPerSample;                                /* Number of bits per sample               */
        //     char SubChunk2ID[4];                                /* "data"                                  */
        //     int SubChunk2Size;                                  /* data length in bytes (filelength - 44)  */
        // } wavHeader;
};




#endif // SDCard_H