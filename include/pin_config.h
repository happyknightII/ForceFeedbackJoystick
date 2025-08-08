#pragma once

// PWM Output pins
#define U_PHASE_PWM 9
#define V_PHASE_PWM 10
#define W_PHASE_PWM 11

// ADC Channels (GPIOs: 1 → ADC1_CHANNEL_1, etc.)
#define CURRENT_SENSE_U ADC1_CHANNEL_1
#define CURRENT_SENSE_V ADC1_CHANNEL_2
#define CURRENT_SENSE_W ADC1_CHANNEL_3

// AS5600 PWM Input
#define AS5600_PWM_PIN 4