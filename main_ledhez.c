#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"

#include "app_defs.h"
#include "mqtt_connector.h"
#include "wifi_connector.h"

#define LED_PIN GPIO_NUM_6

EventGroupHandle_t my_event_group;

QueueHandle_t led_queue;

bool led_state = false;

static void handle_led_blink(void *params) {

	gpio_config_t led_config = {
		.pin_bit_mask = (1ULL << LED_PIN),
		.mode = GPIO_MODE_OUTPUT,
	};
	ESP_ERROR_CHECK(gpio_config(&led_config));

	while (true) {
        if (led_state) {
            gpio_set_level(LED_PIN, true);
            vTaskDelay(100);
            gpio_set_level(LED_PIN, false);
            vTaskDelay(100);
        }
	}
}

static void handle_led_queue(void *params) {
	while (true) {
		xQueueReceive(led_queue, &led_state, portMAX_DELAY);
	}
}


void app_main(void) {

	esp_err_t ret = nvs_flash_init();

	if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
		ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
		ESP_ERROR_CHECK(nvs_flash_erase());
		ret = nvs_flash_init();
	}

	ESP_ERROR_CHECK(ret);

	my_event_group = xEventGroupCreate();

	wifi_init_phase();
	wifi_config_phase();

	ESP_ERROR_CHECK(wifi_start_phase());

	led_queue = xQueueCreate(5, sizeof(int));

	ESP_ERROR_CHECK(mqtt_init());
	ESP_ERROR_CHECK(mqtt_subscribe_to_led());

	xTaskCreate(handle_led_blink, "Handle LED Blink", 2048, NULL, 1, NULL);
    xTaskCreate(handle_led_queue, "Handle LED Queue", 2048, NULL, 1, NULL);
}
