#include "main.h"
extern "C" {
	void app_main();
}

void recording_task(){  // now it will only do a write speed test
	char* m = "POOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOQ";
	SD->printCardInfo();
	SD->beginFile();
	int start = esp_log_timestamp();
	for(int i = 0 ; i < 10000; i++){
		SD->addDataToFile(m);
	}
	int time = esp_log_timestamp() - start;
	printf("written in %d milliSeconds...\n", time);
	
	SD->endFile(); //write out the wav header and close the stream
	while(1){
		//codec->send_I2C_command();
		ESP_LOGI(TAG, "XfreeHeapSize: %d",xPortGetFreeHeapSize());
		//heap_caps_print_heap_info(MALLOC_CAP_8BIT);
		vTaskDelay(5000/portTICK_PERIOD_MS);
	}
}
void Wifi_ethernet_interface_task(){   //this task will controll the device(using wifi, ethernet or the big button)

	while(1){
		vTaskDelay(10/portTICK_PERIOD_MS);
	}
}
void app_main()
{
	esp_pin_config pinout = ESP_PIN_CONFIG_DEFAULT(); //change default config in "settings.h"
	setupPeripherals(&pinout);  //setup for i2s, i2c, etc...
	codec = new WM8960();       //defining the global Codec instance
	SD = new SDCard(&pinout);	//defining the global SD instance, will try to mount the SD
	
	
	

    xTaskCreatePinnedToCore((TaskFunction_t)recording_task,		//task function		   //probably the recording task
							 "recording_task", 					//taks name 
							 1024 * 2, 							//stack size
							 NULL,								//function parameters
							 1,									//priority
							 NULL,								//task handle
							 0									//task core
							 );
	  xTaskCreatePinnedToCore((TaskFunction_t)Wifi_ethernet_interface_task,		//task function		   //probably the recording task
							 "Wifi_ethernet_interface_task", 					//taks name 
							 1024 * 2, 							//stack size
							 NULL,								//function parameters
							 1,									//priority
							 NULL,								//task handle
							 1									//task core
							 );



}

void setupPeripherals(esp_pin_config *pinconfig)
{
	ESP_LOGI(TAG, "setting up peripherals");
    setupI2C(pinconfig);
    setupI2S(pinconfig);

	vTaskDelay(100/portTICK_PERIOD_MS);
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

	i2c_param_config(I2C_NUM_0, &conf);
	i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0);
    ESP_LOGI(TAG, "i2c_clk pin: %d",pinconfig->i2c_clock);
    ESP_LOGI(TAG, "i2c_data pin: %d",pinconfig->i2c_data);
}
void setupI2S(esp_pin_config *pinconfig)
{
    // int i2s_num = EXAMPLE_I2S_NUM;
	//  i2s_config_t i2s_config = {
    //     .mode = I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_TX | I2S_MODE_DAC_BUILT_IN | I2S_MODE_ADC_BUILT_IN,
    //     .sample_rate =  EXAMPLE_I2S_SAMPLE_RATE,
    //     .bits_per_sample = EXAMPLE_I2S_SAMPLE_BITS,
	//     .communication_format = I2S_COMM_FORMAT_I2S_MSB,
	//     .channel_format = EXAMPLE_I2S_FORMAT,
	//     .intr_alloc_flags = 0,
	//     .dma_buf_count = 2,
	//     .dma_buf_len = 1024
	//  };
	//  //install and start i2s driver
	//  i2s_driver_install(i2s_num, &i2s_config, 0, NULL);
	//  //init DAC pad
	//  i2s_set_dac_mode(I2S_DAC_CHANNEL_BOTH_EN);
	//  //init ADC pad
	//  i2s_set_adc_mode(I2S_ADC_UNIT, I2S_ADC_CHANNEL);
}
