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

#include <sys/param.h>
#include <esp_http_server.h> 
#include "webInterface.h"   //code that will handle the web interface



static void phy_device_power_enable_via_gpio(bool enable);				 //function prototype   -> gets automaticly called from ehternet driver, will allow clock to flow to gpio0 (by default it is blocked)
static void eth_gpio_config_rmii(void); 								 //function prototype   -> gets automaticly called from ehternet driver
static esp_err_t eth_event_handler(void *ctx, system_event_t *event);    //function prototype   -> event handler. usefull for debugging. can be commented out when done testing
void wifi_init_softap(esp_shared_buffer *shared_buffer);                                                 //function prototype   -> enable the wifi
void ethernet_init();                                                    //function prototype   -> enable Ethernet

static esp_err_t event_handler(void *ctx, system_event_t *event);        // not used yet

static EventGroupHandle_t s_wifi_event_group;

/*
this task will setup the wifi and handle the webserver. 
It will also handle the communication to Apresa.
*/
void Wifi_ethernet_interface_task(esp_shared_buffer *shared_buffer){   
    //strcpy(sb.session_data->Ethernet_IP_Adress,"NO ADRESS");
    sb.session_data->Ethernet_Ip_received = false;

    tcpip_adapter_init();                                                       
    ethernet_init();
    wifi_init_softap(shared_buffer);
    // server starts when IP is found... search for event handler wifi
    vTaskDelay(2000/portTICK_PERIOD_MS); 
    setupWebserver();                           //normal we should wait for an interface to be connected... but the delay on the previous line works fine too!
                                                //(ethernet might nog be connected, but wifi AP sure is after 2 seconds)
    

    // shared_buffer->recording = true;
    // vTaskDelay(60000/portTICK_PERIOD_MS);
    // shared_buffer->recording = false;
	while(1){
        // while(1){
        //     vTaskDelay(1000/portTICK_PERIOD_MS);
        // }

			if(!shared_buffer->SD->isMounted()){	        // sd card is not in slot... isMounted function takes care of the SD card,, just call it reguarly
				shared_buffer->recording = false;
                
                /*may the odds be ever in your favor*/
			} else {						            	//sd card is in slot
				//do nothing... all the leds should already be in the right position... this function can be used to blink the leds?
			}
			vTaskDelay(250/portTICK_PERIOD_MS);  //dont make delay too big,, otherwise led's will respond late

            if(!shared_buffer->session_data->is_in_TestModus){              //as long as the device is not in test-mode, spam the console with the free memory
                ESP_LOGI(TAG, "Heapsize: %d, minimumheapsize: %d",esp_get_free_heap_size(),esp_get_minimum_free_heap_size());  //print free memory. to find leaks early
            }
            

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

    vTaskDelay(10/portTICK_PERIOD_MS); // Allow the power up/down to take effect, min 300us

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
                    /*horrible quick fix, but this seens to improve the wifi connection*/
                  ip_addr_t ip_Addr;
                    IP_ADDR4( &ip_Addr, 0,0,0,0 );
                    dns_gethostbyname("example.com", &ip_Addr, NULL, NULL );
                    vTaskDelay(1000/ portTICK_PERIOD_MS);
                    dns_gethostbyname("example.com", &ip_Addr, NULL, NULL );
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
void wifi_init_softap(esp_shared_buffer *shared_buffer)
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


    
    //wifi_ap_config_t acfg = {EXAMPLE_ESP_WIFI_SSID,EXAMPLE_ESP_WIFI_PASS,strlen(EXAMPLE_ESP_WIFI_SSID),0,WIFI_AUTH_WPA_WPA2_PSK,0,2,100};
    wifi_ap_config_t acfg;
    
    memcpy(acfg.ssid,EXAMPLE_ESP_WIFI_SSID,strlen(EXAMPLE_ESP_WIFI_SSID));
    //acfg.ssid = EXAMPLE_ESP_WIFI_SSID;                   /**< SSID of ESP32 soft-AP. If ssid_len field is 0, this must be a Null terminated string. Otherwise, length is set according to ssid_len. */
    memcpy(acfg.password,shared_buffer->session_data->macAdressString,63);
    //acfg.password = wifiDirtyFix;                        /**< Password of ESP32 soft-AP. Null terminated string. */
    acfg.ssid_len = strlen(EXAMPLE_ESP_WIFI_SSID);       /**< Optional length of SSID field. */
    acfg.channel = 0;                                    /**< Channel of ESP32 soft-AP */
    acfg.authmode = WIFI_AUTH_WPA_WPA2_PSK;              /**< Auth mode of ESP32 soft-AP. Do not support AUTH_WEP in soft-AP mode */
    acfg.ssid_hidden = 0;                                /**< Broadcast SSID or not, default 0, broadcast the SSID */
    acfg.max_connection = 2;                             /**< Max number of stations allowed to connect in, default 4, max 4 */
    acfg.beacon_interval = 100;                          /**< Beacon interval, 100 ~ 60000 ms, default 100 ms */



    wifi_config_t wifi_config = {acfg};
	
 
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    esp_wifi_set_ps(WIFI_PS_NONE);
    ESP_LOGI(TAG, "wifi_init_softap finished.SSID:%s password:%s",
             EXAMPLE_ESP_WIFI_SSID, shared_buffer->session_data->macAdressString);
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
        printf("Ethernet Link Up\n");
        /*NOTE:,, enabling the ethernet led might need to moced to event: ETH_GOT_IP, in stead of ETH_CONNECTED*/
		sb.gpio_header->digitalWrite(sb.pin_config->led_blue,PCA_HIGH,true);
        sb.gpio_header->digitalWrite(sb.pin_config->ethernet_up_led,PCA_LOW,true);
        sb.apresaConnection->startUpdateApresa(); // update the aprease when connected with ethernet
        break;
    case SYSTEM_EVENT_ETH_DISCONNECTED:
        ESP_LOGI(TAG, "Ethernet Link Down");
		sb.gpio_header->digitalWrite(sb.pin_config->led_blue,PCA_LOW,false);
        sb.gpio_header->digitalWrite(sb.pin_config->ethernet_up_led,PCA_HIGH,true);
        strcpy(sb.session_data->Ethernet_IP_Adress,"NO ADRESS");
        sb.session_data->Ethernet_Ip_received = false;
        break;
    case SYSTEM_EVENT_ETH_START:
        ESP_LOGI(TAG, "Ethernet Started");
        break;
    case SYSTEM_EVENT_ETH_GOT_IP:
        memset(&ip, 0, sizeof(tcpip_adapter_ip_info_t));
        ESP_ERROR_CHECK(tcpip_adapter_get_ip_info(static_cast<tcpip_adapter_if_t>(ESP_IF_ETH), &ip));
        ESP_LOGD(TAG, "Ethernet Got IP Addr");
        ESP_LOGD(TAG, "~~~~~~~~~~~");
        ESP_LOGD(TAG, "ETHIP:" IPSTR, IP2STR(&ip.ip));
        ESP_LOGD(TAG, "ETHMASK:" IPSTR, IP2STR(&ip.netmask));
        ESP_LOGD(TAG, "ETHGW:" IPSTR, IP2STR(&ip.gw));
        ESP_LOGD(TAG, "~~~~~~~~~~~");
        

        sb.session_data->Ethernet_IP_Adress = inet_ntoa(ip.ip);
        sb.session_data->Ethernet_Ip_received = true;
        
        //printf(sb.session_data->Ethernet_IP_Adress);
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