#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "sdkconfig.h"

static const char *TAG = "button";

const gpio_num_t LED = 8;
const gpio_num_t BTN = 3;

// Variable para registrar eventos de pulsación del botón. Como 
// puede modificarse desde el programa y desde la interrupción 
// es mejor declararla volatile para que no trate de optimizar
// el acceso.
// En C no existe un tipo nativo bool, pero:
// 1. ESP-IDF utiliza el compilador C++ por defecto para generar
//    el ejecutable,
// 2. En C99 se define el tipo de usuario bool en <stdbool.h>
volatile bool button_pressed = false;

// Manejador de las interrupciones debidas a la entrada digital.
// Elimina posibles rebotes limitando la tasa de llegadas a 2Hz (una
// interrupción cada 500ms).
// El manejador no hace casi nada, solo señalizar los eventos en una
// variable global, que debe ser declarada como volatile.
static void button_isr_handler(void* arg) 
{ 
    // Almacenamos la hora de la última interrupcion en una global (variable 
    // estática). La hora solo se utiliza aquí, así que no es necesario 
    // definirla como volatile.
    static TickType_t lastEvent;
    // Obtenemos la hora actual. Como es un manejador de interrupción tenemos
    // que usar funciones ...FromISR. Es decir, funciones async-safe.
    TickType_t current = xTaskGetTickCountFromISR();
    // portTICK_PERIOD_MS es el número de milisegundos en un tick del sistema
    // operativo. Por tanto 500 / portTICK_PERIOD_MS es el número de ticks en
    // 500 ms.
    if (current - lastEvent > 500 / portTICK_PERIOD_MS) { // debouncing 500ms
        button_pressed = true;
    }
    lastEvent = current;
}

void app_main(void)
{
    // Variable para el estado del LED. Solo se usa en esta función
    uint8_t s_led_state = 0;

    // Activa el servicio de manejo de interrupciones, que permite manejadores
    // independientes por cada pin de GPIO
    gpio_install_isr_service(0);

    // Resetea la configuración de los pines LED y BTN. De esta forma se
    // selecciona la función GPIO para estos pines y se desconecta cualquier 
    // otro periférico de estos pines
    gpio_reset_pin(LED);
    gpio_reset_pin(BTN);

    // Configura la dirección de los pines involucrados como entradas o
    // salidas
    gpio_set_direction(LED, GPIO_MODE_OUTPUT);
    gpio_set_direction(BTN, GPIO_MODE_INPUT);

    // Activa un pullup en la entrada
    gpio_pullup_en(BTN);
    // Configura la entrada por interrupciones en el flanco negativo (cuando 
    // pasa de nivel alto a nivel bajo).
    gpio_set_intr_type(BTN, GPIO_INTR_NEGEDGE);
    // La función button_isr_handler se configura como el manejador de las
    // interrupciones de la entrada
    gpio_isr_handler_add(BTN, button_isr_handler, NULL);

    // Tarea periódica. Si el botón ha sido pulsado se conmuta el estado
    TickType_t last = xTaskGetTickCount();
    for (;;) {
        if (button_pressed) {
            // Consume el evento escribiendo un false
            button_pressed = false;
            s_led_state = !s_led_state;
            ESP_LOGI(TAG, "Turning the LED %s!", s_led_state == true ? "ON" : "OFF");
        }
        // Pone el valor del estado en la salida
        gpio_set_level(LED, s_led_state);
        xTaskDelayUntil(&last, 500/ portTICK_PERIOD_MS);
    }
}
