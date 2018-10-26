void webInterface(esp_shared_buffer *shared_buffer){ 
while(1){
			ESP_LOGI(TAG,"tcp_server task started \n");
    struct sockaddr_in tcpServerAddr;
    tcpServerAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    tcpServerAddr.sin_family = AF_INET;
    tcpServerAddr.sin_port = htons( 80 );
    int s, r;
    char recv_buf[512];  /// first 512 characters of the request from the browser. (most of the time the entire request)
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
        while(1){
            cs=accept(s,(struct sockaddr *)&remote_addr, &socklen);
            ESP_LOGI(TAG,"New connection request,Request data:");
            //set O_NONBLOCK so that recv will return, otherwise we need to impliment message end 
            //detection logic. If know the client message format you should instead impliment logic
            //detect the end of message
            fcntl(cs,F_SETFL,O_NONBLOCK);
            do {
                bzero(recv_buf, sizeof(recv_buf));
                r = recv(cs, recv_buf, sizeof(recv_buf)-1,0);
                for(int i = 0; i < r; i++) {
                    putchar(recv_buf[i]);
                }
            } while(r > 0);
            
            ESP_LOGI(TAG, "... done reading from socket. Now implement logic and send reply Last read return=%d errno=%d\r\n", r, errno);
            int packetSize = 256;   //for testing ... the desired size of a single packet
            int fileSize;           //size of the Faile
            int currentIndex = 0;   //current index (from 0 to filesize)
            char line[packetSize];  //a single packet to send
            
            for(int i =0; i< packetSize; i++){
                line[i] = ' ';
            }
                    

            ESP_LOGI(TAG, "Reading file");
            FILE* f = fopen("/spiffs/HINDEX.HTM", "r");
            if (f == NULL) {
                ESP_LOGE(TAG, "Failed to open file for reading");
                write(cs , "error!" , strlen("error!"));
                write(cs , MESSAGE , strlen(MESSAGE));            
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
                


            
            write(cs , MESSAGE , strlen(MESSAGE));
            
            ESP_LOGI(TAG, "... socket send success");
            close(cs);
        }
        ESP_LOGI(TAG, "connection to client closed. waiting for new connection");
        	// vTaskDelay(1000/portTICK_PERIOD_MS);
	// shared_buffer->recording = true;
	// vTaskDelay(2000/portTICK_PERIOD_MS);
	// shared_buffer->recording = false;
        //vTaskDelay(500 / portTICK_PERIOD_MS);
    }
    ESP_LOGI(TAG, "...tcp_client task closed\n");

}
}
