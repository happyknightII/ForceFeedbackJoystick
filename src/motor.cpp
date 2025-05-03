// Create FOC motor class
// This class is used to control a motor using Field Oriented Control (FOC) algorithm.
// It provides methods to initialize the motor, set its speed, and run it in open-loop or closed-loop mode.

#include "motor.h"
#include "driver/gpio.h"

typedef struct motorConfig 
{
    int motorPinS[3]; // Motor phase pins
} motorConfig_t;

class Motor 
{
private:
    motorConfig_t motorConfig; // Motor pins structure

public:
    // Constructor to initialize motor pins
    Motor(motorConfig_t pins) :motorConfig(pins) {};

    void init() 
    {
        // Initialize motor and encoder
        // create gpio pin template
        gpio_config_t phase_config = {
            .pin_bit_mask = (1ULL << motorConfig.motorPinS[0]) | (1ULL << motorConfig.motorPinS[1]) | (1ULL << motorConfig.motorPinS[2]), // Set motor phase pins
            .mode = GPIO_MODE_OUTPUT, // Set as output mode
            .pull_up_en = GPIO_PULLUP_DISABLE, // Disable pull-up
            .pull_down_en = GPIO_PULLDOWN_DISABLE, // Disable pull-down
            .intr_type = GPIO_INTR_DISABLE // Disable interrupts
        };
        gpio_config(&phase_config); // Configure GPIO pins

    }
};