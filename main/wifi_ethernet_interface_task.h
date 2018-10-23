//errors might not show up,,, accidently disabled them
#include "esp_event_loop.h"
#include "tcpip_adapter.h"
#include "driver/periph_ctrl.h"
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"
 
#include "webInterface.h"   //code that will handle the web interface



static void phy_device_power_enable_via_gpio(bool enable);				 //function prototype
static void eth_gpio_config_rmii(void); 								 //function prototype
static esp_err_t eth_event_handler(void *ctx, system_event_t *event);    //function prototype

void Wifi_ethernet_interface_task(esp_shared_buffer *shared_buffer){   

	 
   // ESP_ERROR_CHECK( nvs_flash_init() );
    tcpip_adapter_init();

    ESP_ERROR_CHECK(esp_event_loop_init(eth_event_handler, NULL));

    eth_config_t config = DEFAULT_ETHERNET_PHY_CONFIG;
    config.phy_addr = static_cast<eth_phy_base_t>(0);
    config.gpio_config = eth_gpio_config_rmii;
    config.tcpip_input = tcpip_adapter_eth_input;
    config.clock_mode = static_cast<eth_clock_mode_t>(0);   //gpio 0 clock in

    /* Replace the default 'power enable' function with an example-specific one
     that toggles a power GPIO. */
    config.phy_power_enable = phy_device_power_enable_via_gpio;


    ESP_ERROR_CHECK(esp_eth_init(&config));
    ESP_ERROR_CHECK(esp_eth_enable()) ;
    vTaskDelay(5000/portTICK_PERIOD_MS);
    
	/*webinterface task gets created in main... TODO... call a signal that the task can start*/
	
// 		int redLed = shared_buffer->pin_config->led_red;
// 	int yellowLed = shared_buffer->pin_config->led_yellow;
// 	int greenLed = shared_buffer->pin_config->led_green;
// 	int blueLed = shared_buffer->pin_config->led_blue;
// 	int detectSd = shared_buffer->pin_config->sdDetect;
// 	int detectProtect = shared_buffer->pin_config->sdProtect;


	
// 	// shared_buffer->recording = true;
// 	// vTaskDelay(5000/portTICK_PERIOD_MS);
// 	// shared_buffer->recording = false;
// pca9535 * gh = shared_buffer->gpio_header;	
	
	while(1){

			if(!shared_buffer->SD->isMounted()){	// sd card is in slot
				shared_buffer->recording = false;
				//flash led 

				// gh->digitalWrite(redLed,PCA_LOW,false);
				// gh->digitalWrite(yellowLed,PCA_HIGH,false);
				// gh->digitalWrite(greenLed,PCA_HIGH,false);
				// gh->digitalWrite(blueLed,PCA_HIGH,true);
			} else {							//sd card is not in slot
				//shared_buffer->SD->isCardMounted = false;
				// gh->digitalWrite(redLed,PCA_HIGH,false);
				// gh->digitalWrite(yellowLed,PCA_LOW,false);
				// gh->digitalWrite(greenLed,PCA_LOW,false);
				// gh->digitalWrite(blueLed,PCA_LOW,true);
			}
			vTaskDelay(200/portTICK_PERIOD_MS);

	}
			 

			// printf("value of pinDetect: %d   and pinProtect: %d \n", gh->digitalRead(detectSd,true),gh->digitalRead(detectProtect,false));	

			  
	
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


static void phy_device_power_enable_via_gpio(bool enable)
{
    assert(DEFAULT_ETHERNET_PHY_CONFIG.phy_power_enable);

    if (!enable) {
        DEFAULT_ETHERNET_PHY_CONFIG.phy_power_enable(false);
    }

    gpio_pad_select_gpio(static_cast<gpio_num_t>(PIN_PHY_POWER));
    gpio_set_direction(static_cast<gpio_num_t>(PIN_PHY_POWER), GPIO_MODE_OUTPUT);
    if (enable == true) {
        gpio_set_level(static_cast<gpio_num_t>(PIN_PHY_POWER), 1);
        ESP_LOGI(TAG, "Power On Ethernet PHY");
    } else {
        gpio_set_level(static_cast<gpio_num_t>(PIN_PHY_POWER), 0);
        ESP_LOGI(TAG, "Power Off Ethernet PHY");
    }

    vTaskDelay(1); // Allow the power up/down to take effect, min 300us

    if (enable) {
        /* call the default PHY-specific power on function */
        DEFAULT_ETHERNET_PHY_CONFIG.phy_power_enable(true);
    }
}


static void eth_gpio_config_rmii(void)
{
    phy_rmii_configure_data_interface_pins();
    phy_rmii_smi_configure_pins(PIN_SMI_MDC, PIN_SMI_MDIO);
}

static esp_err_t eth_event_handler(void *ctx, system_event_t *event)
{
    tcpip_adapter_ip_info_t ip;

    switch (event->event_id) {
    case SYSTEM_EVENT_ETH_CONNECTED:
        ESP_LOGI(TAG, "Ethernet Link Up");
		sb.gpio_header->digitalWrite(sb.pin_config->led_blue,PCA_HIGH,true);
        break;
    case SYSTEM_EVENT_ETH_DISCONNECTED:
        ESP_LOGI(TAG, "Ethernet Link Down");
		sb.gpio_header->digitalWrite(sb.pin_config->led_blue,PCA_LOW,true);
        break;
    case SYSTEM_EVENT_ETH_START:
        ESP_LOGI(TAG, "Ethernet Started");
        break;
    case SYSTEM_EVENT_ETH_GOT_IP:
        memset(&ip, 0, sizeof(tcpip_adapter_ip_info_t));
        ESP_ERROR_CHECK(tcpip_adapter_get_ip_info(static_cast<tcpip_adapter_if_t>(ESP_IF_ETH), &ip));
        ESP_LOGI(TAG, "Ethernet Got IP Addr");
        ESP_LOGI(TAG, "~~~~~~~~~~~");
        ESP_LOGI(TAG, "ETHIP:" IPSTR, IP2STR(&ip.ip));
        ESP_LOGI(TAG, "ETHMASK:" IPSTR, IP2STR(&ip.netmask));
        ESP_LOGI(TAG, "ETHGW:" IPSTR, IP2STR(&ip.gw));
        ESP_LOGI(TAG, "~~~~~~~~~~~");
        break;
    case SYSTEM_EVENT_ETH_STOP:
        ESP_LOGI(TAG, "Ethernet Stopped");
        break;
    default:
        break;
    }
    return ESP_OK;
}