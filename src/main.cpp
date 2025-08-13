#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <string>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_simplefoc.h"

BLDCMotor motor = BLDCMotor(7, 5.6f, 260.0f*1.1f, 0.01f);
BLDCDriver3PWM driver = BLDCDriver3PWM(4, 5, 6);
AS5600 as5600 = AS5600(I2C_NUM_0, GPIO_NUM_1, GPIO_NUM_2);
const char motor_id = 'M';

Commander commander = Commander(Serial);
void doMotor(char* cmd){commander.motor(&motor, cmd);}

extern "C" void app_main(void)
{
    SimpleFOCDebug::enable();
    Serial.begin(115200);

    as5600.init();
    motor.linkSensor(&as5600);

    driver.pwm_frequency = 20000;
    driver.voltage_power_supply = 20;
    driver.voltage_limit = 4;
    driver.init();
    motor.linkDriver(&driver);

    motor.controller = MotionControlType::velocity;

    motor.PID_velocity.P = 0.03f;
    motor.PID_velocity.I = 0.1f;
    motor.PID_velocity.D = 0.0f;

    motor.PID_current_d.P = 0.001;
    motor.PID_current_q.P = 0.001;

    motor.sensor_direction = Direction::CCW;
    motor.voltage_sensor_align = 2;
    motor.LPF_velocity.Tf = 0.9;
    motor.current_limit = 1.5;
    motor.zero_electric_angle = 6.16967;
    motor.velocity_limit = 45;
    motor.sensor_offset = 0;

    motor.torque_controller = TorqueControlType::voltage;
    motor.foc_modulation = FOCModulationType::SpaceVectorPWM;
    motor.modulation_centered = 1;

    commander.add(motor_id, doMotor, "motor");
    
    motor.useMonitoring(Serial);
    
    motor.monitor_variables =
        _MON_TARGET
        | _MON_VOLT_Q
        | _MON_VOLT_D
        | _MON_CURR_Q
        | _MON_CURR_D
        | _MON_VEL
        | _MON_ANGLE;

    // motor.monitor_start_char = motor_id;
    // motor.monitor_end_char = motor_id;
    

    commander.verbose = VerboseMode::machine_readable;

    motor.init();
    motor.initFOC();
    
    while (1) {
        motor.loopFOC();
        motor.move();
        motor.monitor(); 
        commander.run();
        vTaskDelay(1);
    }
}
