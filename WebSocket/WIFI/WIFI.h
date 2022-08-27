/*
 * WIFI.h
 *
 *  Created on: 1 thg 4, 2022
 *      Author: A315-56
 */

#ifndef COMPONENTS_WIFI_INCLUDE_WIFI_H_
#define COMPONENTS_WIFI_INCLUDE_WIFI_H_


#include "esp_netif.h"
#include "esp_wifi.h"

#ifdef __cplusplus
extern "C" {
#endif

esp_netif_t *WiFi_Station_Init_Netif(char *SSID, char *PASS);
void WiFi_Station_Init(char *SSID, char *PASS);
esp_netif_t *WiFi_STA_Set_IP_Netif(char *LocalIP, char *GateWay, char *NetMask);
void WiFi_STA_Set_IP(esp_netif_t *WiFi_Netif, char *LocalIP, char *GateWay, char *NetMask);
//void WiFi_STA_Set_IP(esp_netif_t *WiFi_Netif, char *LocalIP, char *GateWay, char *NetMask, char *PrimaryDNS, char *SeconDNS);

esp_netif_t *WiFi_SoftAP_Init(char *SSID, char *PASS, uint8_t WiFi_Channel, uint8_t WiFi_Max_Connection);
void WiFi_AP_Set_IP(esp_netif_t *WiFi_Netif, char *LocalIP, char *GateWay, char *NetMask);

char *LocalIP(esp_netif_t *WiFi_Netif);

uint8_t ScanWiFi(void);
char *Scan_Get_SSID(uint8_t Number);





#ifdef __cplusplus
}
#endif

#endif /* COMPONENTS_WIFI_INCLUDE_WIFI_H_ */
