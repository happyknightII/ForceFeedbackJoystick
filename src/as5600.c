#include "as5600.h"
#include "driver/i2c.h"
#include "esp_log.h"
#include "esp_timer.h"
#include <math.h>

static float prev_angle_rad = 0.0f;
static uint32_t prev_time_us = 0;

#define I2C_MASTER_SCL_IO 4
#define I2C_MASTER_SDA_IO 5
#define I2C_MASTER_NUM I2C_NUM_0
#define I2C_MASTER_FREQ_HZ 100000
#define I2C_MASTER_TIMEOUT_MS 1000

#define AS5600_ADDR 0x36
#define AS5600_RAW_ANGLE 0x0C

static const char *TAG = "AS5600";

void as5600_init(void) {
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };
    ESP_ERROR_CHECK(i2c_param_config(I2C_MASTER_NUM, &conf));
    ESP_ERROR_CHECK(i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0, 0, 0));
}

uint16_t as5600_read_raw_angle(void) {
    uint8_t data[2] = {0};
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (AS5600_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, AS5600_RAW_ANGLE, true);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (AS5600_ADDR << 1) | I2C_MASTER_READ, true);
    i2c_master_read(cmd, data, 2, I2C_MASTER_LAST_NACK);
    i2c_master_stop(cmd);

    esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, pdMS_TO_TICKS(I2C_MASTER_TIMEOUT_MS));
    i2c_cmd_link_delete(cmd);

    // if (ret != ESP_OK) {
    //     ESP_LOGE(TAG, "I2C read failed: %s", esp_err_to_name(ret));
    //     return 0xFFFF;
    // }

    return ((uint16_t)data[0] << 8) | data[1];
}

float as5600_read_angle_deg(void) {
    uint16_t raw = as5600_read_raw_angle();
    if (raw == 0xFFFF) return -1.0f;
    return (raw * 360.0f) / 4096.0f;
}

float as5600_read_angle_rad(void) {
    return  as5600_read_angle_deg() * (2.0 * M_PI / 360.0f);
}


float compute_velocity(float current_angle_rad) {
    uint32_t now_us = esp_timer_get_time();
    float dt = (now_us - prev_time_us) / 1e6f;  // seconds
    if (dt == 0) return 0;

    float dtheta = current_angle_rad - prev_angle_rad;

    // Handle angle wraparound
    if (dtheta > M_PI) dtheta -= M_2_PI;
    if (dtheta < -M_PI) dtheta += M_2_PI;

    prev_angle_rad = current_angle_rad;
    prev_time_us = now_us;

    return dtheta / dt;  // rad/s
}
