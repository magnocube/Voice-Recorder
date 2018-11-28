void sendFileBackToClient(char * fileName, int cs); // function prototype. will take a file from file system (spiffs), and writes it to a client(-socket)
void sendSettingsToClient(int cs);
void webInterface(esp_shared_buffer *shared_buffer){ 
while(1){
			ESP_LOGI(TAG,"tcp_server task started \n");
    struct sockaddr_in tcpServerAddr;
    tcpServerAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    tcpServerAddr.sin_family = AF_INET;
    tcpServerAddr.sin_port = htons( 80 );
    int s, r;
    char recv_buf[5120];  /// first 5120 characters of the request from the browser. i hope this is enough for the enitre request... otherwise some strange errors might occour)
    static struct sockaddr_in remote_addr;
    static unsigned int socklen;
    socklen = sizeof(remote_addr);
    int cs;//client socket
    //xEventGroupWaitBits(wifi_event_group,CONNECTED_BIT,false,true,portMAX_DELAY);
   
        s = socket(AF_INET, SOCK_STREAM, 0);
        if(s < 0) {
            ESP_LOGE(TAG, "... Failed to allocate socket.\n");
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            continue;
        }
        ESP_LOGI(TAG, "... allocated socket\n");
         if(bind(s, (struct sockaddr *)&tcpServerAddr, sizeof(tcpServerAddr)) != 0) {
            ESP_LOGE(TAG, "... socket bind failed errno=%d \n", errno);
            close(s);
            vTaskDelay(4000 / portTICK_PERIOD_MS);
            continue;
        }
        ESP_LOGI(TAG, "... socket bind done \n");
        if(listen (s, LISTENQ) != 0) {
            ESP_LOGE(TAG, "... socket listen failed errno=%d \n", errno);
            close(s);
            vTaskDelay(4000 / portTICK_PERIOD_MS);
            continue;
        }
        while(1){                                                       //this loop should never exit
            cs=accept(s,(struct sockaddr *)&remote_addr, &socklen);
            ESP_LOGI(TAG,"New connection request,Request data:");
            //set O_NONBLOCK so that recv will return, otherwise we need to impliment message end 
            //detection logic. If know the client message format you should instead impliment logic
            //detect the end of message 
            fcntl(cs,F_SETFL,O_NONBLOCK);  //TODO: figure this statement out
            do {  //read the start of the packet... only the GET is important
                bzero(recv_buf, sizeof(recv_buf));
                r = recv(cs, recv_buf, sizeof(recv_buf)-1,0);
                for(int i = 0; i < r; i++) {
                    putchar(recv_buf[i]);
                }
                recv_buf[r] = '\0';
            
                    int indexOfNewLine = 0;
                   
                    for (int i = 0; i < r; i++){
                        if (recv_buf[i] =='\n'){
                            printf("found the newLine");
                            printf("%d ",i);
                            printf("\n");
                            indexOfNewLine = i;
                            break;
                        }
                    }

                    char command[indexOfNewLine+1];
                    memcpy(command,recv_buf,indexOfNewLine);  // first line of the http request
                    command[indexOfNewLine] = '\0';
                    printf(command);
                    

                    char* test = (char*)malloc(50);

                    if(strstr(command,"GET / HTTP/")){   //just a empty default request
                        printf("the client request a new webpage... this should reply");
                        int length = sizeof("/spiffs/HINDEX.HTM");
                        memcpy(test,"/spiffs/HINDEX.HTM",length);
                        test[length] = '\0';
                        sendFileBackToClient(test,cs);
                    } else if(strstr(command,"GET /SETTINGS.TXT HTTP/")){ //ask for the index
                        printf("settings requested. i do a self-test!\n");
                        // int length = sizeof("/spiffs/HINDEX.HTM");
                        // memcpy(test,"/spiffs/HINDEX.HTM",length);
                        // test[length] = '\0';
                        // sendFileBackToClient(test,cs);
                                            
                        // int length = sizeof("/spiffs/settings.txt");
                        // memcpy(test,"/spiffs/settings.txt",length);
                        // test[length] = '\0';
                        // sendFileBackToClient(test,cs);
                        sendSettingsToClient(cs);    // the variable parameters (only ethernet IP for now)

                        int length = sizeof("/spiffs/settings.txt");
                        memcpy(test,"/spiffs/settings.txt",length);
                        test[length] = '\0';
                        sendFileBackToClient(test,cs);

                    } else if(strstr(command,"POST /SETTINGS_SAVECONFIG HTTP/")){ //ask for the index
                        printf("settings received.writte and restart!\n");
                        char* index =strstr(recv_buf,"CONFIG:");

                        printf("size of index to end: %d", strlen(index));
                        if(index){
                            FILE* f = fopen("/spiffs/settings.txt", "w");
                            fprintf(f,index+sizeof("CONFIG\n")); 
                            fclose(f);
                            ESP_LOGI(TAG, "DONE WRITING THE NEW SETTINGS FILE>>>> REBOOT IS NOW REQUIRED!");
                        }
                        

                    }else {
                        printf("i cant do anything with this request... nothing will be send back");
                    }
                    free(test);
                
             
            } while(r > 0);   //if the packet has not been read entirely, just read the remaining part again,,, this time there will be no GET and evrything gets ignored (just to clear the buffer)
            
            ESP_LOGI(TAG, "... done reading from socket. Last read return=%d errno=%d\r\n", r, errno);
            ESP_LOGI(TAG, "... socket send success");
            close(cs);
        }
        ESP_LOGI(TAG, "connection to client closed. waiting for new connection");
        	// vTaskDelay(1000/portTICK_PERIOD_MS);
	// shared_buffer->recording = true;
	// vTaskDelay(2000/portTICK_PERIOD_MS);
	// shared_buffer->recording = false;
       // vTaskDelay(500 / portTICK_PERIOD_MS); //prevent flooding
    }
    ESP_LOGI(TAG, "...tcp_client task closed\n");



}


void sendFileBackToClient(char * fileName, int cs){
            int packetSize = 256;   //for testing ... the desired size of a single packet
            int fileSize;           //size of the Faile
            int currentIndex = 0;   //current index (from 0 to filesize)
            char line[packetSize];  //a single packet to send
            
            for(int i =0; i< packetSize; i++){ //clear mem
                line[i] = ' ';
            }
                    

            ESP_LOGI(TAG, "Reading file");
            FILE* f = fopen(fileName, "r");
            // FILE* f = fopen("/spiffs/HINDEX.HTM", "r");
            if (f == NULL) {
                ESP_LOGE(TAG, "Failed to open file for reading");
                write(cs , "error. the file required by the browser is not found on the device" , 66);
                //write(cs , MESSAGE , strlen(MESSAGE));            
                ESP_LOGI(TAG, "... socket send success");
                close(cs);
                //return;
            } else {
                fseek(f, 0, SEEK_END);
                fileSize = ftell(f);
                fseek(f,0,SEEK_SET);
                ESP_LOGI(TAG, "preparing to send data back to the client... size: %d", fileSize);
            
                while(currentIndex < fileSize){
                    int numElementsRead = fread(line,1,packetSize, f);
                    currentIndex+=numElementsRead;
                    write(cs , line , numElementsRead);
                    
                }
                
                
                fclose(f);
                // strip newline
                // char* pos = strchr(line, '\n');
                // if (pos) {
                //     *pos = '\0';
                // }
               // ESP_LOGI(TAG, "Read from file: '%s'", line);
                
        }
}
void sendSettingsToClient(int cs){    
    //*generate the settings file, and send it to the client*//
    //*client will edit the values, and settings will come back and are stored in spiffs and are applied*//
    //char str[10];       //for converting int to char*
    char toSend[1000] = "parameter:value\n";  //might be changed to a bigger or smaller number
    
    strcat(toSend,"ethernet_ip:");      strcat(toSend,sb.session_data->Ethernet_IP_Adress);                                 strcat(toSend,"\n");
    // strcat(toSend,"sample_rate:");      sprintf(str, "%d", sb.audio_config->sample_rate);          strcat(toSend,str);      strcat(toSend,"\n");
    // strcat(toSend,"bit_deptt:");        sprintf(str, "%d", sb.audio_config->bits_per_sample);      strcat(toSend,str);      strcat(toSend,"\n");
    // strcat(toSend,"num_channels:");     sprintf(str, "%d", sb.audio_config->num_channels);         strcat(toSend,str);      strcat(toSend,"\n");
    // strcat(toSend,"a1:b2\n");
    // strcat(toSend,"a3:b2\n");
    // strcat(toSend,"a44:b4323\n");
    // strcat(toSend,"alol:bhihi\n");
    write(cs , toSend , strlen(toSend));


   
}
