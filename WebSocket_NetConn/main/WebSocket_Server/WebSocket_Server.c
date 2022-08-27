
#include "WebSocket_Server.h"

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include <string.h>
#include "driver/ledc.h"


static SemaphoreHandle_t xwebsocket_mutex; // to lock the client array
static QueueHandle_t xwebsocket_queue; // to hold the clients that send messages
static ws_client_t clients[WEBSOCKET_SERVER_MAX_CLIENTS]; // holds list of clients
static TaskHandle_t xtask; // the task itself

static void background_callback(struct netconn* conn, enum netconn_evt evt, u16_t len) {
	switch(evt) {
		case NETCONN_EVT_RCVPLUS:
			xQueueSendToBack(xwebsocket_queue, &conn, WEBSOCKET_SERVER_QUEUE_TIMEOUT);
		break;
		default:
		break;
	}
}

static void handle_read(uint8_t num) {
	ws_header_t header;
	char* msg;

	header.received = 0;
	msg = ws_read(&clients[num], &header);
	if(!header.received) {
		if(msg) free(msg);
		return;
	}
	switch(clients[num].last_opcode) {
		case WEBSOCKET_OPCODE_CONT:
		break;
		case WEBSOCKET_OPCODE_BIN:
			clients[num].scallback(num, WEBSOCKET_BIN, msg, header.length);
		break;
		case WEBSOCKET_OPCODE_TEXT:
			clients[num].scallback(num, WEBSOCKET_TEXT, msg, header.length);
		break;
		case WEBSOCKET_OPCODE_PING:
			ws_send(&clients[num], WEBSOCKET_OPCODE_PONG, msg, header.length, 0);
			clients[num].scallback(num, WEBSOCKET_PING, msg, header.length);
		break;
		case WEBSOCKET_OPCODE_PONG:
			if(clients[num].ping) {
				clients[num].scallback(num, WEBSOCKET_PONG, NULL, 0);
				clients[num].ping = 0;
			}
		break;
		case WEBSOCKET_OPCODE_CLOSE:
			clients[num].scallback(num, WEBSOCKET_DISCONNECT_EXTERNAL, NULL, 0);
			ws_disconnect_client(&clients[num], 0);
		break;
		default:
		break;
	}
	if(msg) free(msg);
}

static void ws_server_task(void* pvParameters) {
	struct netconn* conn;
	xwebsocket_mutex = xSemaphoreCreateMutex();
	xwebsocket_queue = xQueueCreate(WEBSOCKET_SERVER_QUEUE_SIZE, sizeof(struct netconn*));
	// Khởi tạo các client.
	for(int i=0; i<WEBSOCKET_SERVER_MAX_CLIENTS; i++) {
		clients[i].conn = NULL;
		clients[i].url  = NULL;
		clients[i].ping = 0;
		clients[i].last_opcode = 0;
		clients[i].contin = NULL;
		clients[i].len = 0;
		clients[i].ccallback = NULL;
		clients[i].scallback = NULL;
	}
	while(1) {
		xQueueReceive(xwebsocket_queue, &conn, portMAX_DELAY); // Chờ nhận queue.
		if(!conn) continue;

		xSemaphoreTake(xwebsocket_mutex, portMAX_DELAY); // take access
		for(int i=0; i<WEBSOCKET_SERVER_MAX_CLIENTS; i++) {
			if(clients[i].conn == conn) {
				handle_read(i);
				break;
			}
		}
		ESP_LOGE("ws_server_start", "Running");
		xSemaphoreGive(xwebsocket_mutex); // return access
	}
	vTaskDelete(NULL);
}

int ws_server_start(void) {
	if(xtask) return 0;
	xTaskCreate(&ws_server_task, "ws_server_task", WEBSOCKET_SERVER_TASK_STACK_DEPTH, NULL, WEBSOCKET_SERVER_TASK_PRIORITY, &xtask);
	return 1;
}

int ws_server_stop(void) {
	if(!xtask) return 0;
	vTaskDelete(xtask);
	return 1;
}

static bool prepare_response(char* buf, uint32_t buflen, char* handshake, char* protocol) {
/*	Ví dụ Data phản hồi từ client về.
 *  GET /ws HTTP/1.1
 *	Host: 192.168.1.9
 *	Connection: Upgrade
 *	Pragma: no-cache
 *	Cache-Control: no-cache
 *	User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/103.0.0.0 Safari/537.36
 *	Upgrade: websocket
 *	Origin: http://192.168.1.9
 *	Sec-WebSocket-Version: 13
 *	Accept-Encoding: gzip, deflate
 *	Accept-Language: en-US,en;q=0.9,vi;q=0.8
 *	Sec-WebSocket-Key: GaHnSIK+9pRVCqmDcSOfNw==
 *	Sec-WebSocket-Extensions: permessage-deflate; client_max_window_bits
*/
	const char WS_HEADER[] = "Upgrade: websocket\r\n";
	// Header "Upgrade: websocket" xuất hiện khi 1 client có yêu cầu đến server sử dụng WebSocket.
	const char WS_KEY[] =    "Sec-WebSocket-Key: ";
	/* Tiêu đề Sec-WebSocket-Accept được sử dụng trong quá trình bắt tay mở websocket. Nó sẽ xuất hiện trong
	 * tiêu đề phản hồi. Có nghĩa là, đây là tiêu đề được gửi từ máy chủ đến máy khách để thông báo rằng máy chủ
	 * sẵn sàng bắt đầu kết nối websocket.
	*/
	const char WS_RSP[] = "HTTP/1.1 101 Switching Protocols\r\n" \
						  "Upgrade: websocket\r\n" \
						  "Connection: Upgrade\r\n" \
						  "Sec-WebSocket-Accept: %s\r\n" \
						  "%s\r\n";
	char* key_start;
	char* key_end;
	char* hashed_key;
	// Đoạn if này để tìm WebSocket key.
	if(!strstr(buf, WS_HEADER)) return 0; // Data nhận về phải có header "Upgrade: websocket" (có yêu cầu websocket).
	if(!buflen) return 0;
	key_start = strstr(buf, WS_KEY);      // Tìm vị trí xuất hiên WebSocket key, trả về chuỗi bắt đầu từ chỗ có key.
	if(!key_start) return 0;
	key_start += 19;                      // + 19 vì 19 là độ dài của chuỗi "Sec-WebSocket-Key: ".
	key_end = strstr(key_start, "\r\n");  // Tìm vị trí cuối của Sec-WebSocket-Key.
	if(!key_end) return 0;

	hashed_key = ws_hash_handshake(key_start, key_end - key_start); // Tạo mã băm cho "Sec-WebSocket-Accept:".
	if(!hashed_key) return 0;
	if(protocol) {
		char tmp[256];
		sprintf(tmp, WS_RSP, hashed_key, "Sec-WebSocket-Protocol: %s\r\n");
		sprintf(handshake, tmp, protocol);
	}
	else {
		sprintf(handshake, WS_RSP, hashed_key, "");
	}
	free(hashed_key);
	return 1;
}

int ws_server_add_client_protocol(struct netconn* conn, char* msg, uint16_t len, char* url, char* protocol,
                         	 	 void (*callback)(uint8_t num, WEBSOCKET_TYPE_t type, char* msg, uint64_t len)) {
	int ret;
	char handshake[256];

	if(!prepare_response(msg, len, handshake, protocol)) { // Tạo phản hồi cho client.
		netconn_close(conn);  // Đóng kết nối.
		netconn_delete(conn); // Xóa kết nối.
		return -2;
	}
	ret = -1;
	xSemaphoreTake(xwebsocket_mutex, portMAX_DELAY);    // Chờ lấy mutex.
	conn->callback = background_callback;				// Đặt hàm gọi lại khi có sự kiện cho kết nối.
	netconn_write(conn, handshake, strlen(handshake), NETCONN_COPY); // Gửi phản hồi đến client.

	for(int i=0; i<WEBSOCKET_SERVER_MAX_CLIENTS; i++) { // Duyệt qua tất cả các client đang kết nối đến server.
		if(clients[i].conn) continue;                   // Nếu client[i] đang kết nối thì bỏ qua, kiểm tra client tiếp theo.
		clients[i] = ws_connect_client(conn, url, NULL, callback); // Nếu ddã đóng kết nối thì kết nối lại
		callback(i, WEBSOCKET_CONNECT, NULL, 0);		// Thực hiện kết nối lại.
		if(!ws_is_connected(clients[i])) {				// Nếu ko kết nối lại được.
			callback(i, WEBSOCKET_DISCONNECT_ERROR, NULL, 0);   // Ngắt kết nối do lỗi.
			ws_disconnect_client(&clients[i], 0);				// Ngắt kết nối do lỗi.
			break;
		}
		ret = i;
		break;
	}
	xSemaphoreGive(xwebsocket_mutex); // Nhả mutex cho thằng khác sài.
	return ret;
}

int ws_server_len_url(char* url) {
	int ret;
	ret = 0;
	xSemaphoreTake(xwebsocket_mutex,portMAX_DELAY);
	for(int i=0;i<WEBSOCKET_SERVER_MAX_CLIENTS;i++) {
		if(clients[i].url && strcmp(url,clients[i].url)) ret++;
	}
	xSemaphoreGive(xwebsocket_mutex);
	return ret;
}

int ws_server_add_client(struct netconn* conn, char* msg, uint16_t len, char* url,
						void (*callback)(uint8_t num, WEBSOCKET_TYPE_t type, char* msg, uint64_t len)) {
	return ws_server_add_client_protocol(conn, msg, len, url, NULL, callback);
}

int ws_server_len_all(void) {
	int ret;
	ret = 0;
	xSemaphoreTake(xwebsocket_mutex,portMAX_DELAY);
	for(int i=0;i<WEBSOCKET_SERVER_MAX_CLIENTS;i++) {
		if(clients[i].conn) ret++;
	}
	xSemaphoreGive(xwebsocket_mutex);
	return ret;
}

int ws_server_remove_client(int num) {
	int ret = 0;
	xSemaphoreTake(xwebsocket_mutex,portMAX_DELAY);
	if(ws_is_connected(clients[num])) {
		clients[num].scallback(num,WEBSOCKET_DISCONNECT_INTERNAL,NULL,0);
		ws_disconnect_client(&clients[num], 0);
		ret = 1;
	}
	xSemaphoreGive(xwebsocket_mutex);
	return ret;
}

int ws_server_remove_clients(char* url) {
	int ret = 0;
	xSemaphoreTake(xwebsocket_mutex,portMAX_DELAY);
	for(int i=0;i<WEBSOCKET_SERVER_MAX_CLIENTS;i++) {
		if(ws_is_connected(clients[i]) && strcmp(url,clients[i].url)) {
		    clients[i].scallback(i,WEBSOCKET_DISCONNECT_INTERNAL,NULL,0);
		    ws_disconnect_client(&clients[i], 0);
		    ret += 1;
		}
	}
	xSemaphoreGive(xwebsocket_mutex);
	return ret;
}

int ws_server_remove_all() {
	int ret = 0;
	xSemaphoreTake(xwebsocket_mutex,portMAX_DELAY);
	for(int i=0;i<WEBSOCKET_SERVER_MAX_CLIENTS;i++) {
		if(ws_is_connected(clients[i])) {
			clients[i].scallback(i,WEBSOCKET_DISCONNECT_INTERNAL,NULL,0);
			ws_disconnect_client(&clients[i], 0);
			ret += 1;
		}
	}
	xSemaphoreGive(xwebsocket_mutex);
	return ret;
}

// The following functions are already written below, but without the mutex.
int ws_server_send_text_client(int num,char* msg,uint64_t len) {
	xSemaphoreTake(xwebsocket_mutex,portMAX_DELAY);
	int ret = ws_server_send_text_client_from_callback(num, msg, len);
	xSemaphoreGive(xwebsocket_mutex);
	return ret;
}

int ws_server_send_text_clients(char* url,char* msg,uint64_t len) {
	xSemaphoreTake(xwebsocket_mutex,portMAX_DELAY);
	int ret = ws_server_send_text_clients_from_callback(url, msg, len);
	xSemaphoreGive(xwebsocket_mutex);
	return ret;
}

int ws_server_send_text_all(char* msg,uint64_t len) {
	xSemaphoreTake(xwebsocket_mutex,portMAX_DELAY);
	int ret = ws_server_send_text_all_from_callback(msg, len);
	xSemaphoreGive(xwebsocket_mutex);
	return ret;
}

// the following functions should be used inside of the callback. The regular versions
// grab the mutex, but it is already grabbed from inside the callback so it will hang.

int ws_server_send_text_client_from_callback(int num,char* msg,uint64_t len) {
	int ret = 0;
	int err;
	if(ws_is_connected(clients[num])) {
	err = ws_send(&clients[num],WEBSOCKET_OPCODE_TEXT,msg,len,0);
	ret = 1;
		if(err) {
			clients[num].scallback(num,WEBSOCKET_DISCONNECT_ERROR,NULL,0);
			ws_disconnect_client(&clients[num], 0);
			ret = 0;
		}
	}
	return ret;
}

int ws_server_send_text_clients_from_callback(char* url,char* msg,uint64_t len) {
	int ret = 0;
	int err;

	if(url == NULL) {
		return ret;
	}

	for(int i=0;i<WEBSOCKET_SERVER_MAX_CLIENTS;i++) {
		if(clients[i].url != NULL && ws_is_connected(clients[i]) && !strcmp(clients[i].url,url)) {
			err = ws_send(&clients[i],WEBSOCKET_OPCODE_TEXT,msg,len,0);
			if(!err) ret += 1;
			else {
				clients[i].scallback(i,WEBSOCKET_DISCONNECT_ERROR,NULL,0);
				ws_disconnect_client(&clients[i], 0);
			}
		}
	}
	return ret;
}

int ws_server_send_text_all_from_callback(char* msg,uint64_t len) {
	int ret = 0;
	int err;
	for(int i=0;i<WEBSOCKET_SERVER_MAX_CLIENTS;i++) {
		if(ws_is_connected(clients[i])) {
			err = ws_send(&clients[i],WEBSOCKET_OPCODE_TEXT,msg,len,0);
			if(!err) ret += 1;
			else {
				clients[i].scallback(i,WEBSOCKET_DISCONNECT_ERROR,NULL,0);
				ws_disconnect_client(&clients[i], 0);
			}
		}
	}
	return ret;
}


