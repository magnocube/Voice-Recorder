#include "Apresa.h"
#define TAG "apresaTunnelTest"

Apresa::Apresa(esp_session_data *sessionD){  //normal constructor
    fileName = (char*)malloc(9 +20);  //the 20 is for the path that needs to be added, the 9 is for the file path length
    sessionData = sessionD;
}

void Apresa::setup(){  //for setting up any other things
    
}

// esp_err_t SDCard::beginFile(char name[]){
   
//     ESP_LOGI(TAG, "Opening file");
//     file = fopen(name, "w");
//     ESP_LOGI(TAG, "file opened");

//     //printing zero's so the header will not be in the audio data
//     // char * data = (char*)malloc(512);
//     // memset (data,0,512);
//     // fwrite(data,sizeof(uint8_t),512,file);
//     // delete data;

//     return ESP_OK;
// }
// esp_err_t SDCard::addDataToFile(uint8_t* data,int length){
//     fwrite(data,sizeof(uint8_t),length,file);
//     return ESP_OK;
// }   

// //write out the headers and close the file
// void SDCard::endFile(){
       
//     ESP_LOGI(TAG, "File written...");
//     writeWavHeader();  // will also include the "note" field
//     fclose(file);
    
// }


void Apresa::setFileName(int n){  //will set the name used by the "sendFile()" function
          
    int32_t file_counter_copy = n;  //file counter comes from the SD card classs. this is just a quick copy paste
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
    itoa(n,numbers,10);
    
   

    strcat(name,numbers);
    strcat(name,".wav");
    strcpy(fileName,name);  //store it in the class.. variabel name will get out of scope
    
}
void Apresa::sendFile(){  //sends the file (name of file is in "filename")
   
    //file pointer
    ESP_LOGI(TAG,"sending file %s to apresa.",fileName);



    file = fopen(fileName, "r+");
    if (file == NULL) {
        ESP_LOGE(TAG, "Failed to open file for reading, will return");
        return;
    }    
    fseek(file,0,SEEK_END);
    uint32_t fileSize = ftell(file); 
    fseek(file,0,SEEK_SET);

    ESP_LOGI(TAG,"Size of the file to send: %d",fileSize);
  



    //creating the header
    TunnelHeader t;
    t.sync[0]=0xA1;
    t.sync[1]=0xA2;
    t.sync[2]=0xA3;
    t.sync[3]=0xA4;
    t.version = 0b11110100; // analog + version 2
    t.controlFlags = 0b01000000; //first packet
    t.last2MacBytes[0] = sessionData->macAdress[4];
    t.last2MacBytes[1] = sessionData->macAdress[5];
    t.fileLength[0] = fileSize; // >> 0
    t.fileLength[1] = fileSize >> 8;
    t.fileLength[2] = fileSize >> 16;
    t.fileLength[3] = fileSize >> 24;
    t.macAdress[0] = sessionData->macAdress[0];  // maybe use a memcpy here?
    t.macAdress[1] = sessionData->macAdress[1];
    t.macAdress[2] = sessionData->macAdress[2];
    t.macAdress[3] = sessionData->macAdress[3];
    t.macAdress[4] = sessionData->macAdress[4];
    t.macAdress[5] = sessionData->macAdress[5];
    t.apresaChannelLicences[1] = 1;
    t.crVoipChannelLicences[1] = 1;
    t.recordingChannels[1] = 1; 

    //connecting
    connectTCP();

    int packetSize = 1024;  //todo: put this in the settings.txt
        if(err == 0) {  //no problems connecting
        //sending the header
        err = send(sock, &t, sizeof(t), 0);


        //split and send the file (file already has correct headers)
        uint8_t data[packetSize];
        int size = -1;
        while(size  != 0){
            size = fread(data, sizeof(uint8_t), packetSize,file);
            err = send(sock, data, size, 0);
            if (err < 0) {
                ESP_LOGE(TAG, "Error occured during sending: errno %d", errno);
            }
        }
        ESP_LOGI(TAG, "Done Sending file! ");    
        
    }

    //close the file and the stream
    disconnectTCP();
    fclose(file);
    isSendingAFile = false;
    
    


    

    
    

    // int len = recv(sock, rx_buffer, sizeof(rx_buffer) - 1, 0);
    // // Error occured during receiving
    // if (len < 0) {
    //     ESP_LOGE(TAG, "recv failed: errno %d", errno);
    //     break;
    // }
    // // Data received
    // else {
    //     rx_buffer[len] = 0; // Null-terminate whatever we received and treat like a string
    //     ESP_LOGI(TAG, "Received %d bytes from %s:", len, addr_str);
    //     ESP_LOGI(TAG, "%s", rx_buffer);
    // }

    //     vTaskDelay(2000 / portTICK_PERIOD_MS);
    

       
     
    
}
void Apresa::startSending(){
    isSendingAFile = true;
}
bool Apresa::isSending(){
    return isSendingAFile;
}
void Apresa::connectTCP(){  //using tcp protocol
    char addr_str[128];
    int addr_family;
    int ip_protocol;

    struct sockaddr_in destAddr;
    destAddr.sin_addr.s_addr = inet_addr(HOST_IP_ADDR);    //TODO: make this into a variable
    destAddr.sin_family = AF_INET;
    destAddr.sin_port = htons(PORT);                        //TODO: make this into a variable
    addr_family = AF_INET;
    ip_protocol = IPPROTO_IP;
    inet_ntoa_r(destAddr.sin_addr, addr_str, sizeof(addr_str) - 1);

    sock =  socket(addr_family, SOCK_STREAM, ip_protocol);
    if (sock < 0) {
        ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
        return;
    }
    ESP_LOGI(TAG, "Socket created");

    err = connect(sock, (struct sockaddr *)&destAddr, sizeof(destAddr));
    if (err != 0) {
        ESP_LOGE(TAG, "Socket unable to connect: errno %d", errno);
        return;
    }
    ESP_LOGI(TAG, "Successfully connected");
}
void Apresa::disconnectTCP(){
    if (sock != -1) {
        ESP_LOGE(TAG, "Shutting down socket ");
        shutdown(sock, 0);
        close(sock);
    }
}
void Apresa::updateApresa(){ 
    for(int i = 0; i< 5; i++ ){
        ESP_LOGI(TAG,"checking and updating file: %d",i);
    }
    
    bool succes = true;
    if(succes){
        isUpdatingApresa = false;
    } //else it will call this loop again
}
void Apresa::sendLastRecording(){
    setFilePath(sessionData->last_file_name);
    startSending();
}
void Apresa::setFilePath(char * s){
    strcpy(fileName,s);
}

void Apresa::makeNewSyncFile(){

}
bool Apresa::isUpdating(){
    return isUpdatingApresa;
}
void Apresa::startUpdateApresa(){
    isUpdatingApresa = true;
}