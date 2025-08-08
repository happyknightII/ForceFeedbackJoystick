#include "motor_control.h"
#include <math.h>
#include <stdio.h>

#define TWO_PI 6.28318531f

// Normalize angle between 0 and 2π
static float normalize_angle(float angle) {
    while (angle < 0) angle += TWO_PI;
    while (angle >= TWO_PI) angle -= TWO_PI;
    return angle;
}

void run_foc_control(float angle_rad) {
    angle_rad = normalize_angle(angle_rad);

    // Generate 3-phase sinusoidal duty cycles
    float duty_u = 0.5f + 0.5f * sinf(angle_rad);
    float duty_v = 0.5f + 0.5f * sinf(angle_rad - 2.0f * M_PI / 3.0f);
    float duty_w = 0.5f + 0.5f * sinf(angle_rad - 4.0f * M_PI / 3.0f);

    // Convert to percentage
    duty_u *= 100.0f;
    duty_v *= 100.0f;
    duty_w *= 100.0f;

    // Optional: Debug
    printf("Angle: %.2f rad | PWM: U=%.1f%%, V=%.1f%%, W=%.1f%%\n",
           angle_rad, duty_u, duty_v, duty_w);

    // Apply PWM duties
    set_pwm_duty(duty_u, duty_v, duty_w);
}
