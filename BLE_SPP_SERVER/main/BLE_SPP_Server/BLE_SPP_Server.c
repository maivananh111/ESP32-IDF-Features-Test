/*
 * BLE_SPP_Server.c
 *
 *  Created on: 17 thg 7, 2022
 *      Author: A315-56
 */

#include "BLE_SPP_Server.h"
#include "BLE_SPP_Def.h"

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#include "stdbool.h"
#include "string.h"


#define BLE_SPP_PROFILE_NUM     1
#define BLE_SPP_PROFILE_APP_IDX 0
#define BLE_SPP_APP_ID          0xCD
#define BLE_SPP_SVC_INST_ID	    0

#define LOG_COLOR_I LOG_COLOR(LOG_COLOR_CYAN)

static const char *BLE_SPP_TAG = "BLE_SPP_SERVER";

QueueHandle_t RecvQueue = NULL;

char *BLE_SPP_DEVICE_NAME;
uint16_t ble_spp_mtu_size      = 23;
uint16_t ble_spp_connect_id    = 0xffff;
esp_gatt_if_t ble_spp_gatts_if = 0xff;
uint16_t ble_spp_handle_table[SPP_IDX_NB];
esp_bd_addr_t spp_remote_bda   = {0x00,};
bool ble_spp_isconnected       = false;
bool ble_spp_enable_data_ntf   = false;

typedef struct gatts_profile_inst {
	esp_gatts_cb_t gatts_cb;
	uint16_t gatts_if;
	uint16_t app_id;
	uint16_t conn_id;
	uint16_t service_handle;
	esp_gatt_srvc_id_t service_id;
	uint16_t char_handle;
	esp_bt_uuid_t char_uuid;
	esp_gatt_perm_t perm;
	esp_gatt_char_prop_t property;
	uint16_t descr_handle;
	esp_bt_uuid_t descr_uuid;
} gatts_profile_inst;

static uint8_t spp_adv_data[31] = {
    /* Flags */
    0x02, 0x01, 0x06, // 0x01 có nghĩa là 3 bit đầu dạng Flag, 0x02 là độ dài tính từ sau nó(0x01, 0x06)
    /* Complete List of 16-bit Service Class UUIDs */
    0x03, 0x03, 0xF0, 0xAB,
    /* Complete Local Name in advertising */
    0x0A, 0x09, 'E', 'S', 'P', '3', '2', ' ', 'B', 'L', 'E' // 0x09 là complete local name, 0x0D là độ dài của complete local name có tính ký tự kết thúc chuỗi.
}; // Đống này là default.

static esp_ble_adv_params_t spp_adv_params = {
    .adv_int_min        = 0x20,
    .adv_int_max        = 0x40,
    .adv_type           = ADV_TYPE_IND,
    .own_addr_type      = BLE_ADDR_TYPE_PUBLIC,
    .channel_map        = ADV_CHNL_ALL,
    .adv_filter_policy  = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
};

// FUNCTION PROTOTYPE.
static void ble_gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param);
static void ble_gatts_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);
static void ble_gatts_profile_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);

static gatts_profile_inst ble_spp_profile_tab[BLE_SPP_PROFILE_NUM] = {
    [BLE_SPP_PROFILE_APP_IDX] = {
        .gatts_cb = ble_gatts_profile_event_handler,
        .gatts_if = ESP_GATT_IF_NONE,       /* Not get the gatt_if, so initial is ESP_GATT_IF_NONE */
    },
};


// FUNCTION SOURCE.
static uint8_t find_char_and_desr_index(uint16_t handle){
    uint8_t error = 0xff;
    for(int i = 0; i < SPP_IDX_NB ; i++){
        if(handle == ble_spp_handle_table[i]) return i;
    }
    return error;
}

static void ble_gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param){
    esp_err_t err;
    ESP_LOGW(BLE_SPP_TAG, "GAP EVENT: *%s*", BLE_GAP_EVT_STR[event]);

    switch (event) {
		case ESP_GAP_BLE_ADV_DATA_RAW_SET_COMPLETE_EVT:
			esp_ble_gap_start_advertising(&spp_adv_params);
		break;
		case ESP_GAP_BLE_ADV_START_COMPLETE_EVT:
			if((err = param->adv_start_cmpl.status) != ESP_BT_STATUS_SUCCESS) {
				ESP_LOGE(BLE_SPP_TAG, "Advertising start failed: %s", esp_err_to_name(err));
			}
		break;
		default:

        break;
    }
}

static void ble_gatts_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param){
//    ESP_LOGI(BLE_SPP_TAG, "GATT EVENT %d, gatts if %d", event, gatts_if);
	ESP_LOGI(BLE_SPP_TAG, "GATT EVENT: *%s*, gatts interface = %d", BLE_GATTS_EVT_STR[event], gatts_if);

    if (event == ESP_GATTS_REG_EVT) {
        if (param->reg.status == ESP_GATT_OK) {
        	ble_spp_profile_tab[BLE_SPP_PROFILE_APP_IDX].gatts_if = gatts_if;
        } else {
            ESP_LOGI(BLE_SPP_TAG, "Reg app failed, app_id %04x, status %d",param->reg.app_id, param->reg.status);
            return;
        }
    }

    do {
        int idx;
        for (idx = 0; idx < BLE_SPP_PROFILE_NUM; idx++) {
            if (gatts_if == ESP_GATT_IF_NONE || gatts_if == ble_spp_profile_tab[idx].gatts_if) {
                if (ble_spp_profile_tab[idx].gatts_cb) {
                	ble_spp_profile_tab[idx].gatts_cb(event, gatts_if, param);
                }
            }
        }
    } while(0);
}

static void ble_gatts_profile_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param){
	esp_ble_gatts_cb_param_t *p_data = (esp_ble_gatts_cb_param_t *) param;
	uint8_t res = 0xff;
//	ESP_LOGI(BLE_SPP_TAG, "GATT EVENT PROFILE %d, gatts if %d", event, gatts_if);
	switch(event) {
		case ESP_GATTS_REG_EVT:
			esp_ble_gap_set_device_name(BLE_SPP_DEVICE_NAME);
			esp_ble_gap_config_adv_data_raw((uint8_t *)spp_adv_data, sizeof(spp_adv_data));
			esp_ble_gatts_create_attr_tab(spp_gatt_db, gatts_if, SPP_IDX_NB, BLE_SPP_SVC_INST_ID);
		break;
		case ESP_GATTS_READ_EVT:
			res = find_char_and_desr_index(p_data->read.handle);
			if(res == SPP_IDX_SPP_STATUS_VAL){
				//TODO: Làm con mẹ gì đó ở đây.
			}
		break;
		case ESP_GATTS_WRITE_EVT:
			res = find_char_and_desr_index(p_data->write.handle);
			if(p_data->write.is_prep == false){
//				ESP_LOGI(BLE_SPP_TAG, "ESP_GATTS_WRITE_EVT : handle = %d", res);
				if(res == SPP_IDX_SPP_COMMAND_VAL){
					uint8_t * spp_cmd_buff = NULL;
					spp_cmd_buff = (uint8_t *)malloc((ble_spp_mtu_size - 3) * sizeof(uint8_t));
					if(spp_cmd_buff == NULL){
						ESP_LOGE(BLE_SPP_TAG, "%s malloc failed", __func__);
						break;
					}
					memset(spp_cmd_buff, 0x0, (ble_spp_mtu_size - 3));
					memcpy(spp_cmd_buff, p_data->write.value, p_data->write.len);
					ESP_LOGW(BLE_SPP_TAG, "SPP command: %s", spp_cmd_buff);
				}
				else if(res == SPP_IDX_SPP_DATA_NTF_CFG){
					if((p_data->write.len == 2) && (p_data->write.value[0] == 0x01) && (p_data->write.value[1] == 0x00)){
						ble_spp_enable_data_ntf = true;
					}
					else if((p_data->write.len == 2) && (p_data->write.value[0] == 0x00) && (p_data->write.value[1] == 0x00)){
						ble_spp_enable_data_ntf = false;
					}
				}
				else if(res == SPP_IDX_SPP_DATA_RECV_VAL){
					ESP_LOGI(BLE_SPP_TAG, "Receive data: %s", p_data->write.value);
					BLE_SPP_Recv_EventCallback((void *)p_data->write.value);
                    uint8_t * RxData = NULL;
                    uint16_t length = strlen((char *)p_data->write.value);
                    RxData = (uint8_t *)malloc(length * sizeof(uint8_t));
                    memset(RxData, 0x00, length);
                    memcpy(RxData, p_data->write.value, length);
                    xQueueSend(RecvQueue, &RxData, 10/portTICK_PERIOD_MS);
				}
				else{
					//TODO: Làm con mẹ gì đó ở đây.
				}
			}else if((p_data->write.is_prep == true)&&(res == SPP_IDX_SPP_DATA_RECV_VAL)){
				ESP_LOGI(BLE_SPP_TAG, "ESP_GATTS_PREP_WRITE_EVT : handle = %d", res);
//				store_wr_buffer(p_data);
			}
		break;

		case ESP_GATTS_EXEC_WRITE_EVT:
/*	    	    if(p_data->exec_write.exec_write_flag){
	    	        print_write_buffer();
	    	        free_write_buffer();
	    	    }*/
		break;

		case ESP_GATTS_MTU_EVT:
			ble_spp_mtu_size = p_data->mtu.mtu;
			ESP_LOGI(BLE_SPP_TAG, "Maximum transmit unit size: %d", ble_spp_mtu_size);
		break;
		case ESP_GATTS_CONF_EVT:
		break;
		case ESP_GATTS_UNREG_EVT:
		break;
		case ESP_GATTS_DELETE_EVT:
		break;
		case ESP_GATTS_START_EVT:
		break;
		case ESP_GATTS_STOP_EVT:
		break;
		case ESP_GATTS_CONNECT_EVT:
			ble_spp_connect_id = p_data->connect.conn_id;
			ESP_LOGW(BLE_SPP_TAG, "BLE SSP CONNECTED TO DEVICE, ID: %d", ble_spp_connect_id);
			ble_spp_gatts_if = gatts_if;
			ble_spp_isconnected = true;
			BLE_SPP_Conn_EventCallback((void *)"CONNECTED");
			memcpy(&spp_remote_bda, &p_data->connect.remote_bda, sizeof(esp_bd_addr_t));
		break;
		case ESP_GATTS_DISCONNECT_EVT:
			ESP_LOGW(BLE_SPP_TAG, "BLE SSP DISCONNECTED TO DEVICE");
			ble_spp_isconnected = false;
			ble_spp_enable_data_ntf = false;
		    BLE_SPP_Disconn_EventCallback((void *)"DISCONNECTED");
			esp_ble_gap_start_advertising(&spp_adv_params);
		break;
		case ESP_GATTS_OPEN_EVT:
		break;
		case ESP_GATTS_CANCEL_OPEN_EVT:
		break;
		case ESP_GATTS_CLOSE_EVT:
		break;
		case ESP_GATTS_LISTEN_EVT:
		break;
		case ESP_GATTS_CONGEST_EVT:
		break;
		case ESP_GATTS_CREAT_ATTR_TAB_EVT:
			ESP_LOGI(BLE_SPP_TAG, "The number handle = %d", param->add_attr_tab.num_handle);
			if (param->add_attr_tab.status != ESP_GATT_OK){
				ESP_LOGE(BLE_SPP_TAG, "Create attribute table failed, error code=0x%x", param->add_attr_tab.status);
			}
			else if (param->add_attr_tab.num_handle != SPP_IDX_NB){
				ESP_LOGE(BLE_SPP_TAG, "Create attribute table abnormally, num_handle (%d) doesn't equal to HRS_IDX_NB(%d)", param->add_attr_tab.num_handle, SPP_IDX_NB);
			}
			else {
				memcpy(ble_spp_handle_table, param->add_attr_tab.handles, sizeof(ble_spp_handle_table));
				esp_ble_gatts_start_service(ble_spp_handle_table[SPP_IDX_SVC]);
			}
		break;
		default:
		break;
	}
}



void BLE_SPP_Server_Init(char *Bluetooth_DeviceName){
	esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));

    RecvQueue = xQueueCreate(2, sizeof(uint32_t));
    asprintf(&BLE_SPP_DEVICE_NAME, "%s", Bluetooth_DeviceName);
    spp_adv_data[7] = (uint8_t)strlen(Bluetooth_DeviceName) + 1;
    for(int i=0; i<strlen(Bluetooth_DeviceName); i++){
    	spp_adv_data[i+9] = Bluetooth_DeviceName[i];
    }

    esp_bt_controller_init(&bt_cfg);
    esp_bt_controller_enable(ESP_BT_MODE_BLE);
    esp_bluedroid_init();
    esp_bluedroid_enable();

    esp_ble_gap_register_callback(ble_gap_event_handler);
	esp_ble_gatts_register_callback(ble_gatts_event_handler);
    esp_ble_gatts_app_register(BLE_SPP_APP_ID);
}

void BLE_SPP_SendText(char *Text){
	esp_ble_gatts_send_indicate(ble_spp_gatts_if, ble_spp_connect_id, ble_spp_handle_table[SPP_IDX_SPP_DATA_NTY_VAL], strlen(Text), (uint8_t *)Text, false);
	BLE_SPP_Send_EventCallback((void *)Text);
}

bool BLE_SSP_IsConnected(void){
	return ble_spp_isconnected;
}

__weak_symbol void BLE_SPP_Conn_EventCallback(void *Parameters){
	ESP_LOGW(BLE_SPP_TAG, "ESP BLE Connected event, Parameter: %s\n", (char *)Parameters);
}
__weak_symbol void BLE_SPP_Disconn_EventCallback(void *Parameters){
	ESP_LOGW(BLE_SPP_TAG, "ESP BLE Disconnected event, Parameter: %s\n", (char *)Parameters);
}
__weak_symbol void BLE_SPP_Send_EventCallback(void *Parameters){
	ESP_LOGW(BLE_SPP_TAG, "ESP BLE Transmit event, Parameter: %s\n", (char *)Parameters);
}
__weak_symbol void BLE_SPP_Recv_EventCallback(void *Parameters){
	ESP_LOGW(BLE_SPP_TAG, "ESP BLE Receive event, Parameter: %s\n", (char *)Parameters);
}













