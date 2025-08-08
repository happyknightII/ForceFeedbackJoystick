#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "as5600.h"
#include "motor_control.h"
#include "foc.h"

void app_main() {
    as5600_init();          // Init I2C and AS5600
    init_motor_pwm();      
    vTaskDelay(pdMS_TO_TICKS(100));  // let peripherals settle
    while (1) {
        float angle_deg = as5600_read_angle_deg();
        if (angle_deg < 0.0f) {
            printf("Angle read error!\n");
            continue;
        }

        float angle_rad = angle_deg * (3.14159265f / 180.0f);
        run_foc_control(angle_rad);

        vTaskDelay(pdMS_TO_TICKS(1));  // ~1 kHz FOC loop
    }
}
