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
    ESP_LOGI(TAG, "i2c_clk pin: %d",pinconfig->i2c_clock);
    ESP_LOGI(TAG, "i2c_data pin: %d",pinconfig->i2c_data);
}
