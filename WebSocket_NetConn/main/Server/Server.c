
#include "Server.h"

#include "WebSocket_Server.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "lwip/api.h"

#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_event.h"
#include "driver/ledc.h"

#include "string.h"


#define LED_PIN 18
#define AP_SSID "ESP32 Web"
#define AP_PSSWD "159852dm"

QueueHandle_t client_queue;
static const int client_queue_size = 10;
/*
// handles WiFi events
esp_err_t event_handler(void* ctx, system_event_t* event) {
	const char* TAG = "event_handler";
	switch(event -> event_id) {
		case SYSTEM_EVENT_AP_START:
			//ESP_ERROR_CHECK(tcpip_adapter_set_hostname(TCPIP_ADAPTER_IF_STA, "esp32"));
			ESP_LOGI(TAG, "Access Point Started");
		break;
		case SYSTEM_EVENT_AP_STOP:
			ESP_LOGI(TAG, "Access Point Stopped");
		break;
		case SYSTEM_EVENT_AP_STACONNECTED:
			ESP_LOGI(TAG, "STA Connected, MAC=%02x:%02x:%02x:%02x:%02x:%02x AID=%i",
			event->event_info.sta_connected.mac[0],event->event_info.sta_connected.mac[1],
			event->event_info.sta_connected.mac[2],event->event_info.sta_connected.mac[3],
			event->event_info.sta_connected.mac[4],event->event_info.sta_connected.mac[5],
			event->event_info.sta_connected.aid);
		break;
		case SYSTEM_EVENT_AP_STADISCONNECTED:
			ESP_LOGI(TAG, "STA Disconnected, MAC=%02x:%02x:%02x:%02x:%02x:%02x AID=%i",
			event->event_info.sta_disconnected.mac[0],event->event_info.sta_disconnected.mac[1],
			event->event_info.sta_disconnected.mac[2],event->event_info.sta_disconnected.mac[3],
			event->event_info.sta_disconnected.mac[4],event->event_info.sta_disconnected.mac[5],
			event->event_info.sta_disconnected.aid);
		break;
		case SYSTEM_EVENT_AP_PROBEREQRECVED:
			ESP_LOGI(TAG, "AP Probe Received");
		break;
		case SYSTEM_EVENT_AP_STA_GOT_IP6:
			ESP_LOGI(TAG, "Got IP6=%01x:%01x:%01x:%01x",
			event->event_info.got_ip6.ip6_info.ip.addr[0],event->event_info.got_ip6.ip6_info.ip.addr[1],
			event->event_info.got_ip6.ip6_info.ip.addr[2],event->event_info.got_ip6.ip6_info.ip.addr[3]);
		break;
		default:
			ESP_LOGI(TAG, "Unregistered event=%i",event->event_id);
		break;
	}
	return ESP_OK;
}

// sets up WiFi
void wifi_setup() {
	const char* TAG = "wifi_setup";

	ESP_LOGI(TAG,"starting tcpip adapter");

	ESP_ERROR_CHECK(tcpip_adapter_dhcps_stop(TCPIP_ADAPTER_IF_AP));
	//tcpip_adapter_set_hostname(TCPIP_ADAPTER_IF_AP,"esp32");
	tcpip_adapter_ip_info_t info;
	memset(&info, 0, sizeof(info));
	IP4_ADDR(&info.ip, 192, 168, 4, 1);
	IP4_ADDR(&info.gw, 192, 168, 4, 1);
	IP4_ADDR(&info.netmask, 255, 255, 255, 0);
	ESP_LOGI(TAG,"setting gateway IP");
	ESP_ERROR_CHECK(tcpip_adapter_set_ip_info(TCPIP_ADAPTER_IF_AP, &info));
	//ESP_ERROR_CHECK(tcpip_adapter_set_hostname(TCPIP_ADAPTER_IF_AP,"esp32"));
	//ESP_LOGI(TAG,"set hostname to \"%s\"",hostname);
	ESP_LOGI(TAG,"starting DHCPS adapter");
	ESP_ERROR_CHECK(tcpip_adapter_dhcps_start(TCPIP_ADAPTER_IF_AP));
	//ESP_ERROR_CHECK(tcpip_adapter_set_hostname(TCPIP_ADAPTER_IF_AP,hostname));
	ESP_LOGI(TAG,"starting event loop");
	ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL));

	ESP_LOGI(TAG,"initializing WiFi");
	wifi_init_config_t wifi_init_config = WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK(esp_wifi_init(&wifi_init_config));
	ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));

	wifi_config_t wifi_config = {
		.ap = {
			.ssid = AP_SSID,
			.password= AP_PSSWD,
			.channel = 0,
			.authmode = WIFI_AUTH_WPA2_PSK,
			.ssid_hidden = 0,
			.max_connection = 4,
			.beacon_interval = 100
		}
	};

	ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
	ESP_ERROR_CHECK(esp_wifi_start());
	ESP_LOGI(TAG,"WiFi set up");
}
*/

void websocket_callback(uint8_t num, WEBSOCKET_TYPE_t type, char* msg, uint64_t len) {
	const static char* TAG = "websocket_callback";

	switch(type) {
		case WEBSOCKET_CONNECT:
			ESP_LOGI(TAG,"Client %i connected!", num);
		break;
		case WEBSOCKET_DISCONNECT_EXTERNAL:
			ESP_LOGI(TAG,"Client %i sent a disconnect message", num);
		break;
		case WEBSOCKET_DISCONNECT_INTERNAL:
			ESP_LOGI(TAG,"Client %i was disconnected", num);
		break;
		case WEBSOCKET_DISCONNECT_ERROR:
			ESP_LOGI(TAG,"Client %i was disconnected due to an error", num);
		break;
		case WEBSOCKET_TEXT:
			if(len) { // if the message length was greater than zero
				ESP_LOGW(TAG, "Messenge from client: %s", msg);
//				ws_server_send_text_all_from_callback(msg, len); // broadcast it!
				if(!strcmp(msg, "toggle")){
					ws_server_send_text_all_from_callback(msg, len); // broadcast it!
				}
			}
		break;
		case WEBSOCKET_BIN:
			ESP_LOGI(TAG, "Client %i sent binary message of size %i:\n%s", num,(uint32_t)len, msg);
		break;
		case WEBSOCKET_PING:
			ESP_LOGI(TAG, "Client %i pinged us with message of size %i:\n%s", num,(uint32_t)len, msg);
		break;
		case WEBSOCKET_PONG:
			ESP_LOGI(TAG, "Client %i responded to the ping", num);
		break;
	}
}
// serves any clients
void http_server(struct netconn *conn) {
	const static char* TAG = "http_server";
	const static char HTML_HEADER[] = "HTTP/1.1 200 OK\nContent-type: text/html\n\n"; // Nội dung gói tin http dạng text/html.

	const static char JS_HEADER[] = "HTTP/1.1 200 OK\nContent-type: text/javascript\n\n"; // Nội dung gói tin http dạng text/javascript.
	const static char CSS_HEADER[] = "HTTP/1.1 200 OK\nContent-type: text/css\n\n"; // Nội dung gói tin http dạng text/css.
	struct netbuf* inbuf;
	static char* buf;
	static uint16_t buflen;
	static err_t err;

	// default page
	extern const uint8_t root_html_start[] asm("_binary_root_html_start");
	extern const uint8_t root_html_end[] asm("_binary_root_html_end");
	const uint32_t root_html_len = root_html_end - root_html_start;

	// test.js
	extern const uint8_t test_js_start[] asm("_binary_test_js_start");
	extern const uint8_t test_js_end[] asm("_binary_test_js_end");
	const uint32_t test_js_len = test_js_end - test_js_start;

	// test.css
	extern const uint8_t test_css_start[] asm("_binary_test_css_start");
	extern const uint8_t test_css_end[] asm("_binary_test_css_end");
	const uint32_t test_css_len = test_css_end - test_css_start;

	netconn_set_recvtimeout(conn, 1000);  // Cho phép thời gian kết nối giới hạn dưới 1s.
	ESP_LOGI(TAG, "Reading from client...");
	err = netconn_recv(conn, &inbuf); 	// Chờ nhận dữ liệu, dữ liệu nhận về lưu vào inbuf.
	ESP_LOGI(TAG, "Readed from client");
	if(err == ERR_OK) {					// Nếu không có lỗi
		netbuf_data(inbuf, (void**)&buf, &buflen); // Lấy con trỏ chuỗi dữ liệu nhận được.
		ESP_LOGI(TAG, "Data: %s", buf);
		if(buf) {							// Nếu không phải NULL
			if(strstr(buf, "GET /") && !strstr(buf, "Upgrade: websocket")) { // Client yêu cầu GET và không upgrade lên giao thức websocket.
				ESP_LOGI(TAG, "Request from client not require websocket.");
				netconn_write(conn, HTML_HEADER, sizeof(HTML_HEADER)-1, NETCONN_NOCOPY);    // Gửi dữ liệu đến client.
				netconn_write(conn, root_html_start, root_html_len, NETCONN_NOCOPY);		// Gửi dữ liệu đến client.
				netconn_close(conn);  // Đóng kết nối.
				netconn_delete(conn); // Xóa kết nối.
				netbuf_delete(inbuf); // Giải phóng inbuf.
			}
			else if(strstr(buf, "GET /") && strstr(buf, "Upgrade: websocket")) { // Client yêu cầu GET và upgrade lên giao thức websocket.
				ESP_LOGI(TAG,"Requesting websocket on /");
				ws_server_add_client(conn, buf, buflen, "/", websocket_callback); // Thiết lập WebSocket.
				netbuf_delete(inbuf); // Giải phóng inbuf.
			}

			else if(strstr(buf,"GET /test.js ")) { // Yêu cầu lấy file js.
				ESP_LOGI(TAG, "Sending /test.js");
				netconn_write(conn, JS_HEADER, sizeof(JS_HEADER)-1, NETCONN_NOCOPY);
				netconn_write(conn, test_js_start, test_js_len, NETCONN_NOCOPY);
				netconn_close(conn);  // Đóng kết nối.
				netconn_delete(conn); // Xóa kết nối.
				netbuf_delete(inbuf); // Giải phóng inbuf.
			}

			else if(strstr(buf, "GET /test.css ")) { // Yêu cầu lấy file css.
				ESP_LOGI(TAG, "Sending /test.css");
				netconn_write(conn, CSS_HEADER, sizeof(CSS_HEADER)-1,NETCONN_NOCOPY);
				netconn_write(conn, test_css_start, test_css_len,NETCONN_NOCOPY);
				netconn_close(conn);  // Đóng kết nối.
				netconn_delete(conn); // Xóa kết nối.
				netbuf_delete(inbuf); // Giải phóng inbuf.
			}
			else { 									// Yêu cầu không xác định.
				ESP_LOGI(TAG, "Unknown request");
				netconn_close(conn);  // Đóng kết nối.
				netconn_delete(conn); // Xóa kết nối.
				netbuf_delete(inbuf); // Giải phóng inbuf.
			}
		}
		else {
			ESP_LOGI(TAG,"Unknown request (empty?...)");
			netconn_close(conn);  // Đóng kết nối.
			netconn_delete(conn); // Xóa kết nối.
			netbuf_delete(inbuf); // Giải phóng inbuf.
		}

	}
	else { // Nếu lỗi.
		ESP_LOGI(TAG, "Error on read, closing connection.");
		netconn_close(conn);  // Đóng kết nối.
		netconn_delete(conn); // Xóa kết nối.
		netbuf_delete(inbuf); // Giải phóng inbuf.
	}
}

// handles clients when they first connect. passes to a queue
void server_task(void* pvParameters) {
	const static char* TAG = "server_task";
	struct netconn *conn, *newconn;
	static err_t err;
	client_queue = xQueueCreate(client_queue_size, sizeof(struct netconn*)); // Tạo hàng đợi client.

	conn = netconn_new(NETCONN_TCP); // Tạo kết nối TCP.
	netconn_bind(conn, NULL, 80); 	 // Gắn kết nối ở cổng 80 trên bất kỳ IP cục bộ nào đang có.
	netconn_listen(conn); 			 // Đặt trạng thái kết nối thành LISTEN.
	ESP_LOGI(TAG, "server listening");
	do {
		err = netconn_accept(conn, &newconn); // Chặn mọi thứ cho đến khi có client kết nối đến.
		ESP_LOGI(TAG, "new client");
		if(err == ERR_OK) {
			xQueueSendToBack(client_queue, &newconn, portMAX_DELAY); // Nhả về cuối queue.
			ESP_LOGE("server_task", "Running");
		}
	} while(err == ERR_OK);
	netconn_close(conn);  // Đóng kết nối
	netconn_delete(conn); // Xóa kết nối
	ESP_LOGE(TAG, "Task ending, rebooting board");
	esp_restart();
}

// receives clients from queue, handles them
void server_handle_task(void* pvParameters) {
	const static char* TAG = "server_handle_task";
	struct netconn* conn;
	ESP_LOGI(TAG, "task starting");
	while(1) {
		xQueueReceive(client_queue, &conn, portMAX_DELAY); // Chờ có queue từ server_task.
		if(!conn) continue;
		http_server(conn); // Chạy http server.
		ESP_LOGE("server_handle_task", "Running");
	}
	vTaskDelete(NULL);
}
