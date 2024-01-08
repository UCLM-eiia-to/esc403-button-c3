#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "sdkconfig.h"

static const char *TAG = "button";

const gpio_num_t LED = 8;
const gpio_num_t BTN = 3;

volatile bool button_pressed = false;
static void button_isr_handler(void* arg) 
{ 
    static TickType_t lastEvent; // last time stamp
    TickType_t current = xTaskGetTickCountFromISR();
    if (current - lastEvent > 500 / portTICK_PERIOD_MS) { // debouncing 500ms
        button_pressed = true;
    }
    lastEvent = current;
}

void app_main(void)
{
    static uint8_t s_led_state = 0;

    // per pin ISR
    gpio_install_isr_service(0);

    // reset config
    gpio_reset_pin(LED);
    gpio_reset_pin(BTN);

    // config direction
    gpio_set_direction(LED, GPIO_MODE_OUTPUT);
    gpio_set_direction(BTN, GPIO_MODE_INPUT);

    // pullup and ISR
    gpio_pullup_en(BTN);
    gpio_set_intr_type(BTN, GPIO_INTR_NEGEDGE);
    gpio_isr_handler_add(BTN, button_isr_handler, NULL);

    TickType_t last = xTaskGetTickCount();
    for (;;) {
        if (button_pressed) {
            button_pressed = false; // consume event
            s_led_state = !s_led_state;
            ESP_LOGI(TAG, "Turning the LED %s!", s_led_state == true ? "ON" : "OFF");
        }
        gpio_set_level(LED, s_led_state);
        xTaskDelayUntil(&last, 500/ portTICK_PERIOD_MS);
    }
}
