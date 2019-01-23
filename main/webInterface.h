
void sendFileBackToClient(char * fileName, httpd_req_t * client); // function prototype. will take a file from file system (spiffs), and writes it to a client(-socket)
void sendSettingsToClient(httpd_req_t * client);
void setupWebserver();                                                   //function prototype   -> setup the webserver




/* An HTTP GET handler */
esp_err_t defaultURI_get_handler(httpd_req_t *req)
{
    char*  buf;
    size_t buf_len;

    /* Get header value string length and allocate memory for length + 1,
     * extra byte for null termination */
    buf_len = httpd_req_get_hdr_value_len(req, "Host") + 1;
    if (buf_len > 1) {
        buf = (char*)malloc(buf_len);
        /* Copy null terminated value string into buffer */
        if (httpd_req_get_hdr_value_str(req, "Host", buf, buf_len) == ESP_OK) {
            ESP_LOGI(TAG, "Found header => Host: %s", buf);
        }
        free(buf);
    }

    char* test = (char*)malloc(50);  // this will be the file that will be loaded from spiffs and send back to the client

    int length = sizeof("/spiffs/HINDEX.HTM");
    memcpy(test,"/spiffs/HINDEX.HTM",length);
    test[length] = '\0';
    sendFileBackToClient(test,req);
    free(test);
    // const char* resp_str = (const char*) req->user_ctx;
    // httpd_resp_send_chunk(req, resp_str, strlen(resp_str));
    // httpd_resp_send_chunk(req, resp_str, strlen(resp_str));
    // httpd_resp_send_chunk(req, resp_str, strlen(resp_str));
    // httpd_resp_send_chunk(req, resp_str, strlen(resp_str));
    // httpd_resp_send_chunk(req, resp_str, 0);
 
    return ESP_OK;
}

httpd_uri_t defaultURI = {
    .uri       = "/",
    .method    = HTTP_GET,
    .handler   = defaultURI_get_handler,
    /* Let's pass response string in user
     * context to demonstrate it's usage */
    .user_ctx  = (char*)"Unknown Error"
};

/* An HTTP GET handler */
esp_err_t settingsURI_get_handler(httpd_req_t *req)
{
   
    char* test = (char*)malloc(50);  // this will be the file that will be loaded from spiffs and send back to the client

    sendSettingsToClient(req);    // the variable parameters (only ethernet IP for now). the static config will be send after the variables have been send

    //sending static variables

    int length = sizeof("/spiffs/settings.txt");
    memcpy(test,"/spiffs/settings.txt",length);
    test[length] = '\0';
    sendFileBackToClient(test,req);
    free(test);
 
    return ESP_OK;
}
httpd_uri_t settingsURI = {
    .uri       = "/SETTINGS.TXT",
    .method    = HTTP_GET,
    .handler   = settingsURI_get_handler,
    /* Let's pass response string in user
     * context to demonstrate it's usage */
    .user_ctx  = (char*)"Unknown Error"
};


esp_err_t faviconURI_get_handler(httpd_req_t *req)
{  
    char* test = (char*)malloc(50);  // this will be the file that will be loaded from spiffs and send back to the client
    
    int length = sizeof("/spiffs/favicon.ico");
    memcpy(test,"/spiffs/favicon.ico",length);
    test[length] = '\0';
    sendFileBackToClient(test,req);
    free(test);
 
    return ESP_OK;
}
httpd_uri_t faviconURI = {
    .uri       = "/favicon.ico",
    .method    = HTTP_GET,
    .handler   = faviconURI_get_handler,
    /* Let's pass response string in user
     * context to demonstrate it's usage */
    .user_ctx  = (char*)"Unknown Error"
};

/* An HTTP POST handler */
esp_err_t PostURI_post_handler(httpd_req_t *req)
{
    char buf[500]; // request length max 500 chars... otherwise a for loop is needed
    int ret = req->content_len;

    
        /* Read the data for the request */
        ret = httpd_req_recv(req, buf,sizeof(buf));
        buf[ret] = '\n';
        // if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
        //     /* Retry receiving if timeout occurred */
        //     return;
        // }


        /* Log data received */
        ESP_LOGI(TAG, "=========== RECEIVED DATA ==========");
        ESP_LOGI(TAG, "%.*s", ret, buf);
        ESP_LOGI(TAG, "====================================");



         printf("settings received. write and restart!\n");
        

        
        FILE* f = fopen("/spiffs/settings.txt", "w");
        fprintf(f,buf); 
        fclose(f);
        ESP_LOGI(TAG, "DONE WRITING THE NEW SETTINGS FILE>>>> REBOOT IS NOW REQUIRED!");
        //send a hasty respond to the client.

        httpd_resp_send_chunk(req, NULL, 0); 

        vTaskDelay(1000/portTICK_PERIOD_MS);
        esp_restart(); /*magic line of code*/
        

    

    // End response
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}

httpd_uri_t PostURI = {
    .uri       = "/SETTINGS_SAVECONFIG",
    .method    = HTTP_POST,
    .handler   = PostURI_post_handler,
    .user_ctx  = NULL
};



void setupWebserver(){
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    // Start the httpd server
    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK) {
        // Set URI handlers
        ESP_LOGI(TAG, "Registering URI handlers");
        httpd_register_uri_handler(server, &defaultURI);
        httpd_register_uri_handler(server, &settingsURI);
        httpd_register_uri_handler(server, &faviconURI);
        httpd_register_uri_handler(server, &PostURI);
    }

}       



void sendFileBackToClient(char * fileName, httpd_req_t * client){
            int packetSize = 512;   //for testing ... the desired size of a single packet
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
                httpd_resp_send_chunk(client , "error. the file required by the browser is not found on the device" , 66);
                httpd_resp_send_chunk(client , NULL , 0);
                //write(cs , MESSAGE , strlen(MESSAGE));            
                ESP_LOGI(TAG, "... socket send success");
                //return;
            } else {
                fseek(f, 0, SEEK_END);
                fileSize = ftell(f);
                fseek(f,0,SEEK_SET);
                ESP_LOGI(TAG, "preparing to send data back to the client... size: %d", fileSize);
            
                while(currentIndex < fileSize){
                    int numElementsRead = fread(line,1,packetSize, f);
                    currentIndex+=numElementsRead;
                   //write(cs , line , numElementsRead);
                    httpd_resp_send_chunk(client, line, numElementsRead);                    
                }
                
                 httpd_resp_send_chunk(client, line, 0);           
                fclose(f);
                
                // strip newline
                // char* pos = strchr(line, '\n');
                // if (pos) {
                //     *pos = '\0';
                // }
               // ESP_LOGI(TAG, "Read from file: '%s'", line);
                
        }
}
void sendSettingsToClient(httpd_req_t * client){    
    //*generate the settings file, and send it to the client*//
    //*client will edit the values, and settings will come back and are stored in spiffs and are applied (using a restart)*//
    //char str[10];       //for converting int to char*
    char toSend[300] = "";  //might be changed to a bigger or smaller number
    
    strcat(toSend,"ethernet_ip:");      strcat(toSend,sb.session_data->Ethernet_IP_Adress);                                 strcat(toSend,"\r");strcat(toSend,"\n");
    // strcat(toSend,"sample_rate:");      sprintf(str, "%d", sb.audio_config->sample_rate);          strcat(toSend,str);      strcat(toSend,"\n");
    // strcat(toSend,"bit_deptt:");        sprintf(str, "%d", sb.audio_config->bits_per_sample);      strcat(toSend,str);      strcat(toSend,"\n");
    // strcat(toSend,"num_channels:");     sprintf(str, "%d", sb.audio_config->num_channels);         strcat(toSend,str);      strcat(toSend,"\n");
    // strcat(toSend,"a1:b2\n");
    // strcat(toSend,"a3:b2\n");
    // strcat(toSend,"a44:b4323\n");
    // strcat(toSend,"alol:bhihi\n");
    //write(cs , toSend , strlen(toSend));
     httpd_resp_send_chunk(client, toSend, strlen(toSend));       
             
    //NOTE: the other settings are stored in spiffs, and will be send after this function returns!!!!!


   
}
