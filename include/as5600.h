#pragma once 
#include <stdint.h>

#include <stdint.h>

void as5600_init(void);
uint16_t as5600_read_raw_angle(void);
float as5600_read_angle_deg(void);
float compute_velocity(float current_angle_rad);