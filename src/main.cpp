#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "driver/uart.h"
#include "esp_simplefoc.h"

static const char MOTOR_ID = 'M';

BLDCMotor motor = BLDCMotor(7, 5.6f, 260.0f*1.1f, 0.01f);
BLDCDriver3PWM driver = BLDCDriver3PWM(4, 5, 6);
AS5600 as5600 = AS5600(I2C_NUM_0, GPIO_NUM_1, GPIO_NUM_2);

Commander commander = Commander(Serial);

// ---------- IPC ----------
typedef struct { char text[48]; } CmdMsg;      // holds "VP0.5", "C2", "", etc.
typedef struct { float target, vq, vd, vel, angle; } Telemetry;

static QueueHandle_t cmdQ   = nullptr;  // comms -> control
static QueueHandle_t telemQ = nullptr;  // control -> comms
static SemaphoreHandle_t serialMux = nullptr; // serialize Serial writes

// Only enqueues! No motor access here.
void doMotor(char* cmd) {
  if (!cmdQ) return;
  CmdMsg m{};
  // Copy the rest of the command (after 'M'); Commander gave us that pointer
  // Ensure NUL-terminated, avoid blocking
  strlcpy(m.text, cmd, sizeof(m.text));
  xQueueSend(cmdQ, &m, 0);
}

// ---------- CONTROL TASK (owns the motor) ----------
static void control_task(void*) {
  for(;;) {
    // Apply any pending command safely here
    CmdMsg m;
    while (xQueueReceive(cmdQ, &m, 0) == pdTRUE) {
      // Run the actual command on the control thread
      // NOTE: This may print via Serial; protect with a mutex
      if (serialMux) xSemaphoreTake(serialMux, portMAX_DELAY);
      commander.motor(&motor, m.text);
      if (serialMux) xSemaphoreGive(serialMux);
    }

    motor.loopFOC();
    motor.move();

    Telemetry t{
      .target = motor.target,
      .vq     = motor.voltage.q,
      .vd     = motor.voltage.d,
      .vel    = motor.shaft_velocity,
      .angle  = motor.shaft_angle
    };
    if (telemQ) xQueueOverwrite(telemQ, &t);

    vTaskDelay(pdMS_TO_TICKS(0.2)); // cooperative
  }
}

// ---------- COMMS TASK (reads serial, prints telemetry) ----------
static void comms_task(void*) {
  const uint32_t downsample = 10; // ~100Hz if ctrl loop ~1kHz
  uint32_t cnt = 0;
  static char line[160];

  for(;;) {
    // This parses bytes and calls doMotor() -> queue. It does NOT touch motor.
    commander.run();

    if (++cnt >= downsample) {
      cnt = 0;
      Telemetry t;
      if (xQueueReceive(telemQ, &t, 0) == pdTRUE) {
        // Tab-separated 7 values to match Studio/web expectations
        // build the line as you already do:
        int n = snprintf(line, sizeof(line),
                        "%.4f\t%.4f\t%.4f\t%.4f\t%.4f\t%.4f\t%.4f\n",
                        t.target, t.vq, t.vd, 0.0f, 0.0f, t.vel, t.angle);
        if (n > 0) {
        Serial.write((const uint8_t*)line, (size_t)n);
        }

      }
    }

    vTaskDelay(pdMS_TO_TICKS(1));
  }
}

extern "C" void app_main(void) {
  // SimpleFOCDebug::enable(); // keep OFF while tuning to reduce UART noise
  Serial.begin(921600);        // faster baud = fewer TX stalls

  // Hardware init
  as5600.init();
  motor.linkSensor(&as5600);
  driver.pwm_frequency = 20000;
  driver.voltage_power_supply = 20;
  driver.voltage_limit = 4;
  driver.init();
  motor.linkDriver(&driver);

  // Motor config
  motor.controller = MotionControlType::velocity;
  motor.PID_velocity.P = 0.03f;
  motor.PID_velocity.I = 0.10f;
  motor.PID_velocity.D = 0.00f;
  motor.PID_current_d.P = 0.001f;
  motor.PID_current_q.P = 0.001f;
  motor.sensor_direction = Direction::CCW;
  motor.voltage_sensor_align = 2;
  motor.LPF_velocity.Tf = 0.9f;
  motor.current_limit = 1.5f;
  motor.zero_electric_angle = 6.16967f;
  motor.velocity_limit = 45.0f;
  motor.sensor_offset = 0;
  motor.torque_controller = TorqueControlType::voltage;
  motor.foc_modulation = FOCModulationType::SpaceVectorPWM;
  motor.modulation_centered = 1;

  // Commander registration
  commander.add(MOTOR_ID, doMotor, (char*)"motor");
  commander.verbose = VerboseMode::machine_readable;

  motor.init();
  motor.initFOC();

  // IPC + sync
  cmdQ      = xQueueCreate(8, sizeof(CmdMsg));
  telemQ    = xQueueCreate(1, sizeof(Telemetry)); // overwrite newest
  serialMux = xSemaphoreCreateMutex();

  // Tasks: control (higher prio) and comms (lower prio)
  xTaskCreatePinnedToCore(control_task, "foc_ctrl", 4096, nullptr, 6, nullptr, 1);
  xTaskCreatePinnedToCore(comms_task,   "foc_comm", 4096, nullptr, 3, nullptr, 0);
}
