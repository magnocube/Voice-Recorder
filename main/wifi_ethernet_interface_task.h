//errors might not show up,,, accidently disabled them
void Wifi_ethernet_interface_task(esp_shared_buffer *shared_buffer){   
	ESP_LOGI(TAG, "should be off... turning on after 60 seconds");
	vTaskDelay(60000/portTICK_PERIOD_MS);
	shared_buffer->recording = true;
	ESP_LOGI(TAG, "should be on... turning off after 30 seconds");
	vTaskDelay(30000/portTICK_PERIOD_MS);
	ESP_LOGI(TAG, "stop");
	shared_buffer->recording = false;
	while(1){
		
		ESP_LOGI(TAG, "XfreeHeapSize: %d",xPortGetFreeHeapSize());
		//heap_caps_print_heap_info(MALLOC_CAP_8BIT);
		vTaskDelay(1000/portTICK_PERIOD_MS);
		//
	}
}
