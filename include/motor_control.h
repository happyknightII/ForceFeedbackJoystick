#pragma once

void init_motor_pwm();
void set_pwm_duty(float u, float v, float w);
void init_current_sensing();
float read_current_u();
float read_current_v();
float read_current_w();