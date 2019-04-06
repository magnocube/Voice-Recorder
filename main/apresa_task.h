#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>

#define HOST_IP_ADDR "192.168.2.32" //must be placed in a variable from settings.txt
#define PORT 2016					 // ""


static const char *payload = "Message from ESP32 ";

void apresa_task(esp_shared_buffer *shared_buffer){  
	Apresa* apresa = shared_buffer->apresaConnection;
	char rx_buffer[128];
    char addr_str[128];
    int addr_family;
    int ip_protocol;

    while (1) {
		while (1) {
			vTaskDelay(4000/portTICK_PERIOD_MS);


			struct sockaddr_in destAddr;
			destAddr.sin_addr.s_addr = inet_addr(HOST_IP_ADDR);
			destAddr.sin_family = AF_INET;
			destAddr.sin_port = htons(PORT);
			addr_family = AF_INET;
			ip_protocol = IPPROTO_IP;
			inet_ntoa_r(destAddr.sin_addr, addr_str, sizeof(addr_str) - 1);

			int sock =  socket(addr_family, SOCK_STREAM, ip_protocol);
			if (sock < 0) {
				ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
				break;
			}
			ESP_LOGI(TAG, "Socket created");

			int err = connect(sock, (struct sockaddr *)&destAddr, sizeof(destAddr));
			if (err != 0) {
				ESP_LOGE(TAG, "Socket unable to connect: errno %d", errno);
				break;
			}
			ESP_LOGI(TAG, "Successfully connected");

			while (1) {
				int err = send(sock, payload, strlen(payload), 0);
				if (err < 0) {
					ESP_LOGE(TAG, "Error occured during sending: errno %d", errno);
					break;
				}

				int len = recv(sock, rx_buffer, sizeof(rx_buffer) - 1, 0);
				// Error occured during receiving
				if (len < 0) {
					ESP_LOGE(TAG, "recv failed: errno %d", errno);
					break;
				}
				// Data received
				else {
					rx_buffer[len] = 0; // Null-terminate whatever we received and treat like a string
					ESP_LOGI(TAG, "Received %d bytes from %s:", len, addr_str);
					ESP_LOGI(TAG, "%s", rx_buffer);
				}

				vTaskDelay(2000 / portTICK_PERIOD_MS);
			}

			if (sock != -1) {
				ESP_LOGE(TAG, "Shutting down socket and restarting...");
				shutdown(sock, 0);
				close(sock);
			}
		}
    }
}
