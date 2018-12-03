void testDNS(char url[]); //function prototype
void testGPIOExpander(int i); //function prototype
bool bDNSFound = false;   //local needed variable (for testing)
ip_addr_t ip_Addr;
void dns_found_cb(const char *name, const ip_addr_t *ipaddr, void *callback_arg);
void Test_task(esp_shared_buffer *shared_buffer){   
    static const char* TEST_TAG = "*******************SELF-TEST: "; 
    vTaskDelay(1500/portTICK_PERIOD_MS);
    
	while(1){
        for(int i =0; i< 100; i++){      
            printf(".\n");            //clear screen
        }


        ESP_LOGI(TEST_TAG, "**********************************************");
        ESP_LOGI(TEST_TAG, "*DEVICE WILL START IN TEST MODE.");
        ESP_LOGI(TEST_TAG, "*Please follow instructions. if the device happens to crash, the cause can most likely be determined by the last action");
        ESP_LOGI(TEST_TAG, "*The device will still work as normal, only some statistics will be logged here. Please only follow the instructions");
        ESP_LOGI(TEST_TAG, "**********************************************");

    

        ESP_LOGI(TEST_TAG, "Testing ethernet connection. Please make sure a ethernet cable is inserted");
        while(!shared_buffer->session_data->Ethernet_Ip_received){vTaskDelay(500/portTICK_PERIOD_MS);}
        ESP_LOGI(TEST_TAG, "Received a IP from DHCP!");
        ESP_LOGI(TEST_TAG, "Testing ethernet connectivity. using dns to resolve vidicode website IP");
        char url[] = "www.vidicode.com";
        testDNS(url);

        ESP_LOGI(TEST_TAG, "testing GPIO expander. Please note the blinkling led's on de voicerecorder");
        testGPIOExpander(3);


        ESP_LOGI(TEST_TAG, "testing the SD Card. Please make sure the SD card is in the slot");
        while(!shared_buffer->SD->isMounted()){ testGPIOExpander(1);}
        ESP_LOGI(TEST_TAG, "SD card is detected and mounted, please remove the SD card");
        while(shared_buffer->SD->isMounted()){testGPIOExpander(1);}
        ESP_LOGI(TEST_TAG, "SD card is removed, now place it back in");
        while(!shared_buffer->SD->isMounted()){testGPIOExpander(1);}





        ESP_LOGI(TEST_TAG, "**********************************************");
        ESP_LOGI(TEST_TAG, "*TEST CODE COMPLETED.");
        ESP_LOGI(TEST_TAG, "if you see this message it means that all the software-related things have been tested sucesfully");
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
            vTaskDelay(1000/portTICK_PERIOD_MS); //make clear device is in test mode
        }
        printf(".\n");
        
            
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