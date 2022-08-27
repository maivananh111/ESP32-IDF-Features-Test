
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"

#include "esp_system.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "esp_bt.h"
#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_bt_defs.h"
#include "esp_bt_main.h"
#include "BLE_SPP_Server.h"

#include "sdkconfig.h"
#include "driver/gpio.h"

#include "stdio.h"
#include "stdlib.h"
#include "string.h"


static const char *TAG = "MAIN";
extern QueueHandle_t RecvQueue;

static void ble_spp_sendTask(void *pvParameters){
	char *rxdata;
	char *Resp_data = "OK";
	while(1){
		vTaskDelay(50/portTICK_PERIOD_MS);
		if(xQueueReceive(RecvQueue, &rxdata, portMAX_DELAY)) {
			BLE_SPP_SendText(Resp_data);
			free(rxdata);
		}
	}
	vTaskDelete(NULL);
}

void BLE_SPP_Recv_EventCallback(void *Parameters){
	char *RecvData = (char *)Parameters;
	if(strcmp(RecvData, "ON\n") == 0){
		ESP_LOGI(TAG, "COMMAND ON LED.");
		gpio_set_level(GPIO_NUM_2, 1);
	}
	else if(strcmp(RecvData, "OFF\n") == 0){
		ESP_LOGI(TAG, "COMMAND OFF LED.");
		gpio_set_level(GPIO_NUM_2, 0);
	}
	else{
		ESP_LOGI(TAG, "INCORRECT COMMAND OR ORTHER COMMAND.");
	}
}

void app_main(void){
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    } ESP_ERROR_CHECK(ret);

    gpio_set_direction(GPIO_NUM_2, GPIO_MODE_OUTPUT);
    ESP_LOGI(TAG, "ESP32 Bluetooth low energy.");
    BLE_SPP_Server_Init("ESP32 BLE");
    xTaskCreate(ble_spp_sendTask, "ble_spp_sendTask", 2048, NULL, 5, NULL);
}








