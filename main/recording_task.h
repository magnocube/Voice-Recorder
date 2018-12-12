
// now it will only do a write test with values from the adc
void recording_task(esp_shared_buffer *shared_buffer){  
	
	while(1){
		vTaskDelay(100/portTICK_PERIOD_MS);							// reset watchdog
		
		if(shared_buffer->recording == true){						//if the device should be recording
		
			
		
			
			//shared_buffer->SD->printCardInfo();
			shared_buffer->SD->generateNextFileName(); // generate a file name and places it into session_data->last_file_name;
			shared_buffer->SD->beginFile(shared_buffer->session_data->last_file_name);
			sb.gpio_header->digitalWrite(shared_buffer->pin_config->led_green,PCA_HIGH,true); //enable the green led
			int start = esp_log_timestamp();
			//int cal = adc1_get_raw(ADC1_CHANNEL_4); 				//12 bit adc value,. gpio32. this is a quick calibration value
			while(shared_buffer->recording == true){				//while it should be recording,,, take and store 512 samples
				shared_buffer->codec->read();
				uint8_t* data = shared_buffer->codec->audioBuffer1;				
				shared_buffer->SD->addDataToFile(data,AUDIO_BUFFER_SIZE);
				

				// uint8_t* m = (uint8_t *)malloc(512);	
				// for(int i = 0; i < 512; i+=2){						
				// 	int16_t data = adc1_get_raw(ADC1_CHANNEL_4) - cal ;  
				// 	m[i+1] = (data >> 8); 							//inverted, because of little endian
				// 	m[i] = (data);
				// }
				// shared_buffer->SD->addDataToFile(m,512);		 	//add the sampled data to the file
				// free(m);
			}
							
			int time = esp_log_timestamp() - start;
			printf("written in %d milliSeconds...\n", time);	
			
			shared_buffer->SD->endFile();							//write out the wav header and close the stream
			shared_buffer->gpio_header->digitalWrite(shared_buffer->pin_config->led_green,PCA_LOW,true);
			//ESP_LOGW(TAG, "4 ");
			/*should be recording,,, but card is not mounted...*/
			//flash red led!

		}
		// /vTaskDelay(100000/portTICK_PERIOD_MS);
	}
}
