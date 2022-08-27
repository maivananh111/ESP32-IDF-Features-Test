
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "stdio.h"
#include "esp_system.h"
#include "esp_err.h"
#include "esp_wifi.h"
#include "esp_netif.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "string.h"
#include "lwip/api.h"

#include "WIFI.h"
#include "Server.h"
#include "WebSocket.h"
#include "WebSocket_Server.h"

//static const char *TAG = "WebSocket Server";

char *WF_SSID = "Nhat Nam";
char *WF_PASS = "0989339608";
char *AP_SSID = "ESP32XXX";
char *AP_PASS = "159852dcm";
char *IP      = "192.168.1.5";
char *GATEWAY = "192.168.4.1";
char *NETMASK = "255.255.255.0";
char *MAINDNS = "0.0.0.0";
char *BACKDNS = "0.0.0.0";


void app_main(void){
	esp_err_t ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
		ESP_ERROR_CHECK(nvs_flash_erase());
		ret = nvs_flash_init();
	}
	ESP_ERROR_CHECK(ret);
	ESP_ERROR_CHECK(esp_netif_init());
	ESP_ERROR_CHECK(esp_event_loop_create_default());
//    tcpip_adapter_init();

	esp_netif_t *STA = WiFi_STA_Set_IP_Netif(IP, GATEWAY, NETMASK);
	WiFi_Station_Init(WF_SSID, WF_PASS);

	ESP_LOGW("LOCAL IP", "%s", LocalIP(STA));
	uint8_t Scan_Num = ScanWiFi();
	ESP_LOGI("SCAN WIFI", "Total access point finded: %d", Scan_Num);
	for(uint8_t i=0; i< Scan_Num; i++){
		ESP_LOGW("FIND SSID", "%s", Scan_Get_SSID(i));
	}
	/* NGUYÊN LÝ CHẠY CODE:
	 * 1. Khởi tạo wifi stastion, scan wifi, in ra IP của ESP.
	 * 2. Chờ có client truy cập vào IP của esp, chạy task "server_task" để thực hiện tạo kết nối,
	 * 	  ngồi chờ client gửi yêu cầu WebSocket.
	 * 3. Thực hiện task "server_handle_task" khi có yêu cầu nhận được từ client.
	 * 4. Khi có sự kiện nhận data từ client, thực hiện task trong hàm "ws_server_start".
	 * server_task và server_handle_task chỉ được chạy khi có 1 client mới thiết lập kết nối với server.
	 * ws_server_start() được thực hiện trong quá trình truyền nhận fullduplex giữa server với client.
	 */

    ws_server_start();
    xTaskCreate(&server_task, "server_task", 3000, NULL, 9, NULL);
    xTaskCreate(&server_handle_task, "server_handle_task", 4000, NULL, 6, NULL);
}


















