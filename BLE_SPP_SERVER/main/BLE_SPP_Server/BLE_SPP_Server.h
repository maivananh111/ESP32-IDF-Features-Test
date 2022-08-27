/*
 * BLE_SPP.h
 *
 *  Created on: 17 thg 7, 2022
 *      Author: A315-56
 */

#ifndef BLE_SPP_H_
#define BLE_SPP_H_

#include "esp_bt.h"
#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_bt_defs.h"
#include "esp_bt_main.h"


bool BLE_SSP_IsConnected(void);
void BLE_SPP_Server_Init(char *Bluetooth_DeviceName);
void BLE_SPP_SendText(char *Text);
void BLE_SPP_RecvText(void);
void BLE_SPP_Conn_EventCallback(void *Parameters);
void BLE_SPP_Disconn_EventCallback(void *Parameters);
void BLE_SPP_Send_EventCallback(void *Parameters);
void BLE_SPP_Recv_EventCallback(void *Parameters);

#endif /* BLE_SPP_H_ */
