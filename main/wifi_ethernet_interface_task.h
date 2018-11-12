//errors might not show up,,, accidently disabled them
#include "esp_event_loop.h"
#include "tcpip_adapter.h"
#include "driver/periph_ctrl.h"
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"
#include "esp_wifi.h"
 
#include "webInterface.h"   //code that will handle the web interface



static void phy_device_power_enable_via_gpio(bool enable);				 //function prototype   -> gets automaticly called from ehternet driver, will allow clock to flow to gpio0
static void eth_gpio_config_rmii(void); 								 //function prototype   -> gets automaticly called from ehternet driver
static esp_err_t eth_event_handler(void *ctx, system_event_t *event);    //function prototype   -> event handler. usefull for debugging
void wifi_init_softap();                                                 //function prototype   -> enable the wifi
void ethernet_init();                                                    //function prototype   -> enable Ethernet
static esp_err_t event_handler(void *ctx, system_event_t *event);        // not used yet

static EventGroupHandle_t s_wifi_event_group;

void Wifi_ethernet_interface_task(esp_shared_buffer *shared_buffer){   

    tcpip_adapter_init();                                                       
    ethernet_init();
    wifi_init_softap();
    

    // shared_buffer->recording = true;
    // vTaskDelay(60000/portTICK_PERIOD_MS);
    // shared_buffer->recording = false;
	while(1){
        // while(1){
        //     vTaskDelay(1000/portTICK_PERIOD_MS);
        // }

			if(!shared_buffer->SD->isMounted()){	        // sd card is not in slot... isMounted function takes care of the SD card,, just call it reguarly
				shared_buffer->recording = false;
                sb.gpio_header->digitalWrite(shared_buffer->pin_config->led_green,PCA_LOW,true);
                /*may the odds be ever in your favor*/
			} else {						            	//sd card is in slot
				//do nothing... all the leds should already be in the right position... this function can be used to blink the leds?
			}
			vTaskDelay(200/portTICK_PERIOD_MS);
            ESP_LOGI(TAG, "XfreeHeapSize: %d",xPortGetFreeHeapSize());

	}
			 


}


static void phy_device_power_enable_via_gpio(bool enable) //toggles the chip thats blocks the clock to pass the clock to gpio 0
{
    assert(DEFAULT_ETHERNET_PHY_CONFIG.phy_power_enable);

    if (!enable) {
        DEFAULT_ETHERNET_PHY_CONFIG.phy_power_enable(false);
    }

    gpio_pad_select_gpio(static_cast<gpio_num_t>(PIN_PHY_POWER));
    gpio_set_direction(static_cast<gpio_num_t>(PIN_PHY_POWER), GPIO_MODE_OUTPUT);
    if (enable == true) {
        gpio_set_level(static_cast<gpio_num_t>(PIN_PHY_POWER), 0);
        ESP_LOGI(TAG, "Power On Ethernet PHY");
    } else {
        gpio_set_level(static_cast<gpio_num_t>(PIN_PHY_POWER), 1);
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
static esp_err_t event_handler(void *ctx, system_event_t *event)   //wifi
{
    switch(event->event_id) {
    case SYSTEM_EVENT_AP_STACONNECTED:
        // ESP_LOGI(TAG, "station:"MACSTR" join, AID=%d",
        //          MAC2STR(event->event_info.sta_connected.mac),
        //         event->event_info.sta_connected.aid);
                 ESP_LOGI(TAG, "wifi connected");
        break;
    case SYSTEM_EVENT_AP_STADISCONNECTED:
        //ESP_LOGI(TAG, "station:"MACSTR"leave, AID=%d",
        //         MAC2STR(event->event_info.sta_disconnected.mac),
         //        event->event_info.sta_disconnected.aid);
         ESP_LOGI(TAG, "wifi disconnected");
        break;
    default:
        break;
    }
    return ESP_OK;
}
void wifi_init_softap()
{
    /*set a static IP*/
    tcpip_adapter_dhcps_stop(TCPIP_ADAPTER_IF_AP);
    tcpip_adapter_ip_info_t ip_info;
    IP4_ADDR(&ip_info.ip,192,168,1,1);
    IP4_ADDR(&ip_info.gw,192,168,1,1);
    IP4_ADDR(&ip_info.netmask,255,255,255,0);
    printf("set ip ret (0 means OK, others mean FAIL): %d\n", tcpip_adapter_set_ip_info(TCPIP_ADAPTER_IF_AP, &ip_info)); //set static IP
    tcpip_adapter_dhcps_start(TCPIP_ADAPTER_IF_AP); 

    
    s_wifi_event_group = xEventGroupCreate(); // not used yet
    //ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL));
	wifi_init_config_t wifiInitializationConfig = WIFI_INIT_CONFIG_DEFAULT(); 
    esp_wifi_init(&wifiInitializationConfig); 
    esp_wifi_set_storage(WIFI_STORAGE_RAM); 
    esp_wifi_set_mode(WIFI_MODE_AP); 

    wifi_ap_config_t acfg = {EXAMPLE_ESP_WIFI_SSID,EXAMPLE_ESP_WIFI_PASS,strlen(EXAMPLE_ESP_WIFI_SSID),0,WIFI_AUTH_WPA_WPA2_PSK,0,2,100};
    wifi_config_t wifi_config = {acfg};
	
 
    esp_wifi_set_config(WIFI_IF_AP, &wifi_config);
 
    esp_wifi_start();
    ESP_LOGI(TAG, "wifi_init_softap finished.SSID:%s password:%s",
             EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);
}

void ethernet_init(){    //configure ethernet ... done currently by using the DHCP service
    ESP_ERROR_CHECK(esp_event_loop_init(eth_event_handler, NULL));
    eth_config_t config = DEFAULT_ETHERNET_PHY_CONFIG;   //congigure ethernet driver
    config.phy_addr = static_cast<eth_phy_base_t>(0);
    config.gpio_config = eth_gpio_config_rmii;
    config.tcpip_input = tcpip_adapter_eth_input;
    config.clock_mode = static_cast<eth_clock_mode_t>(0);   //gpio 0 clock in
    config.phy_power_enable = phy_device_power_enable_via_gpio; //method for enabling power on the clock

    
    ESP_ERROR_CHECK(esp_eth_init(&config));
    ESP_ERROR_CHECK(esp_eth_enable()) ;     // e
}
static esp_err_t eth_event_handler(void *ctx, system_event_t *event)    //event handler (only for debugging)
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
    case SYSTEM_EVENT_AP_STACONNECTED:
        // ESP_LOGI(TAG, "station:"MACSTR" join, AID=%d",
        //          MAC2STR(event->event_info.sta_connected.mac),
        //         event->event_info.sta_connected.aid);
                 ESP_LOGI(TAG, "wifi connected");
        break;
    case SYSTEM_EVENT_AP_STADISCONNECTED:
        //ESP_LOGI(TAG, "station:"MACSTR"leave, AID=%d",
        //         MAC2STR(event->event_info.sta_disconnected.mac),
         //        event->event_info.sta_disconnected.aid);
         ESP_LOGI(TAG, "wifi disconnected");
        break;
    default:
        break;
    }
    return ESP_OK;
}