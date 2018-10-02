#include "main.h"

extern "C" { 														 //this just needs to be here
	void app_main();
}



void app_main()
{
	pinout = ESP_PIN_CONFIG_DEFAULT(); 				//change default pin-config in "settings.h"
	setupPeripherals(&pinout);						//setup for i2c, etc...
	audioConfig = ESP_AUDIO_CONFIG_DEFAULT();		//change default audio-config in "settings.h"

	pca9535 *pca_ptr = new pca9535(&pinout);
	SDCard *SD_ptr = new SDCard(&pinout, &audioConfig);
	WM8960 *audio_codec_ptr = new WM8960(&audioConfig, SD_ptr, pca_ptr, &pinout);
	sb = {	.recording = false,						//this shared buffer is passed to the tasks that will run each on a core. it contains all the pointers Both tasks might need.
			.gpio_header = pca_ptr,					
			.SD = SD_ptr,
			.codec = audio_codec_ptr,
			.audio_config = &audioConfig,
			.pin_config = &pinout			
			};
	 									
	

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
	esp_vfs_spiffs_unregister(NULL);
	}

	/*create a recording task. This task will handle the SD card and the I2S*/
    xTaskCreatePinnedToCore((TaskFunction_t)recording_task,						//task function		   
							 "recording_task", 									//task name 
							 1024 * 4, 											//stack size
							 &sb,												//function parameters (struct with pointers to shared classes)
							 1,													//priority
							 NULL,												//task handle
							 0													//task core
							 );	
							 
	/*create a "other task", this task will do everything else*/
	xTaskCreatePinnedToCore((TaskFunction_t)Wifi_ethernet_interface_task,		//task function		   //probably the tast that does everything except recording
							 "Wifi_ethernet_interface_task", 					//task name 
							 1024 * 2, 											//stack size
							 &sb,												//function parameters (struct with pointers to shared classes)
							 1,													//priority
							 NULL,												//task handle
							 1													//task core
							 );


	//while(1){ 											// -- this loop has cost me a couple of hours of hopeless debugging
	//	vTaskDelay(1000/portTICK_PERIOD_MS);   				//    when the main ends the structs pinconfig and audioconfig will be removed from the stack
	//}														//    and overwritten by other memory... took me some time to figure that out... this loop prevents
															// 	  the main from ending and keeping the 2 structs in memory. (i know... i m too lazy to place the
															//	  structs on the heap and pass a pointer to the required functions): update: i removed the
															// the structs from the stack. and placed them in the header... too lazy to remove the comment.
}

void setupPeripherals(esp_pin_config *pinconfig)
{
	ESP_LOGI(TAG, "setting up peripherals");
    setupI2C(pinconfig);
	setupSPIFFS();
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

	i2c_param_config(I2C_NUM_0, &conf);
	i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0);
    // ESP_LOGI(TAG, "i2c_clk pin: %d",pinconfig->i2c_clock);
    // ESP_LOGI(TAG, "i2c_data pin: %d",pinconfig->i2c_data);
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
