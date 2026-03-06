#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/Task.h"

#define BTN_PIN GPIO_NUM_4
#define LED_PIN GPIO_NUM_6

#define DOUBLE_MS 500

bool btn_state = false;
bool prev_btn_state = false;

bool led_state = false;

bool do_count = false;
int timer_cntr = 0;

void handle_btn (void *params) {
    while (true) {
        btn_state = gpio_get_level(BTN_PIN);
        if (btn_state == true && prev_btn_state == false) {
            do_count = !do_count;
            if (timer_cntr < DOUBLE_MS && do_count == false) {
                led_state = !led_state;
            } else {
                do_count = true;
            }
            timer_cntr = 0;
        }
        prev_btn_state = btn_state;
        vTaskDelay(1);
    }
}

void handle_led (void *params) {
    while (true) {
        gpio_set_level(LED_PIN, led_state);
        vTaskDelay(1);
    }
}

void stop_watch (void *params) {
    while (true) {
        if (do_count) {
            timer_cntr++;
        }
        vTaskDelay(1);
    }
}

void app_main(void) {

    gpio_config_t btn_config = {
        .pin_bit_mask = 1ULL << BTN_PIN,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = 0,
        .pull_down_en = 1,
    };

    gpio_config_t led_config = {
        .pin_bit_mask = 1ULL << LED_PIN,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = 0,
        .pull_down_en = 0,
    };

    gpio_config(&btn_config);
    gpio_config(&led_config);

    xTaskCreate(handle_btn, "Handle Button", 2048, NULL, 1, NULL);
    xTaskCreate(handle_led, "Handle LED", 2048, NULL, 1, NULL);
    xTaskCreate(stop_watch, "Stopwatch", 2048, NULL, 1, NULL);

}
