
#include <esp_wifi.h>
#include <esp_event.h>
#include <esp_log.h>
#include <esp_system.h>
#include <nvs_flash.h>
#include "nvs_flash.h"
#include "esp_netif.h"
#include "WIFI.h"
#include "esp_http_server.h"
#include "driver/gpio.h"

#define MAX_SOCKET_CLIENTS 10
static const char *TAG = "WebSocket Server";
char *WF_SSID = "Nhat Nam";
char *WF_PASS = "0989339608";
char *IP      = "192.168.1.8";
char *GATEWAY = "192.168.4.1";
char *NETMASK = "255.255.255.0";


struct async_resp_arg {
    httpd_handle_t hd;
    int fd;
};
uint8_t led_state = 0;
httpd_handle_t server = NULL;
httpd_handle_t webserver = NULL;
int Num_CLient = 0;
int sockfd[MAX_SOCKET_CLIENTS];

static void ws_async_send(void *arg){
    static const char * data_on = "ON";
    static const char * data_off = "OFF";
    struct async_resp_arg *resp_arg = arg;
    httpd_handle_t hd = resp_arg->hd;
    httpd_ws_frame_t ws_pkt;
    memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));

    if(led_state == 0){
    	led_state = 1;
        ws_pkt.payload = (uint8_t*)data_on;
        ws_pkt.len = strlen(data_on);
    }
    else{
    	led_state = 0;
        ws_pkt.payload = (uint8_t*)data_off;
        ws_pkt.len = strlen(data_off);
    }
    ws_pkt.type = HTTPD_WS_TYPE_TEXT;
    for(int fd=0; fd<Num_CLient; fd++){
    	ESP_LOGI("Send messenge from sockfd:", "%d", sockfd[fd]);
    	httpd_ws_send_frame_async(hd, sockfd[fd], &ws_pkt);
    }
    free(resp_arg);
    gpio_set_level(GPIO_NUM_2, led_state);
}

static esp_err_t trigger_async_send(httpd_handle_t handle, httpd_req_t *req){
    struct async_resp_arg *resp_arg = malloc(sizeof(struct async_resp_arg));
    resp_arg->hd = req->handle;
    resp_arg->fd = httpd_req_to_sockfd(req);
    return httpd_queue_work(handle, ws_async_send, resp_arg);
}


static esp_err_t Web_handler(httpd_req_t *req){
	extern const uint8_t root_html_start[] asm("_binary_root_html_start");
	extern const uint8_t root_html_end[] asm("_binary_root_html_end");
	const uint32_t root_html_len = root_html_end - root_html_start;

	esp_err_t ret = httpd_resp_send(req, (char *)root_html_start, root_html_len);
	if(ret != ESP_OK) ESP_LOGE("WebSever", "Error send data html init web.");
	return ret;
}

static esp_err_t WebSocket_handler(httpd_req_t *req){
    if (req->method == HTTP_GET) { // Lần đầu, khởi tạo kết nối.
        ESP_LOGI(TAG, "Handshake done, the new connection was opened");
        int socket = httpd_req_to_sockfd(req); // Lấy số socket.

        for(int i=0; i<MAX_SOCKET_CLIENTS; i++){ // Kiểm tra xem có trùng với các socket đã mở hay không.
        	if(socket == sockfd[i]) return ESP_OK; // Nếu socket đã được mở thì thoát.
        }

		sockfd[Num_CLient] = socket; // Gán socket của client mới vào danh sách socket.
		ESP_LOGI("Client sockfd", "%d", sockfd[Num_CLient]);
		Num_CLient++; // Tăng số client.
		ESP_LOGI(TAG, "Number Client connected: %d", Num_CLient);

        return ESP_OK;
    }

    httpd_ws_frame_t ws_pkt;
    uint8_t *buf = NULL;
    memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
    ws_pkt.type = HTTPD_WS_TYPE_TEXT;
    esp_err_t ret = httpd_ws_recv_frame(req, &ws_pkt, 0);   // Lấy độ dài dữ liệu.
    if (ret != ESP_OK) { 									// Kiểm tra quá trình nhận.
        ESP_LOGE(TAG, "httpd_ws_recv_frame failed to get frame len with %d", ret);
        return ret;
    }
    ESP_LOGI(TAG, "frame len is %d", ws_pkt.len);
    if (ws_pkt.len) {
        buf = calloc(1, ws_pkt.len + 1); // Cấp phát bộ nhớ để lưu data từ client.
        if (buf == NULL) {
            ESP_LOGE(TAG, "Failed to calloc memory for buf");
            return ESP_ERR_NO_MEM;
        }
        ws_pkt.payload = buf; // Gán còn trỏ lưu dữ liệu bằng con trỏ bộ đệm.
        esp_err_t ret = httpd_ws_recv_frame(req, &ws_pkt, ws_pkt.len); // Lấy full data.
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "httpd_ws_recv_frame failed with %d", ret);
            free(buf);
            return ret;
        }
        ESP_LOGW(TAG, "Got packet with message: %s", ws_pkt.payload);
    }
    ESP_LOGI(TAG, "Packet type: %d", ws_pkt.type);
    if (ws_pkt.type == HTTPD_WS_TYPE_TEXT && strcmp((char*)ws_pkt.payload, "toggle") == 0) {
        free(buf);
        return trigger_async_send(req->handle, req);
    }

    ret = httpd_ws_send_frame(req, &ws_pkt);
    if (ret != ESP_OK) ESP_LOGE(TAG, "httpd_ws_send_frame failed with %d", ret);
    free(buf);
    return ret;
}
// Đăng ký 1 uri cho websocket được tự khởi tạo sau khi chạy html
static const httpd_uri_t ws = {
        .uri        = "/ws",
        .method     = HTTP_GET,
        .handler    = WebSocket_handler,
        .user_ctx   = NULL,
        .is_websocket = true
};
// Đăng ký 1 uri cho webserver load html khi truy cập vào URL (IP) của esp.
static const httpd_uri_t web = {
        .uri        = "/",
        .method     = HTTP_GET,
        .handler    = Web_handler,
        .user_ctx   = NULL,
};

static httpd_handle_t start_webserver(void){
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    ESP_LOGI(TAG, "Starting WebSocket Server on port: '%d'", config.server_port);
    if (httpd_start(&webserver, &config) == ESP_OK) {
        ESP_LOGI(TAG, "Registering URI handlers");
        httpd_register_uri_handler(webserver, &web);
        httpd_register_uri_handler(webserver, &ws);
        ESP_LOGI(TAG, "Registering URI handlers OKE");
        return webserver;
    }
    ESP_LOGI(TAG, "Error starting server!");
    return NULL;
}

static void stop_webserver(httpd_handle_t server){
    httpd_stop(server);
    ESP_LOGI(TAG, "WebSocket server disconnected!");
}

void Event_STA_Disconnect_Handler(void){
    if(server) {
        ESP_LOGI(TAG, "Stopping webserver");
        stop_webserver(server);
        server = NULL;
    }
}

void Event_STA_GotIP_Handler(void){
    if (server == NULL) {
        ESP_LOGI(TAG, "Starting webserver");
        server = start_webserver();

    }
}

void app_main(void){
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

	WiFi_STA_Set_IP_Netif(IP, GATEWAY, NETMASK);
	WiFi_Station_Init(WF_SSID, WF_PASS);

	uint8_t Scan_Num = ScanWiFi();
	ESP_LOGI("SCAN WIFI", "Total access point finded: %d", Scan_Num);
	for(uint8_t i=0; i< Scan_Num; i++) ESP_LOGW("FIND SSID", "%s", Scan_Get_SSID(i));

	gpio_set_direction(GPIO_NUM_2, GPIO_MODE_OUTPUT);

	gpio_set_level(GPIO_NUM_2, 1);
	vTaskDelay(200/portTICK_PERIOD_MS);
	gpio_set_level(GPIO_NUM_2, 0);
	vTaskDelay(200/portTICK_PERIOD_MS);
	gpio_set_level(GPIO_NUM_2, 1);
	vTaskDelay(200/portTICK_PERIOD_MS);
	gpio_set_level(GPIO_NUM_2, 0);
	vTaskDelay(200/portTICK_PERIOD_MS);
	gpio_set_level(GPIO_NUM_2, 1);
	vTaskDelay(200/portTICK_PERIOD_MS);
	gpio_set_level(GPIO_NUM_2, 0);

//    server = start_webserver();
}






