#include "motor_control.h"
#include <math.h>
#include <stdio.h>
#include "esp_timer.h"  // Required for esp_timer_get_time()

#define MAX_DUTY_PERCENT 95.0f
#define POSITION_KP 1.0f
#define POLE_PAIRS 7
#define PHASE_ADVANCE_RAD (M_PI / 2.0f)
#define OFFSET_RAD 0.5583308f

typedef enum {
    FOC_MODE_POSITION,
    FOC_MODE_VELOCITY
} foc_mode_t;

static foc_mode_t foc_mode = FOC_MODE_POSITION;
static float target_angle_rad = 0.0f;
static float target_velocity_rad_per_s = 0.0f;
static float virtual_angle = 0.0f;


static float normalize_angle(float angle) {
    while (angle < 0) angle += M_TWOPI;
    while (angle >= M_TWOPI) angle -= M_TWOPI;
    return angle;
}

static float amplitude_percent = 0.3f;  // 30% default, range: 0.0 to 1.0

void set_foc_amplitude_percent(float percent) {
    if (percent < 0.0f) percent = 0.0f;
    if (percent > 1.0f) percent = 1.0f;
    amplitude_percent = percent;
}

void run_foc_control(float measured_angle_rad) {
    static uint64_t last_us = 0;
    uint64_t now_us = esp_timer_get_time();
    float dt = (now_us - last_us) / 1e6f;
    if (last_us == 0) { last_us = now_us; return; }  // skip first cycle
    last_us = now_us;

    float control_angle;

    if (foc_mode == FOC_MODE_VELOCITY) {
        // Advance virtual rotor angle
        virtual_angle += target_velocity_rad_per_s * dt;
        float electrical_angle = POLE_PAIRS * normalize_angle(virtual_angle);
        control_angle = normalize_angle(electrical_angle + PHASE_ADVANCE_RAD);
    } else {
        // Position mode
        float electrical_angle = POLE_PAIRS * normalize_angle(measured_angle_rad - OFFSET_RAD);
        control_angle = normalize_angle(electrical_angle + PHASE_ADVANCE_RAD);
    }

    // Sinusoidal commutation
    float duty_u = 0.5f + 0.5f * sinf(control_angle) * amplitude_percent;
    float duty_v = 0.5f + 0.5f * sinf(control_angle - M_TWOPI / 3.0f) * amplitude_percent;
    float duty_w = 0.5f + 0.5f * sinf(control_angle - 2.0f * M_TWOPI / 3.0f) * amplitude_percent;

    // Scale
    duty_u *= MAX_DUTY_PERCENT;
    duty_v *= MAX_DUTY_PERCENT;
    duty_w *= MAX_DUTY_PERCENT;

    set_pwm_duty(duty_u, duty_v, duty_w);

    printf("Mode: %s | Ctrl: %.2f | U:%.1f%% V:%.1f%% W:%.1f%%\n",
           foc_mode == FOC_MODE_VELOCITY ? "VEL" : "POS",
           control_angle, duty_u, duty_v, duty_w);
}





void set_foc_position_target(float degrees) {
    target_angle_rad = degrees * (M_TWOPI / 360.0f);
    foc_mode = FOC_MODE_POSITION;
}

void set_foc_velocity_target(float degrees_per_sec) {
    target_velocity_rad_per_s = degrees_per_sec * (M_TWOPI / 360.0f);
    foc_mode = FOC_MODE_VELOCITY;
}

void set_foc_mode_position(void) { foc_mode = FOC_MODE_POSITION; }
void set_foc_mode_velocity(void) { foc_mode = FOC_MODE_VELOCITY; }
