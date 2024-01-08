# Ejemplo de E/S

En este ejemplo se demuestra el uso de entradas y salidas digitales para un ESP32-C3.  Se configura un pin como entrada por interrupciones.

El pin 3 se configura con pullup y la interrupción es disparada por flanco de bajada (NEGEDGE). Se implementa un debounce sencillo.

Se configura GPIO8 como salida digital para comandar el LED azul incluido en el ESP32 C3 mini.

Esta es la forma habitual en la que utilizaremos las E/S en este curso.

Además, se utiliza wokwi para la simulación. Instala la extensión Wokwi Simulator para ver el resultado.