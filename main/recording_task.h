

void recording_task(esp_shared_buffer *shared_buffer){  
	uint8_t* monoData = (uint8_t*)malloc(AUDIO_BUFFER_SIZE/2);  // a buffer to store the data when recording in mono.
	while(1){
		vTaskDelay(100/portTICK_PERIOD_MS);						
		
		if(shared_buffer->recording == true){						//if the device should be recording
		
			
		
			
			shared_buffer->SD->printCardInfo();
			shared_buffer->SD->generateNextFileName();										  // generate a file name and places it into session_data->last_file_name;
			shared_buffer->SD->beginFile(shared_buffer->session_data->last_file_name);

			if(sb.audio_config->enablePhantom){														//can be configured with the web interface. 
				sb.gpio_header->digitalWrite(shared_buffer->pin_config->enable48V,PCA_HIGH,false);  // enable the phantom power cirquit
			}
			sb.gpio_header->digitalWrite(shared_buffer->pin_config->led_green,PCA_HIGH,true); //enable the green led to indicate the recording has started
			int start = esp_log_timestamp();												  //time to indicate the time the device has been recording								
			//int cal = adc1_get_raw(ADC1_CHANNEL_4); 				//12 bit adc value,. gpio32. this is a quick calibration value
			while(shared_buffer->recording == true){				//while it should be recording,,, take and store AUDIO_BUFFER_SIZE samples
				shared_buffer->codec->read();						//reads the content from the i2s bus, and stores it into 'audioBuffer1'
				uint8_t* data = shared_buffer->codec->audioBuffer1;		//aqquire a pointer to that buffer (TODO: the buffer location won't change... aqquiering the pointer should only be done once when the recording starts)
				if(shared_buffer->audio_config->num_channels == 2){								//when recording in Stereo
					shared_buffer->SD->addDataToFile(data,AUDIO_BUFFER_SIZE);					//just write the raw i2s data to SD
				}else {																			//else it is recording with mono settings... the codec still delivers a stereo signal
					for (int i = 0; i < AUDIO_BUFFER_SIZE; i+=4)  //records only first channel
					{
						monoData[i/2]   = data[i];
						monoData[i/2+1] =  data[i+1];
						
					}
					shared_buffer->SD->addDataToFile(monoData,AUDIO_BUFFER_SIZE/2);
				}	
				
				
				// /shared_buffer->audio_config->num_channels == 2
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
			shared_buffer->gpio_header->digitalWrite(shared_buffer->pin_config->enable48V,PCA_LOW,false);   // disable phanton power( regarding if it was on or not)
			shared_buffer->gpio_header->digitalWrite(shared_buffer->pin_config->led_green,PCA_LOW,true);	// indicate that the recodring is done (green led goes off)
			//ESP_LOGW(TAG, "4 ");
			/*should be recording,,, but card is not mounted...*/
			//flash red led!

		} else { //the device should not be recording. so let's check for sound to automaticly activate

		}
		// /vTaskDelay(100000/portTICK_PERIOD_MS);
	}
}
