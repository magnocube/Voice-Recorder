#include "Apresa.h"
#define TAG "apresaTunnelTest"

Apresa::Apresa(){  //normal constructor
    fileName = (char*)malloc(9 +20);  //the 20 is for the path that needs to be added, the 9 is for the file path length
}

void Apresa::setup(){  //for setting up any other things

}
void Apresa::setFileName(char* name){  //will set the name used by the "sendFile()" function
    
}
void Apresa::sendFile(){
    	
	//char rx_buffer[128];
    

    //assuming nothing will be received
    
    connectTCP();
    if(err == 0) {  //no problems
        err = send(sock, payload, strlen(payload), 0);
        ESP_LOGI(TAG, "succesfuls packet send");
        if (err < 0) {
            ESP_LOGE(TAG, "Error occured during sending: errno %d", errno);
        }
    }
    disconnectTCP();
    


    

    
    

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