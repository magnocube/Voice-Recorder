//errors might not show up,,, accidently disabled them
void Wifi_ethernet_interface_task(esp_shared_buffer *shared_buffer){   
	ESP_LOGI(TAG, "should be off... turning on after 5 seconds");
	vTaskDelay(5000/portTICK_PERIOD_MS);
	shared_buffer->recording = true;
	ESP_LOGI(TAG, "should be on... turning off after 60 seconds");
	gpio_pad_select_gpio(22);
    /* Set the GPIO as a push/pull output */
    gpio_set_direction((gpio_num_t)22, GPIO_MODE_OUTPUT);
	for(int i = 0; i<100000000;i++){
		 gpio_set_level((gpio_num_t)22, 0);
        //vTaskDelay(10 / portTICK_PERIOD_MS);
        /* Blink on (output high) */
        gpio_set_level((gpio_num_t)22, 1);
        //vTaskDelay(10 / portTICK_PERIOD_MS);
	}
	ESP_LOGI(TAG, "stop");
	shared_buffer->recording = false;
	while(1){
		
		ESP_LOGI(TAG, "XfreeHeapSize: %d",xPortGetFreeHeapSize());
		//heap_caps_print_heap_info(MALLOC_CAP_8BIT);
		vTaskDelay(1000/portTICK_PERIOD_MS);
		//
	}
}
