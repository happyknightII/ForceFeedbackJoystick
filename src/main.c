#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "as5600.h"
#include "motor_control.h"
#include "foc.h"
#include <math.h>

void app_main() {
    as5600_init();
    init_motor_pwm();
    vTaskDelay(pdMS_TO_TICKS(100));

    set_foc_velocity_target(360*5);  // 180 deg/sec spin
    

    while (1) {
        float angle_deg = as5600_read_angle_deg();
        if (angle_deg < 0.0f) continue;
        float angle_rad = angle_deg * (M_2_PI / 360.0f);
        run_foc_control(angle_rad);
        //printf("%.2f\n", angle_deg);  // CSV: target,actual
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}
