#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "encoder.h" // Include the Encoder class

extern "C" 
{
    void vTaskFunction(void *pvParameters) 
    {
        Encoder encoder(4); // Initialize encoder with GPIO pin 16
        encoder.init(); // Initialize MCPWM capture

        while (true) 
        {
            float angle = encoder.getAngle(); // Read encoder angle
            ESP_LOGI("Encoder Task", "Angle: %.2f degrees", angle); // Print angle to serial

            vTaskDelay(pdMS_TO_TICKS(1000)); // Delay for 1 second
        }
    }

    void app_main(void) 
    {
        ESP_LOGI("main", "Hello, ESP32-S3!");

        xTaskCreate(
            vTaskFunction,   // Task function
            "Encoder Task",  // Name of the task
            4096,            // Stack size
            NULL,            // Parameter
            1,               // Priority
            NULL             // Task handle
        );
    }
}
