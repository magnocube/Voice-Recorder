void testDNS(char url[]); //function prototype
void testGPIOExpander(int i); //function prototype
void testReadWrite();   //function prototype
bool bDNSFound = false;   //local needed variable (for testing)
ip_addr_t ip_Addr;
void dns_found_cb(const char *name, const ip_addr_t *ipaddr, void *callback_arg);

//clear the screen and display a message
void startTest(char* name){
    for(int i =0; i< 100; i++){printf("\n");}// clear screen
    printf("############################################## Test: ");
    printf(name);
    printf(" ##############################################\n");
}
//ask for a button press and display some dots.
void stopTest(esp_shared_buffer *shared_buffer, char* message){
    printf("############################################## Test: ");
    printf(message);
    printf(" ##############################################\n");
    
    printf("############################################## Test Completed. Please press the button to continue ##############################################n");
    while(gpio_get_level((gpio_num_t)pinout.big_button)){ // high signal will generate 'True' (not pressed because of pullup)
        printf(".");
        fflush(stdout);
        vTaskDelay(300/portTICK_PERIOD_MS);
    }
    printf("\n");

}



void Test_task(esp_shared_buffer *shared_buffer){   
    static const char* TEST_TAG = "Test: "; 
    vTaskDelay(1500/portTICK_PERIOD_MS);

    char* finishMessage = (char*)malloc(100);
    char* startMessage = (char*)malloc(100);
    
	while(1){
        
        for(int i =0; i< 100; i++){printf(".\n");}// clear screen



        ESP_LOGI(TEST_TAG, "**********************************************");
        ESP_LOGI(TEST_TAG, "*DEVICE WILL START IN TEST MODE.");
        ESP_LOGI(TEST_TAG, "*Version: 1.0.0");
        ESP_LOGI(TEST_TAG, "*Please follow instructions. if the device happens to crash, the cause can most likely be determined by the last action (stack trace will point it out)");
        ESP_LOGI(TEST_TAG, "*The device will still work as normal, You will be prompted to to various actions when needed. Please only follow the instructions");
        ESP_LOGI(TEST_TAG, "*this test is runned automaticly, you will be asked press the big button on top of the case to continue");
        ESP_LOGI(TEST_TAG, "*NOTE: it is possible that some other peripherals will log their data during testing... this data will make no sense regarding the test and can be ignored");
        ESP_LOGI(TEST_TAG, "**********************************************");
        ESP_LOGI(TEST_TAG, "the following is a description of the version of the IDF on which this code has been compiled");
        const char* v = esp_get_idf_version();
        ESP_LOGI(TEST_TAG, "%s",v);
        strcpy(finishMessage,"End of Instructions page");
        stopTest(shared_buffer,finishMessage);

        strcpy(startMessage,"Ethernet cable Check");
        startTest(startMessage);
        ESP_LOGI(TEST_TAG, "Testing ethernet connection. Please make sure a ethernet cable is inserted");
        while(!shared_buffer->session_data->Ethernet_Ip_received){vTaskDelay(500/portTICK_PERIOD_MS);}   // the boolean Ethernet_Ip_received is only true when there is a succesfull connection with the network
        ESP_LOGI(TEST_TAG, "Received a IP from DHCP!");
        ESP_LOGI(TEST_TAG, "Testing ethernet connectivity. using dns to resolve vidicode website IP");
        char url[] = "www.vidicode.com";
        testDNS(url);
        strcpy(finishMessage,"The Ip on the previous line should NOT be 0.0.0.0");
        stopTest(shared_buffer,finishMessage);

        strcpy(startMessage,"Led's Check");
        startTest(startMessage);
        ESP_LOGI(TEST_TAG, "testing GPIO expander. Please note the blinkling led's on de voicerecorder");
        testGPIOExpander(10); // 5 times before proceeding.. this can not be checked with software, and needs to be inspected manually
        int redLed = sb.pin_config->led_red;
        int yellowLed = sb.pin_config->led_yellow;
        int greenLed = sb.pin_config->led_green;
        int blueLed = sb.pin_config->led_blue;
        pca9535 * gh = sb.gpio_header;
        gh->digitalWrite(redLed,PCA_LOW,false);
		gh->digitalWrite(yellowLed,PCA_HIGH,false);
		gh->digitalWrite(greenLed,PCA_HIGH,false);
		gh->digitalWrite(blueLed,PCA_HIGH,true);
        strcpy(finishMessage,"Continue when leds are all on in the right order\n->red\n->green\n->blue\n->yellow\n");
        stopTest(shared_buffer,finishMessage);


        strcpy(startMessage,"SD card check");
        startTest(startMessage);
        ESP_LOGI(TEST_TAG, "testing the SD Card. Please make sure the SD card is in the slot");
        while(!shared_buffer->SD->isCardMounted){ vTaskDelay(100/portTICK_PERIOD_MS);}
        ESP_LOGI(TEST_TAG, "SD card is detected and mounted, please remove the SD card");
        while(shared_buffer->SD->isCardMounted){ vTaskDelay(100/portTICK_PERIOD_MS);}
        ESP_LOGI(TEST_TAG, "SD card is removed, now place it back in");
        while(!shared_buffer->SD->isCardMounted){ vTaskDelay(500/portTICK_PERIOD_MS);}       



        testReadWrite();
        strcpy(finishMessage,"Finished testing the SD card. please confirm there is no Red led");
        stopTest(shared_buffer,finishMessage);


        //ESP_LOGI(TEST_TAG, "Please remove the Ethernet cable");
       // while(shared_buffer->session_data->Ethernet_Ip_received){vTaskDelay(500/portTICK_PERIOD_MS);}   // the boolean Ethernet_Ip_received is only true when there is a succesfull connection with the network

        strcpy(startMessage,"recording check,");
        startTest(startMessage);

        ESP_LOGI(TEST_TAG, "Press the button on top of the case to start the recording. It will take 5 seconds. be sure to say something recognisable");
        vTaskDelay(1000/portTICK_PERIOD_MS);
         while(gpio_get_level((gpio_num_t)pinout.big_button)){ vTaskDelay(100/portTICK_PERIOD_MS);}
        
        ESP_LOGI(TEST_TAG, "Testing Audio codec, NOTE: the settings currently stored on the device will be used");
        ESP_LOGI(TEST_TAG, "Testing for 5 seconds");
        shared_buffer->recording = true;
        vTaskDelay(5000/ portTICK_PERIOD_MS);
        shared_buffer->recording = false;
        ESP_LOGI(TEST_TAG, "Testing Audio done, please notice that the green LED is off. audio must be checked manually (for now)");
        vTaskDelay(100/portTICK_PERIOD_MS); //time to write out the header
        strcpy(finishMessage,"Finished testing a recording. please make sure to check the quallity of the audio manually (for now)");
        stopTest(shared_buffer,finishMessage);

        //todo.. play the recorded sond back using the audio jack to verify the analog cirquit
        //todo.. record with different microphones
        //todo.. send recording to apresa/computer software.
        

        for(int i =0; i< 100; i++){printf(".\n");}// clear screen
        vTaskDelay(300/ portTICK_PERIOD_MS);

        ESP_LOGI(TEST_TAG, "**********************************************");
        ESP_LOGI(TEST_TAG, "*TEST CODE COMPLETED.");
        ESP_LOGI(TEST_TAG, "if you see this message it means that all the hardware-related things have been tested sucesfully");
        ESP_LOGI(TEST_TAG, "please manually check the following items");
        ESP_LOGI(TEST_TAG, " -(MUST)remove USB cable and check if the led's (especially the blue one) light up when using PoE");
        ESP_LOGI(TEST_TAG, " -(MUST)remove USB cable and check if the led's light up when using a 9-12v DC adapter");
        ESP_LOGI(TEST_TAG, " -(COULD)connect to the wifi acces point and type the IP '192.168.1.1' in your browser to connect to the web interface");
        ESP_LOGI(TEST_TAG, " -(COULD)use the ethernet IP (which can be found on the web interface or earlier during this test) to connect to the web interface");
        ESP_LOGI(TEST_TAG, "**********************************************");
        ESP_LOGI(TEST_TAG, "Testcode will run again in 50 seconds\n");
        for(int i =0; i< 50; i++){      
            printf(".");      
            fflush(stdout); 
            vTaskDelay(1000/portTICK_PERIOD_MS); //make clear device is still running
        }
        printf(".\n");
        
            
	}
			 


}
void testReadWrite(){
        static const char* TEST_TAG = "Test: ";
        ESP_LOGI(TEST_TAG, "testing a write action to the SD card. Test message will be written to a file on the SD card");
        char fileName[] = "/sdcard/test.txt";
        char textToWrite[] = "This text is placed during an auto-test of the device. This file can be removed";

        FILE* f = fopen(fileName, "w");
        fprintf(f,textToWrite);   
        fclose(f);

        ESP_LOGI(TEST_TAG, "Done writing, now reading the file");

        f = fopen(fileName, "r");
        if (f == NULL) {
            ESP_LOGE(TAG, "The file can not be opened. this however is most likely a software issue (dont't expect this ever to show up)");
        
        } else {
            char line[100];
            fgets(line, sizeof(line), f);
            fclose(f);
            // strip newline
            char* pos = strchr(line, '\n');
            if (pos) {
                *pos = '\0';
            }
            ESP_LOGI(TAG, "Read from file: '%s'", line);
            if (strcmp(textToWrite, line) == 0)
            {
            ESP_LOGI(TEST_TAG, "Reading file succesfull!");
            } else {
            ESP_LOGE(TEST_TAG, "written and read line do not equal... might be software");
            }
        }
        
}
void testGPIOExpander(int i){
    int redLed = sb.pin_config->led_red;
	int yellowLed = sb.pin_config->led_yellow;
	int greenLed = sb.pin_config->led_green;
	int blueLed = sb.pin_config->led_blue;
	pca9535 * gh = sb.gpio_header;

	// shared_buffer->recording = true;
	// vTaskDelay(5000/portTICK_PERIOD_MS);
	// shared_buffer->recording = false;

	while(i > 0){	
			i--;
		
			gh->digitalWrite(redLed,PCA_LOW,false);
			gh->digitalWrite(yellowLed,PCA_LOW,false);
			gh->digitalWrite(greenLed,PCA_LOW,false);
			gh->digitalWrite(blueLed,PCA_LOW,true);
			 vTaskDelay(100/portTICK_PERIOD_MS); //reset watchdog
			 
	 
			gh->digitalWrite(redLed,PCA_HIGH,false);
			gh->digitalWrite(yellowLed,PCA_LOW,false);
			gh->digitalWrite(greenLed,PCA_HIGH,false);
			gh->digitalWrite(blueLed,PCA_LOW,true);
			 vTaskDelay(100/portTICK_PERIOD_MS); //reset watchdog

			  
			gh->digitalWrite(redLed,PCA_HIGH,false);
			gh->digitalWrite(yellowLed,PCA_LOW,false);
			gh->digitalWrite(greenLed,PCA_LOW,false);
			gh->digitalWrite(blueLed,PCA_HIGH,true);
			 vTaskDelay(100/portTICK_PERIOD_MS); //reset watchdog

			  
			gh->digitalWrite(redLed,PCA_HIGH,false);
			gh->digitalWrite(yellowLed,PCA_HIGH,false);
			gh->digitalWrite(greenLed,PCA_LOW,false);
			gh->digitalWrite(blueLed,PCA_LOW,true);
			 vTaskDelay(100/portTICK_PERIOD_MS); //reset watchdog

			  
			gh->digitalWrite(redLed,PCA_HIGH,false);
			gh->digitalWrite(yellowLed,PCA_LOW,false);
			gh->digitalWrite(greenLed,PCA_LOW,false);
			gh->digitalWrite(blueLed,PCA_HIGH,true);
			 vTaskDelay(100/portTICK_PERIOD_MS); //reset watchdog

			  
			gh->digitalWrite(redLed,PCA_HIGH,false);
			gh->digitalWrite(yellowLed,PCA_LOW,false);
			gh->digitalWrite(greenLed,PCA_HIGH,false);
			gh->digitalWrite(blueLed,PCA_LOW,true);
			 vTaskDelay(100/portTICK_PERIOD_MS); //reset watchdog
	}
            gh->digitalWrite(redLed,PCA_HIGH,false);
			gh->digitalWrite(yellowLed,PCA_HIGH,false);
			gh->digitalWrite(greenLed,PCA_LOW,false);
			gh->digitalWrite(blueLed,PCA_HIGH,true);
}
void testDNS(char url[]){
    ip_addr_t ip_Addr;
    IP_ADDR4( &ip_Addr, 0,0,0,0 );
     dns_gethostbyname(url, &ip_Addr, dns_found_cb, NULL );
      vTaskDelay(1000/ portTICK_PERIOD_MS);
    dns_gethostbyname(url, &ip_Addr, dns_found_cb, NULL );

   
    while( !bDNSFound ){vTaskDelay(1000/ portTICK_PERIOD_MS);}
        
    printf( "DNS found: %i.%i.%i.%i\n", 
        ip4_addr1(&ip_Addr.u_addr.ip4), 
        ip4_addr2(&ip_Addr.u_addr.ip4), 
        ip4_addr3(&ip_Addr.u_addr.ip4), 
        ip4_addr4(&ip_Addr.u_addr.ip4) );

        
}
void dns_found_cb(const char *name, const ip_addr_t *ipaddr, void *callback_arg)
{
    ip_Addr = *ipaddr;
    bDNSFound = true;
}
