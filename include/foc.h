#pragma once

void run_foc_control(float angle_rad);
void set_foc_position_target(float degrees);
void set_foc_velocity_target(float degrees_per_sec);
void set_foc_mode_velocity(void);
void set_foc_mode_position(void);
