


void apresa_task(esp_shared_buffer *shared_buffer){
	//check if there are files on SD that have not ye been synked with apresa, and handle them
	
	//if(!shared_buffer->apresaConnection->isSending()){
		//shared_buffer->apresaConnection->setFileName(182);
		// shared_buffer->apresaConnection->startSending();
	//}
	while(1){ //task should not end
	vTaskDelay(1000/portTICK_PERIOD_MS);
		if(shared_buffer->apresaConnection->isSending()){  //an other thread has called the "startSending()" method, which will let this fuction return true;
			shared_buffer->apresaConnection->sendFile();   //quick send method, might be changed later
			
		}


		
	}

}
