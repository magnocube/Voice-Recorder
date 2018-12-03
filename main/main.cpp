/*
when reading trhough the code, start at settings.h.
then read main.h and main.cpp
pressing F12 while selecting a function will jump to the declaration of that funtion (VScode)
*/

#include "main.h"

extern "C" { 														 //this just needs to be here
	void app_main();
}

void IRAM_ATTR button_isr_handler(void* arg) { //the button on the device will create an interrupt that will be handled here
	
    uint32_t gpio_num = (uint32_t) arg;
    xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
}
static void gpio_task_example(void* arg)
{
    uint32_t lastTimeInterrupt = 0;
    uint32_t io_num;
    for(;;) {
        if(xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY)) {
            uint32_t timeNow = esp_log_timestamp();
            if(timeNow - lastTimeInterrupt>= 200){  // to prevent 2 fast interrupts... at least 200 milliseconds between a interrupt -> this value might be changed to a higher number
            printf("time since last button: %d",timeNow - lastTimeInterrupt);
                lastTimeInterrupt = timeNow;                
                ESP_LOGI(TAG, "Whoop Whoop, Button has been pressed!, Whoop Whoop.");

                if(sb.recording == false){
                    if(sb.SD->isMounted()){						//and the card is acually mounted
                        sb.recording = true;
                        ESP_LOGI(TAG, "recording set to true");
                    } else{
                        ESP_LOGI(TAG, "not recording... card is not in slot");
                    }
                                            //the recording led will be turned on by the recording task (this funtion does not know if the card is mounted)
                }else{
                    sb.recording = false;
                    sb.gpio_header->digitalWrite(pinout.led_green,PCA_LOW,true); //turn of the recording led
                    /*NOTE: THE RECORDING MAY CONTINUE IF THE SD CARD HAS PROBLEMS, THIS ONLY SETS THE FLAG 'RECORDING' false*/
                }
            } else {

            }
        }
    }
}

void app_main()
{
   
   
	pinout = ESP_PIN_CONFIG_DEFAULT(); 			                	//change default pin-config in "settings.h"	
	audioConfig = ESP_AUDIO_CONFIG_DEFAULT();		                //change default audio-config in "settings.h"
    sessionData = ESP_SESSION_DATA_DEFAULT();                       //change default parameters in "settings.h"
    setupInterruptBigButton(&pinout);                               // must be called before reading the large button to check if the device must be tested. otherwise pin is not configured.
        
    ESP_LOGI(TAG,"READING BUTTON (FOR TESTING PURPOSES :D)");
    sessionData.is_in_TestModus = !gpio_get_level((gpio_num_t)pinout.big_button);                       //read the large button and invert the input
    ESP_LOGI(TAG, "divice modus. 1=testmodus, 0=normal operatopn: %d", sessionData.is_in_TestModus);
    if(sessionData.is_in_TestModus){
        printf("**********************************************\n*\n*\n");
         printf("DEVICE WILL START IN TEST MODE\n*\n*\n");
          printf("**********************************************\n");
        vTaskDelay(5000/portTICK_PERIOD_MS); //make clear device is in test mode
    } else{
        printf("**********************************************\n*\n*\n");
         printf("DEVICE WILL START IN NORMAL OPERATING MODE\n*\n*\n");
          printf("**********************************************\n");
        //idk, some startip sound maybe :D?
    }


    setupPeripherals(&pinout);					                	//setup for i2c, etc...
    setupDeviceSettingsFromSPIFFS();                                //will fill the corresponding config-structs with the desired settings (settings can be cahnged using the browser and a restart, or to reflash the storage-partition)   
   

	pca9535 *pca_ptr = new pca9535(&pinout, &sessionData);                                   //make instance of the i2c-GPIO-expander
	SDCard *SD_ptr = new SDCard(&pinout, &audioConfig, pca_ptr);                            //make instance of the SD card interface
	WM8960 *audio_codec_ptr = new WM8960(&audioConfig, SD_ptr, pca_ptr, &pinout);           //make instance of the audio codec interface
	sb = {	.recording = false,						                                        //this shared_buffer is passed to the different tasks it contains all the pointers Both tasks need.
			.gpio_header = pca_ptr,					                                        /*TODO: rename 'shared_buffer' to 'shared_memory'*/
			.SD = SD_ptr,
			.codec = audio_codec_ptr,
			.audio_config = &audioConfig,
			.pin_config = &pinout,
            .session_data = &sessionData	
			};

	testSPIFFSRead();	
    configureGPIOExpander();  // sets all the required pinmodes (can be changed dynamicly anywhere in the code)
    start_mdns_service();

    
    pca_ptr->digitalWrite(pinout.sdPower,PCA_HIGH,true);	
    // pca_ptr->digitalWrite(pinout.phy_reset,PCA_LOW,true); 
    // vTaskDelay(20/portTICK_PERIOD_MS);
    // pca_ptr->digitalWrite(pinout.phy_reset,PCA_HIGH,true);  				
    
    audio_codec_ptr->printCopyCodecRegisters();

	

	/*create a recording task. This task will handle the SD card and the I2S*/
    xTaskCreatePinnedToCore((TaskFunction_t)recording_task,						//task function		   
							 "recording_task", 									//task name 
							 1024 * 4, 											//stack size
							 &sb,												//function parameters (struct with pointers to shared classes)
							 99,												//priority
							 NULL,												//task handle
							 0													//task core
							 );	
							 
                             
	/*create a "other task", this task will do everything else (wifi,ethernet)*/
	xTaskCreatePinnedToCore((TaskFunction_t)Wifi_ethernet_interface_task,		//task function		   //probably the tast that does everything except recording
							 "Wifi_ethernet_interface_task", 					//task name 
							 1024 * 8, 											//stack size
							 &sb,												//function parameters (struct with pointers to shared classes)
							 2,													//priority
							 NULL,												//task handle
							 1													//task core
							 );

    
    xTaskCreatePinnedToCore((TaskFunction_t)webInterface,	                	//task function		   //handles the webpage
							 "webInterface_task", 					            //task name 
							 1024 * 8, 											//stack size
							 &sb,												//function parameters (struct with pointers to shared classes)
							 1,													//priority
							 NULL,												//task handle
							 1													//task core
							 );

    xTaskCreatePinnedToCore((TaskFunction_t)gpio_task_example,		            //task function		   //hanles the big button on the case(when an interupt is send)
							 "button_task", 					                //task name 
							 1024 * 2, 											//stack size
							 NULL,												//function parameters (struct with pointers to shared classes)
							 1,													//priority
							 NULL,												//task handle
							 1													//task core
							 );

    if(sessionData.is_in_TestModus){
        xTaskCreatePinnedToCore((TaskFunction_t)Test_task,		                    //task function		   //using software to determine if all the hardware is working
                                "test_task", 					                    //task name 
                                1024 * 2, 											//stack size
                                &sb,												//function parameters (struct with pointers to shared classes)
                                1,													//priority
                                NULL,												//task handle
                                1													//task core
                                );
    }


	//while(1){ 											// -- this loop has cost me a couple of hours of hopeless debugging
	//	vTaskDelay(1000/portTICK_PERIOD_MS);   				//    when the main ends the structs pinconfig and audioconfig will be removed from the stack
	//}														//    and overwritten by other memory... took me some time to figure that out... this loop prevents
															// 	  the main from ending and keeping the 2 structs in memory. (i know... i m too lazy to place the
															//	  structs on the heap and pass a pointer to the required functions): update: i removed the
															// the structs from the stack. and placed them in the heap... too lazy to remove the comment.
}

void setupDeviceSettingsFromSPIFFS(){
    FILE* f = fopen("/spiffs/settings.txt", "r");
        fseek(f, 0, SEEK_END);
        int fileSize = ftell(f);
        fseek(f,0,SEEK_SET);
        ESP_LOGI(TAG, "size of settigns file: ... size: %d\n", fileSize);

        
        char buf[100];  // line max 100 chars. if a line in the settings file is longer than 100 chars this variable has to be changed
        // char* command = (char*)malloc(60); //a name of a variable can be no longer than 60 chars. 
        // char* data = (char*)malloc(60);  //the acutal value of a variable can be no longer than 60 chars.

        const char s[2] = ":";
        
        while (fgets(buf, sizeof(buf), f) != NULL) {
            if(strchr(buf, ':')){   // the ':' is used to split a variables name and parameter. if a line has no ':', it will probably be empty
            char* end = strchr(buf, '\0');  // this is the end of the buffer. index is required to remove the new line
            int indexOfEnd = (int)(end-buf);
            buf[indexOfEnd-1] = '\0';


            char* c = strtok(buf, s);   
            char* d = strtok(NULL, s);
            printf( " %s    :%s\n", c,d );

                
            //us a switch-case to fill in all the parameters to the right buffers    
           if(strcmp(c,"sample_rate") == 0){
                int sample_rate = atoi(d);
                printf("found sample rate: %d\n", sample_rate);
                audioConfig.sample_rate = sample_rate;
           } else if(strcmp(c,"bit_depth") == 0) {
                int bit_depth = atoi(d);
                printf("found bit depth: %d\n", bit_depth);
                audioConfig.bits_per_sample = bit_depth;
           } else if(strcmp(c,"num_channels") == 0) {
                int num_channels = atoi(d);
                printf("found num channels: %d\n", num_channels);
                audioConfig.num_channels = num_channels;
           }
                    
         
           }       
        }  
        // free(command);
        // free(data);               
        fclose(f);

}
void start_mdns_service()
{
    // _Static_assert(sizeof(c_config_hostname) < CONFIG_MAIN_TASK_STACK_SIZE/2, "Configured mDNS name consumes more than half of the stack. Please select a shorter host name or extend the main stack size please.");
    // const size_t config_hostname_len = sizeof(c_config_hostname) - 1; // without term char
    // char hostname[config_hostname_len + 1 + 3*2 + 1]; // adding underscore + 3 digits + term char
    // uint8_t mac[6];

    // // adding 3 LSBs from mac addr to setup a board specific name
    // esp_read_mac(mac, ESP_MAC_WIFI_STA);
    // snprintf(hostname, sizeof(hostname), "%s_%02x%02X%02X", c_config_hostname, mac[3], mac[4], mac[5]);

    // //initialize mDNS
    // ESP_ERROR_CHECK( mdns_init() );
    // //set mDNS hostname (required if you want to advertise services)
    // ESP_ERROR_CHECK( mdns_hostname_set(hostname) );
    // ESP_LOGI(TAG, "mdns hostname set to: [%s]", hostname);
    // //set default mDNS instance name
    // ESP_ERROR_CHECK( mdns_instance_name_set(EXAMPLE_MDNS_INSTANCE) );

    // //structure with TXT records
    // mdns_txt_item_t serviceTxtData[3] = {
    //     {"board","esp32"},
    //     {"u","user"},
    //     {"p","password"}
    // };

    // //initialize service
    // ESP_ERROR_CHECK( mdns_service_add("ESP32-WebServer", "_http", "_tcp", 80, serviceTxtData, 3) );
    // //add another TXT item
    // ESP_ERROR_CHECK( mdns_service_txt_item_set("_http", "_tcp", "path", "/foobar") );
    // //change TXT item value
    // ESP_ERROR_CHECK( mdns_service_txt_item_set("_http", "_tcp", "u", "admin") );
}

void setupPeripherals(esp_pin_config *pinconfig)
{
	ESP_LOGI(TAG, "setting up peripherals");
    setupI2C(pinconfig);  
	setupSPIFFS();
	setupNVS();			//also does a restart counter
	ESP_LOGI(TAG, "done setting up peripherals");
 
    


	//testsetup adc... can be deleted later
	adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC1_CHANNEL_0,ADC_ATTEN_DB_0);


}
void setupI2C(esp_pin_config *pinconfig)
{
    i2c_config_t conf;
	conf.mode = I2C_MODE_MASTER;
	conf.sda_io_num = (gpio_num_t)pinconfig->i2c_data;
	conf.scl_io_num = (gpio_num_t)pinconfig->i2c_clock;
	conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
	conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
	conf.master.clk_speed = I2C_CLOCKSPEED;    

	i2c_param_config(I2C_DRIVER_NUM, &conf);
	i2c_driver_install(I2C_DRIVER_NUM, I2C_MODE_MASTER, 0, 0, 0);
    // ESP_LOGI(TAG, "i2c_clk pin: %d",pinconfig->i2c_clock);
    // ESP_LOGI(TAG, "i2c_data pin: %d",pinconfig->i2c_data);
}
void configureGPIOExpander(){
    pca9535 * gh = sb.gpio_header;
	// shared_buffer->gpio_header->pinMode(shared_buffer->pin_config->led_red,PCA_INPUT,false);
    gh->digitalWrite(pinout.phy_reset,PCA_HIGH,true);
	gh->pinMode(sb.pin_config->sdDetect,PCA_INPUT,false);
	gh->pinMode(sb.pin_config->sdProtect,PCA_INPUT,false); //change this to true... trying to fix a bug
    gh->pinMode(sb.pin_config->sdPower,PCA_OUTPUT,false);
    gh->pinMode(sb.pin_config->phy_reset,PCA_OUTPUT,false);
    gh->pinMode(sb.pin_config->mic_select_0, PCA_OUTPUT,false);    
    gh->pinMode(sb.pin_config->mic_select_1,PCA_OUTPUT,false); 
    gh->pinMode(sb.pin_config->led_red,PCA_OUTPUT,false);
	gh->pinMode(sb.pin_config->led_yellow,PCA_OUTPUT,false);
	gh->pinMode(sb.pin_config->led_green,PCA_OUTPUT,false);
    gh->pinMode(sb.pin_config->led_green,PCA_OUTPUT,false);
	gh->pinMode(sb.pin_config->led_blue,PCA_OUTPUT,true); //last parameter true (flushes all the data)
    gh->digitalWrite(sb.pin_config->sdPower,PCA_HIGH,false);
    gh->digitalWrite(sb.pin_config->mic_select_0,PCA_HIGH,false);
    gh->digitalWrite(sb.pin_config->mic_select_1,PCA_HIGH,false);
    gh->digitalWrite(sb.pin_config->led_yellow,PCA_HIGH,false);
    gh->digitalWrite(sb.pin_config->led_red,PCA_LOW,true); //enable the red led. let the device decice when to turn it off (only happens if a sd card is mounted) (default of gpio header is low.. so this line could be remoced)
    
}

void setupInterruptBigButton(esp_pin_config *pinconfig){
    gpio_pad_select_gpio((gpio_num_t)pinconfig->big_button);
		// set the correct direction
	gpio_set_direction((gpio_num_t)pinconfig->big_button, GPIO_MODE_INPUT);
	// enable interrupt on falling (1->0) edge for button pin
	gpio_set_intr_type((gpio_num_t)pinconfig->big_button, GPIO_INTR_NEGEDGE);	
	// install ISR service with default configuration
	gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);	
	// attach the interrupt service routine
	gpio_isr_handler_add((gpio_num_t)pinconfig->big_button, button_isr_handler, NULL);
    //create a queue to handle gpio event from isr
    gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));
    //start gpio task
   
}
void setupSPIFFS(){
	ESP_LOGI(TAG, "Initializing SPIFFS");
    
    esp_vfs_spiffs_conf_t conf = {
      .base_path = "/spiffs",
      .partition_label = NULL,
      .max_files = 5,
      .format_if_mount_failed = true
    };
    
    // Use settings defined above to initialize and mount SPIFFS filesystem.
    // Note: esp_vfs_spiffs_register is an all-in-one convenience function.
    esp_err_t ret = esp_vfs_spiffs_register(&conf);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount or format filesystem");
        } else if (ret == ESP_ERR_NOT_FOUND) {
            ESP_LOGE(TAG, "Failed to find SPIFFS partition");
        } else {
            ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
        }
        return;
    }
    
    size_t total = 0, used = 0;
    ret = esp_spiffs_info(NULL, &total, &used);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get SPIFFS partition information (%s)", esp_err_to_name(ret));
    } else {
        ESP_LOGI(TAG, "Partition size: total: %d, used: %d", total, used);
    }
}
void testSPIFFSRead(){
	ESP_LOGI(TAG, "Reading file");
    FILE* f = fopen("/spiffs/foo.txt", "r");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for reading");
        //return;
    } else {
    char line[64];
    fgets(line, sizeof(line), f);
    fclose(f);
    // strip newline
    char* pos = strchr(line, '\n');
    if (pos) {
        *pos = '\0';
    }
    ESP_LOGI(TAG, "Read from file: '%s'", line);
	//esp_vfs_spiffs_unregister(NULL);
	}
}
void setupNVS(){
	// Initialize NVS
    nvs_handle my_NVS_handle;
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // NVS partition was truncated and needs to be erased
        // Retry nvs_flash_init
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK( err );

    // Open
    printf("\n");
    printf("Opening Non-Volatile Storage (NVS) handle... ");
    
    err = nvs_open("storage", NVS_READWRITE, &my_NVS_handle);
    if (err != ESP_OK) {
        printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
    } else {
        printf("Done\n");

        // Read
        printf("Reading restart counter from NVS ... ");
        int32_t restart_counter = 0; // value will default to 0, if not set yet in NVS
        err = nvs_get_i32(my_NVS_handle, "restart_counter", &restart_counter);
        switch (err) {
            case ESP_OK:
                printf("Done\n");
                printf("Restart counter = %d\n", restart_counter);
                break;
            case ESP_ERR_NVS_NOT_FOUND:
                printf("The value is not initialized yet!\n");
                break;
            default :
                printf("Error (%s) reading!\n", esp_err_to_name(err));
        }

        // Write
        printf("Updating restart counter in NVS ... ");
        restart_counter++;
        err = nvs_set_i32(my_NVS_handle, "restart_counter", restart_counter);
        printf((err != ESP_OK) ? "Failed!\n" : "Done\n");

        // Commit written value.
        // After setting any values, nvs_commit() must be called to ensure changes are written
        // to flash storage. Implementations may write to storage at other times,
        // but this is not guaranteed.
        printf("Committing updates in NVS ... ");
        err = nvs_commit(my_NVS_handle);
        printf((err != ESP_OK) ? "Failed!\n" : "Done\n");

        // Close
         nvs_close(my_NVS_handle); 
    }

    printf("\n");

}
