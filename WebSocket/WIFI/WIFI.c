#include <stdio.h>
#include "string.h"
#include "WIFI.h"
#include "esp_system.h"
#include "esp_err.h"
#include "esp_wifi.h"
#include "esp_netif.h"
#include "esp_log.h"
#include "freertos/event_groups.h"
#include "lwip/err.h"
#include "lwip/sys.h"

 /* STATION MODE */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

static EventGroupHandle_t WF_Event_GR;
static int reconnect_num = 0;
static const char *TAGWIFI_STA = "WIFI STATION INIT";

void WiFiStation_Connect_Event_Handle(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);
/* ACCESS POINT MODE */
static const char *TAGWIFI_AP = "WIFI ACCESS POINT INIT";

static void WiFiAccessPoint_Init_Event_Handle(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);
/* SCAN WIFI */
uint16_t number = 20;
wifi_ap_record_t ap_info[20];


/* SOURCE STATION */

__weak_symbol void Event_STA_GotIP_Handler(void){
	ESP_LOGW(TAGWIFI_STA, "Event station got ip hanlder not registered.");
}
__weak_symbol void Event_STA_Disconnect_Handler(void){
	ESP_LOGW(TAGWIFI_STA, "Event station disconnected hanlder not registered.");
}
void WiFiStation_Connect_Event_Handle(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
    	ESP_LOGI(TAGWIFI_STA, "WiFi connecting..........");
        esp_wifi_connect();
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
    	Event_STA_Disconnect_Handler();
    	ESP_LOGI(TAGWIFI_STA, "WiFi lost connection!");
        if (reconnect_num < 5) {
            esp_wifi_connect();
            reconnect_num++;
            ESP_LOGI(TAGWIFI_STA, "Retry to connect to the AP..........");
        }
        else {
            xEventGroupSetBits(WF_Event_GR, WIFI_FAIL_BIT);
        }
        ESP_LOGI(TAGWIFI_STA,"Connect to the AP fail!");
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_CONNECTED) {
    	ESP_LOGI(TAGWIFI_STA, "WiFi connected successfully!");
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAGWIFI_STA, "Got IP:" IPSTR, IP2STR(&event->ip_info.ip));
        Event_STA_GotIP_Handler();
        reconnect_num = 0;
        xEventGroupSetBits(WF_Event_GR, WIFI_CONNECTED_BIT);
    }
}

esp_netif_t *WiFi_Station_Init_Netif(char *SSID, char *PASS){
	ESP_LOGI(TAGWIFI_STA, "ESP_WIFI_MODE_STATION");
	WF_Event_GR = xEventGroupCreate();
	esp_netif_t *station = esp_netif_create_default_wifi_sta();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &WiFiStation_Connect_Event_Handle, NULL, &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &WiFiStation_Connect_Event_Handle, NULL, &instance_got_ip));

    wifi_config_t wifi_config = {
        .sta = {
			.threshold.authmode = WIFI_AUTH_WPA2_PSK,
            .pmf_cfg = {
                .capable = true,
                .required = false
            },
        },
    };
    memcpy(wifi_config.sta.ssid, SSID, strlen(SSID));
    memcpy(wifi_config.sta.password, PASS, strlen(PASS));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
    ESP_ERROR_CHECK(esp_wifi_start() );
    ESP_LOGI(TAGWIFI_STA, "wifi_init_sta finished.");

    EventBits_t bits = xEventGroupWaitBits(WF_Event_GR, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT, pdFALSE, pdFALSE, portMAX_DELAY);
    if (bits & WIFI_CONNECTED_BIT) {
    	ESP_LOGI(TAGWIFI_STA, "Connected to ap SSID: %s password: %s", wifi_config.sta.ssid, wifi_config.sta.password);
    } else if (bits & WIFI_FAIL_BIT) {
    	ESP_LOGI(TAGWIFI_STA, "Failed to connect to SSID: %s, password: %s", wifi_config.sta.ssid, wifi_config.sta.password);
    } else {
    	ESP_LOGI(TAGWIFI_STA, "UNEXPECTED EVENT");
    }
    return station;
}

void WiFi_Station_Init(char *SSID, char *PASS){
	ESP_LOGI(TAGWIFI_STA, "ESP_WIFI_MODE_STATION");
	WF_Event_GR = xEventGroupCreate();
//	esp_netif_t *station = esp_netif_create_default_wifi_sta();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &WiFiStation_Connect_Event_Handle, NULL, &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &WiFiStation_Connect_Event_Handle, NULL, &instance_got_ip));

    wifi_config_t wifi_config = {
        .sta = {
			.threshold.authmode = WIFI_AUTH_WPA2_PSK,
            .pmf_cfg = {
                .capable = true,
                .required = false
            },
        },
    };
    memcpy(wifi_config.sta.ssid, SSID, strlen(SSID));
    memcpy(wifi_config.sta.password, PASS, strlen(PASS));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
    ESP_ERROR_CHECK(esp_wifi_start() );
    ESP_LOGI(TAGWIFI_STA, "wifi_init_sta finished.");

    EventBits_t bits = xEventGroupWaitBits(WF_Event_GR, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT, pdFALSE, pdFALSE, portMAX_DELAY);
    if (bits & WIFI_CONNECTED_BIT) {
    	ESP_LOGI(TAGWIFI_STA, "Connected to Access point: %s.", wifi_config.sta.ssid);
    } else if (bits & WIFI_FAIL_BIT) {
    	ESP_LOGI(TAGWIFI_STA, "Failed to connect to Access point: %s", wifi_config.sta.ssid);
    } else {
    	ESP_LOGI(TAGWIFI_STA, "UNEXPECTED EVENT");
    }
//    return station;
}

/* SOURCE ACCESS POINT */
static void WiFiAccessPoint_Init_Event_Handle(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data){
    if (event_id == WIFI_EVENT_AP_STACONNECTED) {
        ip_event_got_ip_t* evt = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAGWIFI_AP, "got ip:" IPSTR, IP2STR(&evt->ip_info.ip));
        wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
        ESP_LOGI(TAGWIFI_AP, "Station "MACSTR" join, AID = %d", MAC2STR(event->mac), event->aid);
    } else if (event_id == WIFI_EVENT_AP_STADISCONNECTED) {
        wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
        ESP_LOGI(TAGWIFI_AP, "Station "MACSTR" leave, AID = %d", MAC2STR(event->mac), event->aid);
    }
}
esp_netif_t *WiFi_SoftAP_Init(char *SSID, char *PASS, uint8_t WiFi_Channel, uint8_t WiFi_Max_Connection){
	    esp_netif_t *softAP = esp_netif_create_default_wifi_ap();
	    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
	    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &WiFiAccessPoint_Init_Event_Handle, NULL, NULL));
	    wifi_config_t wifi_config = {
	        .ap = {
	            .ssid_len = strlen(SSID),
	            .channel = WiFi_Channel,
	            .max_connection = WiFi_Max_Connection,
	            .authmode = WIFI_AUTH_WPA_WPA2_PSK
	        },
	    };
	    memcpy(wifi_config.ap.ssid, SSID, strlen(SSID));
	    memcpy(wifi_config.ap.password, PASS, strlen(PASS));
	    if (strlen(PASS) == 0) {
	        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
	    }

	    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
	    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
	    ESP_ERROR_CHECK(esp_wifi_start());

	    ESP_LOGI(TAGWIFI_AP, "wifi_init_softap finished. SSID: %s PASSWORD: %s channel: %d", SSID, PASS, WiFi_Channel);
	    return softAP;
}

void WiFi_AP_Set_IP(esp_netif_t *WiFi_Netif, char *LocalIP, char *GateWay, char *NetMask){
    esp_netif_ip_info_t ip_info;
    memset(&ip_info, 0, sizeof(esp_netif_ip_info_t));
    if (WiFi_Netif){
        esp_netif_dhcps_stop(WiFi_Netif);
        ip_info.ip.addr = esp_ip4addr_aton((const char *)LocalIP);
        ip_info.netmask.addr = esp_ip4addr_aton((const char *)NetMask);
        ip_info.gw.addr = esp_ip4addr_aton((const char *)GateWay);
        esp_netif_set_ip_info(WiFi_Netif, &ip_info);
        esp_netif_dhcps_start(WiFi_Netif);
    }
}

void WiFi_STA_Set_IP(esp_netif_t *WiFi_Netif, char *LocalIP, char *GateWay, char *NetMask){
//void WiFi_STA_Set_IP(esp_netif_t *WiFi_Netif){
	esp_netif_ip_info_t ip_info = {0};
//	esp_netif_dns_info_t p_dns_info = {0};
//	esp_netif_dns_info_t s_dns_info = {0};
	if(WiFi_Netif){
		memset(&ip_info, 0, sizeof(esp_netif_ip_info_t));
		esp_netif_dhcpc_stop(WiFi_Netif);
        ip_info.ip.addr = esp_ip4addr_aton((const char *)LocalIP);
        ip_info.netmask.addr = esp_ip4addr_aton((const char *)NetMask);
        ip_info.gw.addr = esp_ip4addr_aton((const char *)GateWay);
	    if (esp_netif_set_ip_info(WiFi_Netif,&ip_info) != ESP_OK) {
	        ESP_LOGE("STATION SET IP", "Failed to set ip info");
	        return;
	    }
/*
		memset(&p_dns_info, 0, sizeof(esp_netif_dns_info_t));
		p_dns_info.ip.u_addr.ip4.addr = esp_ip4addr_aton((const char *)PrimaryDNS);
		esp_netif_set_dns_info(WiFi_Netif,ESP_NETIF_DNS_MAIN, &p_dns_info);
		memset(&s_dns_info, 0, sizeof(esp_netif_dns_info_t));
		s_dns_info.ip.u_addr.ip4.addr = esp_ip4addr_aton((const char *)SeconDNS);
		esp_netif_set_dns_info(WiFi_Netif,ESP_NETIF_DNS_BACKUP, &s_dns_info);
		*/
	}
}

esp_netif_t *WiFi_STA_Set_IP_Netif(char *LocalIP, char *GateWay, char *NetMask){
	esp_netif_t *station = esp_netif_create_default_wifi_sta();
	esp_netif_ip_info_t ip_info = {0};
//	esp_netif_dns_info_t p_dns_info = {0};
//	esp_netif_dns_info_t s_dns_info = {0};
	if(station){
		memset(&ip_info, 0, sizeof(esp_netif_ip_info_t));
		esp_netif_dhcpc_stop(station);
        ip_info.ip.addr = esp_ip4addr_aton((const char *)LocalIP);
        ip_info.netmask.addr = esp_ip4addr_aton((const char *)NetMask);
        ip_info.gw.addr = esp_ip4addr_aton((const char *)GateWay);
	    if (esp_netif_set_ip_info(station,&ip_info) != ESP_OK) {
	        ESP_LOGE("STATION SET IP", "Failed to set ip info");
	    }
/*
		memset(&p_dns_info, 0, sizeof(esp_netif_dns_info_t));
		p_dns_info.ip.u_addr.ip4.addr = esp_ip4addr_aton((const char *)PrimaryDNS);
		esp_netif_set_dns_info(WiFi_Netif,ESP_NETIF_DNS_MAIN, &p_dns_info);
		memset(&s_dns_info, 0, sizeof(esp_netif_dns_info_t));
		s_dns_info.ip.u_addr.ip4.addr = esp_ip4addr_aton((const char *)SeconDNS);
		esp_netif_set_dns_info(WiFi_Netif,ESP_NETIF_DNS_BACKUP, &s_dns_info);
		*/
	}
	return station;
}

char *LocalIP(esp_netif_t *WiFi_Netif){
	esp_netif_ip_info_t IP_info_t = {0};
	char *buf;
	buf = malloc(16*sizeof(char));
	esp_netif_get_ip_info(WiFi_Netif, &IP_info_t);
	esp_ip4addr_ntoa(&IP_info_t.ip, (char *)buf, 16);
	return (char *)buf;
}

uint8_t ScanWiFi(void){
	uint16_t ap_count = 0;
	memset(ap_info, 0, sizeof(ap_info));
	esp_wifi_scan_start(NULL, true);
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&number, ap_info));
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&ap_count));
    return (uint8_t)ap_count;
}

char *Scan_Get_SSID(uint8_t Number){
	char *buffer;
	uint8_t len = sizeof(ap_info[Number].ssid);
	buffer = malloc(len * sizeof(uint8_t));
	memcpy(buffer, ap_info[Number].ssid, sizeof(ap_info[Number].ssid));
	return buffer;
}





