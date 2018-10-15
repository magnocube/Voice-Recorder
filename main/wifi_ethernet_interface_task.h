//errors might not show up,,, accidently disabled them
void Wifi_ethernet_interface_task(esp_shared_buffer *shared_buffer){   
	int redLed = shared_buffer->pin_config->led_red;
	int yellowLed = shared_buffer->pin_config->led_yellow;
	pca9535 * gh = shared_buffer->gpio_header;
	gh->pinMode(redLed,PCA_OUTPUT,false);
	gh->pinMode(yellowLed,PCA_OUTPUT,true);

	while(1){
			
			 ESP_LOGI(TAG, "red on ");
			gh->digitalWrite(redLed,PCA_HIGH,false);
			gh->digitalWrite(yellowLed,PCA_HIGH,true);
			 vTaskDelay(100/portTICK_PERIOD_MS); //reset watchdog
			 ESP_LOGI(TAG, "red off");
			gh->digitalWrite(redLed,PCA_LOW,false);
			gh->digitalWrite(yellowLed,PCA_LOW,true);
			 vTaskDelay(100/portTICK_PERIOD_MS); //reset watchdog
	}
	// ESP_LOGI(TAG, "should be off... turning on after 5 seconds");
	// vTaskDelay(5000/portTICK_PERIOD_MS);
	// shared_buffer->codec->printCopyCodecRegisters();
	// shared_buffer->recording = true;
	// ESP_LOGI(TAG, "should be on... turning off after 60 seconds");

	// 	// esp_err_t err = nvs_open("storage", NVS_READWRITE, &shared_buffer->my_NVS_handle);
	// 	// if (err != ESP_OK) {
	// 	// 	printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
	// 	// }	
	//     //  err = nvs_set_i32(shared_buffer->my_NVS_handle, "restart_counter", 42069);
    //     // printf((err != ESP_OK) ? "Failed!\n" : "Done\n");
    //     // printf("Committing updates in NVS ... ");
    //     // err = nvs_commit(shared_buffer->my_NVS_handle);
    //     // printf((err != ESP_OK) ? "Failed!\n" : "Done\n");



	// gpio_pad_select_gpio(22);
    // /* Set the GPIO as a push/pull output */
    // gpio_set_direction((gpio_num_t)22, GPIO_MODE_OUTPUT);
	// for(int i = 0; i<100000000;i++){
	// 	 gpio_set_level((gpio_num_t)22, 0);
    //     //vTaskDelay(10 / portTICK_PERIOD_MS);
    //     /* Blink on (output high) */
    //     gpio_set_level((gpio_num_t)22, 1);
    //     //vTaskDelay(10 / portTICK_PERIOD_MS);
	// }
	// ESP_LOGI(TAG, "stop");
	// shared_buffer->recording = false;


	// while(1){
		
	// 	ESP_LOGI(TAG, "XfreeHeapSize: %d",xPortGetFreeHeapSize());
	// 	//heap_caps_print_heap_info(MALLOC_CAP_8BIT);
	// 	vTaskDelay(1000/portTICK_PERIOD_MS);
	// 	//
	// }
}
