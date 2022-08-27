/*
 * BLE_SPP_Def.h
 *
 *  Created on: 17 thg 7, 2022
 *      Author: A315-56
 */

#ifndef BLE_SPP_DEF_H_
#define BLE_SPP_DEF_H_

#include "stdio.h"

enum {
    SPP_IDX_SVC,

    SPP_IDX_SPP_DATA_RECV_CHAR,
    SPP_IDX_SPP_DATA_RECV_VAL,

    SPP_IDX_SPP_DATA_NOTIFY_CHAR,
    SPP_IDX_SPP_DATA_NTY_VAL,
    SPP_IDX_SPP_DATA_NTF_CFG,

    SPP_IDX_SPP_COMMAND_CHAR,
    SPP_IDX_SPP_COMMAND_VAL,

    SPP_IDX_SPP_STATUS_CHAR,
    SPP_IDX_SPP_STATUS_VAL,
    SPP_IDX_SPP_STATUS_CFG,

    SPP_IDX_NB,
};

//typedef enum {
//    ESP_GATTS_REG_EVT                 = 0,       /*!< When register application id, the event comes */
//    ESP_GATTS_READ_EVT                = 1,       /*!< When gatt client request read operation, the event comes */
//    ESP_GATTS_WRITE_EVT               = 2,       /*!< When gatt client request write operation, the event comes */
//    ESP_GATTS_EXEC_WRITE_EVT          = 3,       /*!< When gatt client request execute write, the event comes */
//    ESP_GATTS_MTU_EVT                 = 4,       /*!< When set mtu complete, the event comes */
//    ESP_GATTS_CONF_EVT                = 5,       /*!< When receive confirm, the event comes */
//    ESP_GATTS_UNREG_EVT               = 6,       /*!< When unregister application id, the event comes */
//    ESP_GATTS_CREATE_EVT              = 7,       /*!< When create service complete, the event comes */
//    ESP_GATTS_ADD_INCL_SRVC_EVT       = 8,       /*!< When add included service complete, the event comes */
//    ESP_GATTS_ADD_CHAR_EVT            = 9,       /*!< When add characteristic complete, the event comes */
//    ESP_GATTS_ADD_CHAR_DESCR_EVT      = 10,      /*!< When add descriptor complete, the event comes */
//    ESP_GATTS_DELETE_EVT              = 11,      /*!< When delete service complete, the event comes */
//    ESP_GATTS_START_EVT               = 12,      /*!< When start service complete, the event comes */
//    ESP_GATTS_STOP_EVT                = 13,      /*!< When stop service complete, the event comes */
//    ESP_GATTS_CONNECT_EVT             = 14,      /*!< When gatt client connect, the event comes */
//    ESP_GATTS_DISCONNECT_EVT          = 15,      /*!< When gatt client disconnect, the event comes */
//    ESP_GATTS_OPEN_EVT                = 16,      /*!< When connect to peer, the event comes */
//    ESP_GATTS_CANCEL_OPEN_EVT         = 17,      /*!< When disconnect from peer, the event comes */
//    ESP_GATTS_CLOSE_EVT               = 18,      /*!< When gatt server close, the event comes */
//    ESP_GATTS_LISTEN_EVT              = 19,      /*!< When gatt listen to be connected the event comes */
//    ESP_GATTS_CONGEST_EVT             = 20,      /*!< When congest happen, the event comes */
//    /* following is extra event */
//    ESP_GATTS_RESPONSE_EVT            = 21,      /*!< When gatt send response complete, the event comes */
//    ESP_GATTS_CREAT_ATTR_TAB_EVT      = 22,      /*!< When gatt create table complete, the event comes */
//    ESP_GATTS_SET_ATTR_VAL_EVT        = 23,      /*!< When gatt set attr value complete, the event comes */
//    ESP_GATTS_SEND_SERVICE_CHANGE_EVT = 24,      /*!< When gatt send service change indication complete, the event comes */
//} esp_gatts_cb_event_t;

#define CHAR_DECLARATION_SIZE   (sizeof(uint8_t))

static const uint16_t ESP_GATT_UUID_SPP_SERVICE_UUID    = 0xABF0;
static const uint16_t ESP_GATT_UUID_SPP_DATA_RECEIVE    = 0xABF1;
static const uint16_t ESP_GATT_UUID_SPP_DATA_NOTIFY     = 0xABF2;
static const uint16_t ESP_GATT_UUID_SPP_COMMAND_RECEIVE = 0xABF3;
static const uint16_t ESP_GATT_UUID_SPP_COMMAND_NOTIFY  = 0xABF4;

#define SPP_DATA_MAX_LEN           (512)
#define SPP_CMD_MAX_LEN            (20)
#define SPP_STATUS_MAX_LEN         (20)
#define SPP_DATA_BUFF_MAX_LEN      (2*1024)

static const uint16_t primary_service_uuid = ESP_GATT_UUID_PRI_SERVICE;
static const uint16_t character_declaration_uuid = ESP_GATT_UUID_CHAR_DECLARE;
static const uint16_t character_client_config_uuid = ESP_GATT_UUID_CHAR_CLIENT_CONFIG;

static const uint8_t char_prop_read_notify = ESP_GATT_CHAR_PROP_BIT_READ|ESP_GATT_CHAR_PROP_BIT_NOTIFY;
static const uint8_t char_prop_read_write = ESP_GATT_CHAR_PROP_BIT_WRITE_NR|ESP_GATT_CHAR_PROP_BIT_READ;

///SPP Service - data receive characteristic, read&write without response
static const uint16_t spp_data_receive_uuid = ESP_GATT_UUID_SPP_DATA_RECEIVE;
static const uint8_t  spp_data_receive_val[20] = {0x00};

///SPP Service - data notify characteristic, notify&read
static const uint16_t spp_data_notify_uuid = ESP_GATT_UUID_SPP_DATA_NOTIFY;
static const uint8_t  spp_data_notify_val[20] = {0x00};
static const uint8_t  spp_data_notify_ccc[2] = {0x00, 0x00};

///SPP Service - command characteristic, read&write without response
static const uint16_t spp_command_uuid = ESP_GATT_UUID_SPP_COMMAND_RECEIVE;
static const uint8_t  spp_command_val[10] = {0x00};

///SPP Service - status characteristic, notify&read
static const uint16_t spp_status_uuid = ESP_GATT_UUID_SPP_COMMAND_NOTIFY;
static const uint8_t  spp_status_val[10] = {0x00};
static const uint8_t  spp_status_ccc[2] = {0x00, 0x00};

///Full HRS Database Description - Used to add attributes into the database
static const esp_gatts_attr_db_t spp_gatt_db[SPP_IDX_NB] = {
    //SPP -  Service Declaration
    [SPP_IDX_SVC]                  =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&primary_service_uuid, ESP_GATT_PERM_READ,
    sizeof(ESP_GATT_UUID_SPP_SERVICE_UUID), sizeof(ESP_GATT_UUID_SPP_SERVICE_UUID), (uint8_t *)&ESP_GATT_UUID_SPP_SERVICE_UUID}},

    //SPP -  data receive characteristic Declaration
    [SPP_IDX_SPP_DATA_RECV_CHAR]   =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ,
    CHAR_DECLARATION_SIZE, CHAR_DECLARATION_SIZE, (uint8_t *)&char_prop_read_write}},

    //SPP -  data receive characteristic Value
    [SPP_IDX_SPP_DATA_RECV_VAL]    =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&spp_data_receive_uuid, ESP_GATT_PERM_READ|ESP_GATT_PERM_WRITE,
    SPP_DATA_MAX_LEN, sizeof(spp_data_receive_val), (uint8_t *)spp_data_receive_val}},

    //SPP -  data notify characteristic Declaration
    [SPP_IDX_SPP_DATA_NOTIFY_CHAR] =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ,
    CHAR_DECLARATION_SIZE,CHAR_DECLARATION_SIZE, (uint8_t *)&char_prop_read_notify}},

    //SPP -  data notify characteristic Value
    [SPP_IDX_SPP_DATA_NTY_VAL]     =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&spp_data_notify_uuid, ESP_GATT_PERM_READ,
    SPP_DATA_MAX_LEN, sizeof(spp_data_notify_val), (uint8_t *)spp_data_notify_val}},

    //SPP -  data notify characteristic - Client Characteristic Configuration Descriptor
    [SPP_IDX_SPP_DATA_NTF_CFG]     =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_client_config_uuid, ESP_GATT_PERM_READ|ESP_GATT_PERM_WRITE,
    sizeof(uint16_t),sizeof(spp_data_notify_ccc), (uint8_t *)spp_data_notify_ccc}},

    //SPP -  command characteristic Declaration
    [SPP_IDX_SPP_COMMAND_CHAR]     =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ,
    CHAR_DECLARATION_SIZE,CHAR_DECLARATION_SIZE, (uint8_t *)&char_prop_read_write}},

    //SPP -  command characteristic Value
    [SPP_IDX_SPP_COMMAND_VAL]      =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&spp_command_uuid, ESP_GATT_PERM_READ|ESP_GATT_PERM_WRITE,
    SPP_CMD_MAX_LEN,sizeof(spp_command_val), (uint8_t *)spp_command_val}},

    //SPP -  status characteristic Declaration
    [SPP_IDX_SPP_STATUS_CHAR]      =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ,
    CHAR_DECLARATION_SIZE,CHAR_DECLARATION_SIZE, (uint8_t *)&char_prop_read_notify}},

    //SPP -  status characteristic Value
    [SPP_IDX_SPP_STATUS_VAL]       =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&spp_status_uuid, ESP_GATT_PERM_READ,
    SPP_STATUS_MAX_LEN,sizeof(spp_status_val), (uint8_t *)spp_status_val}},

    //SPP -  status characteristic - Client Characteristic Configuration Descriptor
    [SPP_IDX_SPP_STATUS_CFG]       =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_client_config_uuid, ESP_GATT_PERM_READ|ESP_GATT_PERM_WRITE,
    sizeof(uint16_t),sizeof(spp_status_ccc), (uint8_t *)spp_status_ccc}},
};



static const char *BLE_GATTS_EVT_STR[] = {
    "ESP_GATTS_REG_EVT"                 ,       /*!< When register application id, the event comes */
    "ESP_GATTS_READ_EVT"                ,       /*!< When gatt client request read operation, the event comes */
    "ESP_GATTS_WRITE_EVT"               ,       /*!< When gatt client request write operation, the event comes */
    "ESP_GATTS_EXEC_WRITE_EVT"          ,       /*!< When gatt client request execute write, the event comes */
    "ESP_GATTS_MTU_EVT"                 ,       /*!< When set mtu complete, the event comes */
    "ESP_GATTS_CONF_EVT"                ,       /*!< When receive confirm, the event comes */
    "ESP_GATTS_UNREG_EVT"               ,       /*!< When unregister application id, the event comes */
    "ESP_GATTS_CREATE_EVT"              ,       /*!< When create service complete, the event comes */
    "ESP_GATTS_ADD_INCL_SRVC_EVT"       ,       /*!< When add included service complete, the event comes */
    "ESP_GATTS_ADD_CHAR_EVT"            ,       /*!< When add characteristic complete, the event comes */
    "ESP_GATTS_ADD_CHAR_DESCR_EVT"      ,      /*!< When add descriptor complete, the event comes */
    "ESP_GATTS_DELETE_EVT"              ,      /*!< When delete service complete, the event comes */
    "ESP_GATTS_START_EVT"               ,      /*!< When start service complete, the event comes */
    "ESP_GATTS_STOP_EVT"                ,      /*!< When stop service complete, the event comes */
    "ESP_GATTS_CONNECT_EVT"             ,      /*!< When gatt client connect, the event comes */
    "ESP_GATTS_DISCONNECT_EVT"          ,      /*!< When gatt client disconnect, the event comes */
    "ESP_GATTS_OPEN_EVT"                ,      /*!< When connect to peer, the event comes */
    "ESP_GATTS_CANCEL_OPEN_EVT"         ,      /*!< When disconnect from peer, the event comes */
    "ESP_GATTS_CLOSE_EVT"               ,      /*!< When gatt server close, the event comes */
    "ESP_GATTS_LISTEN_EVT"              ,      /*!< When gatt listen to be connected the event comes */
    "ESP_GATTS_CONGEST_EVT"             ,      /*!< When congest happen, the event comes */
    /* following is extra event */
    "ESP_GATTS_RESPONSE_EVT"            ,      /*!< When gatt send response complete, the event comes */
    "ESP_GATTS_CREAT_ATTR_TAB_EVT"      ,      /*!< When gatt create table complete, the event comes */
    "ESP_GATTS_SET_ATTR_VAL_EVT"        ,      /*!< When gatt set attr value complete, the event comes */
    "ESP_GATTS_SEND_SERVICE_CHANGE_EVT" ,      /*!< When gatt send service change indication complete, the event comes */
};

static const char *BLE_GAP_EVT_STR[] = {
#if (BLE_42_FEATURE_SUPPORT == TRUE)
    "ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT",          /*!< When advertising data set complete, the event comes = 0 */
    "ESP_GAP_BLE_SCAN_RSP_DATA_SET_COMPLETE_EVT",             /*!< When scan response data set complete, the event comes */
    "ESP_GAP_BLE_SCAN_PARAM_SET_COMPLETE_EVT",                /*!< When scan parameters set complete, the event comes */
    "ESP_GAP_BLE_SCAN_RESULT_EVT",                            /*!< When one scan result ready, the event comes each time */
    "ESP_GAP_BLE_ADV_DATA_RAW_SET_COMPLETE_EVT",              /*!< When raw advertising data set complete, the event comes */
    "ESP_GAP_BLE_SCAN_RSP_DATA_RAW_SET_COMPLETE_EVT",         /*!< When raw advertising data set complete, the event comes */
    "ESP_GAP_BLE_ADV_START_COMPLETE_EVT",                     /*!< When start advertising complete, the event comes */
    "ESP_GAP_BLE_SCAN_START_COMPLETE_EVT",                    /*!< When start scan complete, the event comes */
#endif // #if (BLE_42_FEATURE_SUPPORT == TRUE)
    "ESP_GAP_BLE_AUTH_CMPL_EVT",                          /* Authentication complete indication. = 8 */
    "ESP_GAP_BLE_KEY_EVT",                                    /* BLE  key event for peer device keys */
    "ESP_GAP_BLE_SEC_REQ_EVT",                                /* BLE  security request */
    "ESP_GAP_BLE_PASSKEY_NOTIF_EVT",                          /* passkey notification event */
    "ESP_GAP_BLE_PASSKEY_REQ_EVT",                            /* passkey request event */
    "ESP_GAP_BLE_OOB_REQ_EVT",                                /* OOB request event */
    "ESP_GAP_BLE_LOCAL_IR_EVT",                               /* BLE local IR event */
    "ESP_GAP_BLE_LOCAL_ER_EVT",                               /* BLE local ER event */
    "ESP_GAP_BLE_NC_REQ_EVT",                                 /* Numeric Comparison request event */
#if (BLE_42_FEATURE_SUPPORT == TRUE)
    "ESP_GAP_BLE_ADV_STOP_COMPLETE_EVT",                      /*!< When stop adv complete, the event comes */
    "ESP_GAP_BLE_SCAN_STOP_COMPLETE_EVT",                     /*!< When stop scan complete, the event comes */
#endif // #if (BLE_42_FEATURE_SUPPORT == TRUE)
    "ESP_GAP_BLE_SET_STATIC_RAND_ADDR_EVT",              /*!< When set the static rand address complete, the event comes = 19 */
    "ESP_GAP_BLE_UPDATE_CONN_PARAMS_EVT",                     /*!< When update connection parameters complete, the event comes */
    "ESP_GAP_BLE_SET_PKT_LENGTH_COMPLETE_EVT",                /*!< When set pkt length complete, the event comes */
    "ESP_GAP_BLE_SET_LOCAL_PRIVACY_COMPLETE_EVT",             /*!< When  Enable/disable privacy on the local device complete, the event comes */
    "ESP_GAP_BLE_REMOVE_BOND_DEV_COMPLETE_EVT",               /*!< When remove the bond device complete, the event comes */
    "ESP_GAP_BLE_CLEAR_BOND_DEV_COMPLETE_EVT",                /*!< When clear the bond device clear complete, the event comes */
    "ESP_GAP_BLE_GET_BOND_DEV_COMPLETE_EVT",                  /*!< When get the bond device list complete, the event comes */
    "ESP_GAP_BLE_READ_RSSI_COMPLETE_EVT",                     /*!< When read the rssi complete, the event comes */
    "ESP_GAP_BLE_UPDATE_WHITELIST_COMPLETE_EVT",              /*!< When add or remove whitelist complete, the event comes */
#if (BLE_42_FEATURE_SUPPORT == TRUE)
    "ESP_GAP_BLE_UPDATE_DUPLICATE_EXCEPTIONAL_LIST_COMPLETE_EVT",  /*!< When update duplicate exceptional list complete, the event comes */
#endif //#if (BLE_42_FEATURE_SUPPORT == TRUE)
    "ESP_GAP_BLE_SET_CHANNELS_EVT",                           /*!< When setting BLE channels complete, the event comes = 29 */
#if (BLE_50_FEATURE_SUPPORT == TRUE)
    "ESP_GAP_BLE_READ_PHY_COMPLETE_EVT",
    "ESP_GAP_BLE_SET_PREFERED_DEFAULT_PHY_COMPLETE_EVT",
    "ESP_GAP_BLE_SET_PREFERED_PHY_COMPLETE_EVT",
    "ESP_GAP_BLE_EXT_ADV_SET_RAND_ADDR_COMPLETE_EVT",
    "ESP_GAP_BLE_EXT_ADV_SET_PARAMS_COMPLETE_EVT",
    "ESP_GAP_BLE_EXT_ADV_DATA_SET_COMPLETE_EVT",
    "ESP_GAP_BLE_EXT_SCAN_RSP_DATA_SET_COMPLETE_EVT",
    "ESP_GAP_BLE_EXT_ADV_START_COMPLETE_EVT",
    "ESP_GAP_BLE_EXT_ADV_STOP_COMPLETE_EVT",
    "ESP_GAP_BLE_EXT_ADV_SET_REMOVE_COMPLETE_EVT",
    "ESP_GAP_BLE_EXT_ADV_SET_CLEAR_COMPLETE_EVT",
    "ESP_GAP_BLE_PERIODIC_ADV_SET_PARAMS_COMPLETE_EVT",
    "ESP_GAP_BLE_PERIODIC_ADV_DATA_SET_COMPLETE_EVT",
    "ESP_GAP_BLE_PERIODIC_ADV_START_COMPLETE_EVT",
    "ESP_GAP_BLE_PERIODIC_ADV_STOP_COMPLETE_EVT",
    "ESP_GAP_BLE_PERIODIC_ADV_CREATE_SYNC_COMPLETE_EVT",
    "ESP_GAP_BLE_PERIODIC_ADV_SYNC_CANCEL_COMPLETE_EVT",
    "ESP_GAP_BLE_PERIODIC_ADV_SYNC_TERMINATE_COMPLETE_EVT",
    "ESP_GAP_BLE_PERIODIC_ADV_ADD_DEV_COMPLETE_EVT",
    "ESP_GAP_BLE_PERIODIC_ADV_REMOVE_DEV_COMPLETE_EVT",
    "ESP_GAP_BLE_PERIODIC_ADV_CLEAR_DEV_COMPLETE_EVT",
    "ESP_GAP_BLE_SET_EXT_SCAN_PARAMS_COMPLETE_EVT",
    "ESP_GAP_BLE_EXT_SCAN_START_COMPLETE_EVT",
    "ESP_GAP_BLE_EXT_SCAN_STOP_COMPLETE_EVT",
    "ESP_GAP_BLE_PREFER_EXT_CONN_PARAMS_SET_COMPLETE_EVT",
    "ESP_GAP_BLE_PHY_UPDATE_COMPLETE_EVT",
    "ESP_GAP_BLE_EXT_ADV_REPORT_EVT",
    "ESP_GAP_BLE_SCAN_TIMEOUT_EVT",
    "ESP_GAP_BLE_ADV_TERMINATED_EVT",
    "ESP_GAP_BLE_SCAN_REQ_RECEIVED_EVT",
    "ESP_GAP_BLE_CHANNEL_SELETE_ALGORITHM_EVT",
    "ESP_GAP_BLE_PERIODIC_ADV_REPORT_EVT",
    "ESP_GAP_BLE_PERIODIC_ADV_SYNC_LOST_EVT",
    "ESP_GAP_BLE_PERIODIC_ADV_SYNC_ESTAB_EVT",
#endif // #if (BLE_50_FEATURE_SUPPORT == TRUE)
    "ESP_GAP_BLE_EVT_MAX",
};

#endif /* BLE_SPP_DEF_H_ */
