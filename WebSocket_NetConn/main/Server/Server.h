

#ifndef SERVER_H_
#define SERVER_H_

#include <esp_system.h>
#include <esp_event.h>
#include "WebSocket.h"

//esp_err_t event_handler(void* ctx, system_event_t* event);
// void wifi_setup();
void websocket_callback(uint8_t num, WEBSOCKET_TYPE_t type,char* msg,uint64_t len);
void http_server(struct netconn *conn);
void server_task(void* pvParameters);
void server_handle_task(void* pvParameters);
#endif /* SERVER_H_ */
