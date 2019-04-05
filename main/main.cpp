/*
when reading trhough the code, start at settings.h.
then read main.h and main.cpp
pressing F12 while selecting a function will jump to the declaration of that funtion (VScode)
*/

#include "main.h"

extern "C" { 	
	void app_main();
}


/* Whenever the button on the device is pressed, this interrupt handler will be called. 
* It will send a signal to that will be received by the 'gpio_task_example'. 
* The 'gpio_task_example' contains the logic that should happen when the button is pressed.*/
void IRAM_ATTR button_isr_handler(void* arg) { 
    uint32_t gpio_num = (uint32_t) arg;
    xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
}

/* This function will implement the logic that handles the pressing of the botton on top of the case.
* It contains an infinity loop (because it is a task runned by RTOS) that every time it starts checks if there has been a signal from the button.
* If a signal from the button (interupt handler) has been received the funtion will check if some time has been passed (default 1000ms).
* This is because the interrupt handler might send multiple signals when the button is not pressed perfectly.
* After that the logic is preformed.:
* If the device is not recording., it will check if a SD is mounted and sets a global recording variable on 'true'
* Else it will turn that recording variable to false.*/
static void gpio_task_example(void* arg)  // might need some cleaning
{
    uint32_t lastTimeInterrupt = 0;
    uint32_t io_num; // to store the argument of the interupt handler
    for(;;) {
        if(xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY)) { // continues if a signal from the interrupt handler has been send.
            if(!sb.session_data->is_in_TestModus){
                uint32_t timeNow = esp_log_timestamp();                 
                if(timeNow - lastTimeInterrupt >= 1000){  // to prevent 2 fast interrupts... at least 1000 milliseconds between a interrupt (minimum recording length is 1 second... and 1 second between each recording)
                    
                    printf("time since last button: %d",timeNow - lastTimeInterrupt);
                    lastTimeInterrupt = timeNow;                
                    ESP_LOGI(TAG, "Whoop Whoop, Button has been pressed!, Whoop Whoop.");
                    
                    if(sb.recording == false){                      //when the device is not in recording mode.
                        if(sb.SD->isMounted()){						//and the card is acually mounted
                            
                                    
                                    
                                    
                                //TODO"::::::::::::: implement free file size detection and handling  (this is just a test to indicate it's working)!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! 
                                    FATFS *fs;
                                    DWORD fre_clust, fre_sect, tot_sect;
                                    /* Get volume information and free clusters of drive 0 */
                                    f_getfree("0:", &fre_clust, &fs);
                                    /* Get total sectors and free sectors */
                                    tot_sect = (fs->n_fatent - 2) * fs->csize;
                                    fre_sect = fre_clust * fs->csize;
                                    /* Print the free space (assuming 512 bytes/sector) */
                                    printf("\n\n\n\n%10lu KiB total drive space.\n%10lu KiB available.\n\n\n",tot_sect / 2, fre_sect / 2);





                                if(sb.session_data->SD_Write_Protect_on == false){
                                    sb.recording = true;                
                                    ESP_LOGI(TAG, "recording set to true");
                                }else{
                                    ESP_LOGE(TAG, "ERROR. write protection is on... can't start a recording");
                                }
                            
                        } else{
                            ESP_LOGE(TAG, "not recording... card is not in slot");
                        }
                                                
                    }else{
                        sb.recording = false;
                        sb.gpio_header->digitalWrite(pinout.led_green,PCA_LOW,true); //turn of the recording led
                        /*NOTE: THE RECORDING MAY CONTINUE IF THE SD CARD HAS PROBLEMS, THIS ONLY SETS THE FLAG 'RECORDING' false*/
                        /*THE RECORDING TASK COULD BE STUCK ON READING THE I2S BUS*/
                    }
                } else {

                }
            }
            vTaskDelay(100/portTICK_PERIOD_MS);; //prevent loss of CPU power
        }
    }
}

/*The app main runs every time the program is started. see the flow diagram for the logic. */
void app_main()
{

   
	pinout = ESP_PIN_CONFIG_DEFAULT(); 			                	//change default pin-config in "settings.h"	
	audioConfig = ESP_AUDIO_CONFIG_DEFAULT();		                //change default audio-config in "settings.h"  --> this config will be changed after loading the settings from spiffs
    sessionData = ESP_SESSION_DATA_DEFAULT();                       //change default parameters in "settings.h"
    strcpy(sessionData.last_file_name,"/sdcard/notDefined.wav");    //set the default value of the file name.
    strcpy(sessionData.Ethernet_IP_Adress,"NO ADRESS");             //set the default value of the Ethernet IP (for the browser)
    esp_read_mac(sessionData.macAdress,ESP_MAC_ETH);                //reads the mac adress

    
    /*converts the mac adress to a string for the wifi password*/
    unsigned char * pin = sessionData.macAdress;
    const char * hex = "0123456789ABCDEF";
    uint8_t * pout = sessionData.macAdressString;
    int i = 0;
    for(; i < MAC_SIZE-1; ++i){
        *pout++ = hex[(*pin>>4)&0xF];
        *pout++ = hex[(*pin++)&0xF];
        *pout++ = ':';
    }
    *pout++ = hex[(*pin>>4)&0xF];
    *pout++ = hex[(*pin)&0xF];
    *pout = 0;

    printf("Mac Adress in String Format:   ----- %s -----\n", sessionData.macAdressString);
    
    setupInterruptBigButton(&pinout);                               //setup the big button on top of the case (set pinmode and assign interrupt handler)
        

    /*
    * this block will see of the button is pressed( when de device has just booted up) and then determines if the divice should be in test mode.
    */    
    ESP_LOGI(TAG,"READING BUTTON (FOR TESTING PURPOSES :D)");
    sessionData.is_in_TestModus = !gpio_get_level((gpio_num_t)pinout.big_button);          //read the large button and invert the input
    ESP_LOGI(TAG, "divice modus. 1=testmodus, 0=normal operation:  %d", sessionData.is_in_TestModus);
    if(sessionData.is_in_TestModus){
        printf("**********************************************\n*\n*\n");
         printf("DEVICE WILL START IN TEST MODE\n*\n*\n");
          printf("**********************************************\n");
        vTaskDelay(5000/portTICK_PERIOD_MS); //make clear device is in test mode. keep this test some seconds on the screen.
    } else{
        printf("**********************************************\n*\n*\n");
         printf("DEVICE WILL START IN NORMAL OPERATING MODE\n*\n*\n");
          printf("**********************************************\n");
        //idk, Windows XP startup sound maybe :D?
    }


    setupPeripherals(&pinout);					                	//setup for i2c and spiffs... all other peripherals have been moved to other classes
    setupDeviceSettingsFromSPIFFS();                                //will fill the corresponding config-structs with the desired settings (settings can be changed using the browser and a restart, or to reflash the storage-partition)   
   

	pca9535 *pca_ptr = new pca9535(&pinout, &sessionData);                                  //make instance of the i2c-GPIO-expander
	SDCard *SD_ptr = new SDCard(&pinout, &audioConfig, pca_ptr,&sessionData);               //make instance of the SD card interface
	WM8960 *audio_codec_ptr = new WM8960(&audioConfig, SD_ptr, pca_ptr, &pinout);           //make instance of the audio codec interface
    Apresa *apresa_connection_ptr = new Apresa();
	sb = {	.recording = false,						                                        //this shared_buffer is passed to the different tasks it contains all the pointers Both tasks need.
			.gpio_header = pca_ptr,					                                        /*TODO: rename 'shared_buffer' to 'shared_memory'*/  
			.SD = SD_ptr,
			.codec = audio_codec_ptr,
			.audio_config = &audioConfig,
			.pin_config = &pinout,
            .session_data = &sessionData,	
            .apresaConnection = apresa_connection_ptr
	};

	testSPIFFSRead();	      //TODO... delete or move this to test code. 
    configureGPIOExpander();  // sets all the required pinmodes (can be changed dynamicly anywhere in the code) to make sure all the hardware connected to it start up correctly.


    audio_codec_ptr->printCopyCodecRegisters();             //print out a copy of the codec registers. this is to verify the the programmed config with the data to be printed.

	

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

    
    // xTaskCreatePinnedToCore((TaskFunction_t)webInterface,	                	//task function		   //handles the webpage   --> does not exist anymore
	// 						 "webInterface_task", 					            //task name 
	// 						 1024 * 8, 											//stack size
	// 						 &sb,												//function parameters (struct with pointers to shared classes)
	// 						 1,													//priority
	// 						 NULL,												//task handle
	// 						 1													//task core
	// 						 );

    xTaskCreatePinnedToCore((TaskFunction_t)gpio_task_example,		            //task function		   //hanles the big button on the case(when an interupt is send)
							 "button_task", 					                //task name 
							 1024 * 2, 											//stack size
							 NULL,												//function parameters (struct with pointers to shared classes)
							 1,													//priority
							 NULL,												//task handle
							 1													//task core
							 );

    //captdnsInit(); // just testing... this should provide dns... please just remove and use a static IP if this is not desired. currently it will only send a static IP on every request

    if(sessionData.is_in_TestModus){                                                                       /*only create this thread when the device should start in test mode.*/
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


/*
*This method will read the settings.txt from the spiffs memory.
*It will analyse it line by line and sets the struct to the good position.
*/
void setupDeviceSettingsFromSPIFFS(){
    FILE* f = fopen("/spiffs/settings.txt", "r");
        fseek(f, 0, SEEK_END);
        int fileSize = ftell(f);
        fseek(f,0,SEEK_SET);
        ESP_LOGI(TAG, "size of settigns file: ... size: %d\n", fileSize);

        
        char buf[100]; 
      
        const char s[2] = ":";
        
        while (fgets(buf, sizeof(buf), f) != NULL) {
            if(strchr(buf, ':')){   // the ':' is used to split a variables name and parameter. if a line has no ':', it will probably be empty
            char* end = strchr(buf, '\0');  // this is the end of the buffer. index is required to remove the new line
            int indexOfEnd = (int)(end-buf);
            buf[indexOfEnd] = '\0'; // just to make sure ;-)


            char* c = strtok(buf, s);   //command
            char* d = strtok(NULL, s);   //data
            printf( " %s    :%s", c,d );

                
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
           } else if(strcmp(c,"channel1") == 0) {
               if(strcmp(d,"BuildIn") == 0) {
                    audioConfig.channel1 = MIC_BUILD_IN;
               } else if(strcmp(d,"3.5") == 0) {
                    audioConfig.channel1 = MIC_EXTERNAL_3_5_mm;
               } else if(strcmp(d,"5.0") == 0) {
                    audioConfig.channel1 = MIC_EXTERNAL_5_0_mm;
               }             
                printf("channel1 value: %d\n", audioConfig.channel1);
           } else if(strcmp(c,"channel2") == 0) {
               if(strcmp(d,"BuildIn") == 0) {
                    audioConfig.channel2 = MIC_BUILD_IN;
               } else if(strcmp(d,"3.5") == 0) {
                    audioConfig.channel2 = MIC_EXTERNAL_3_5_mm;
               } else if(strcmp(d,"5") == 0) {
                    audioConfig.channel2 = MIC_EXTERNAL_5_0_mm;
               }             
                printf("channel2 value: %d\n", audioConfig.channel2);
           } else if(strcmp(c,"enablePhantom") == 0) {
                int state = atoi(d);
                printf("found phantom state: %d\n", state);
                audioConfig.enablePhantom = state;
           }
           //:TODO: add more settings
                    
         
           }       
        }  
        // free(command);
        // free(data);               
        fclose(f);

}

void setupPeripherals(esp_pin_config *pinconfig)
{
	ESP_LOGI(TAG, "setting up peripherals");
    setupI2C(pinconfig);  
	setupSPIFFS();
	setupNVS();			//this does not a setup, but increments a restart counter.
	ESP_LOGI(TAG, "done setting up peripherals");
 


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

/*The GPIO expander is the i2c gpio (pca9535) IC. 
See pinconfig or hardware schematic to see where pins are connected.
these are only the defaults... and will be overwritten very soon*/
void configureGPIOExpander(){
    pca9535 * gh = sb.gpio_header;
    gh->digitalWrite(pinout.phy_reset,PCA_HIGH,true);
    gh->pinMode(sb.pin_config->enable48V,PCA_OUTPUT,false);
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
    gh->pinMode(sb.pin_config->ethernet_up_led,PCA_OUTPUT,false);
	gh->pinMode(sb.pin_config->led_blue,PCA_OUTPUT,true); //last parameter true (flushes all the data)
    gh->digitalWrite(sb.pin_config->enable48V,PCA_LOW,false);  // HIGH = on, LOW = off
    gh->digitalWrite(sb.pin_config->sdPower,PCA_HIGH,false);  //low = on, High = off
    gh->digitalWrite(sb.pin_config->mic_select_0,PCA_HIGH,false); //high = build in ,, low = extern (3.5mm)
    gh->digitalWrite(sb.pin_config->mic_select_1,PCA_HIGH,false); //high = build in ,, low = extern (5mm)
    gh->digitalWrite(sb.pin_config->led_yellow,PCA_HIGH,false);
    gh->digitalWrite(sb.pin_config->ethernet_up_led,PCA_HIGH,false); //LOW = on
    gh->digitalWrite(sb.pin_config->led_blue,PCA_LOW,false);        //HIGH = on
    gh->digitalWrite(sb.pin_config->led_red,PCA_LOW,true); //enable the red led. let the device decice when to turn it off (only happens if a sd card is mounted) (default of gpio header is low.. so this line could be remoced)
    
}

/*The Button on top of the case is used for recordings, whenever the button gets pressed a interrupt must be send. 
The interrupt will be handled by the interrupt handler ('button_isr_handler')*/
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
      
}
/*SPIFFS = SPI Fat FileSystem. it acts like the SD card, but needs a seperate setup*/
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

/*redunant function... the testcode in 'test.h' does the same thing.. this can be deleted*/
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

/*this function does increment a restart counter, while at the same time it indicates if there are problems with NVS*/
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
