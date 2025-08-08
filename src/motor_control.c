#include "motor_control.h"
#include "driver/mcpwm_prelude.h"
#include "esp_log.h"
#include "pin_config.h"
#include <math.h>
#include "driver/adc.h"

#define PWM_FREQ_HZ 20000
#define TAG "motor"

static mcpwm_timer_handle_t timers[3];
static mcpwm_oper_handle_t operators[3];
static mcpwm_cmpr_handle_t comparators[3];
static mcpwm_gen_handle_t generators[3];

void init_motor_pwm() {
    for (int i = 0; i < 3; i++) {
        // 1. Create timer
        mcpwm_timer_config_t timer_config = {
            .group_id = 0,
            .clk_src = MCPWM_TIMER_CLK_SRC_DEFAULT,
            .resolution_hz = 1e6,  // 1 MHz resolution (1 tick = 1 µs)
            .count_mode = MCPWM_TIMER_COUNT_MODE_UP,
            .period_ticks = 1e6 / PWM_FREQ_HZ
        };
        ESP_ERROR_CHECK(mcpwm_new_timer(&timer_config, &timers[i]));

        // 2. Create operator
        mcpwm_operator_config_t operator_config = {
            .group_id = 0
        };
        ESP_ERROR_CHECK(mcpwm_new_operator(&operator_config, &operators[i]));
        ESP_ERROR_CHECK(mcpwm_operator_connect_timer(operators[i], timers[i]));

        // 3. Create comparator
        mcpwm_comparator_config_t comparator_config = {
            .flags.update_cmp_on_tez = true
        };
        ESP_ERROR_CHECK(mcpwm_new_comparator(operators[i], &comparator_config, &comparators[i]));

        // 4. Create generator
        int gpio = (i == 0) ? U_PHASE_PWM : (i == 1) ? V_PHASE_PWM : W_PHASE_PWM;
        mcpwm_generator_config_t gen_config = {
            .gen_gpio_num = gpio
        };
        ESP_ERROR_CHECK(mcpwm_new_generator(operators[i], &gen_config, &generators[i]));

        // 5. Configure generator actions
        mcpwm_gen_timer_event_action_t timer_action = {
            .direction = MCPWM_TIMER_DIRECTION_UP,
            .event = MCPWM_TIMER_EVENT_EMPTY,
            .action = MCPWM_GEN_ACTION_HIGH
        };
        ESP_ERROR_CHECK(mcpwm_generator_set_action_on_timer_event(generators[i], timer_action));

        mcpwm_gen_compare_event_action_t compare_action = {
            .comparator = comparators[i],
            .direction = MCPWM_TIMER_DIRECTION_UP,
            .action = MCPWM_GEN_ACTION_LOW
        };
        ESP_ERROR_CHECK(mcpwm_generator_set_action_on_compare_event(generators[i], compare_action));

        // 6. Enable and start timer
        ESP_ERROR_CHECK(mcpwm_timer_enable(timers[i]));
        ESP_ERROR_CHECK(mcpwm_timer_start_stop(timers[i], MCPWM_TIMER_START_NO_STOP));
    }

    set_pwm_duty(50.0f, 50.0f, 50.0f);  // Initial 50% duty
}

void set_pwm_duty(float u, float v, float w) {
    float duties[3] = { u, v, w };
    for (int i = 0; i < 3; i++) {
        uint32_t compare_ticks = (duties[i] / 100.0f) * (1e6 / PWM_FREQ_HZ);
        ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(comparators[i], compare_ticks));
    }
}

void init_current_sensing() {
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(CURRENT_SENSE_U, ADC_ATTEN_DB_11);
    adc1_config_channel_atten(CURRENT_SENSE_V, ADC_ATTEN_DB_11);
    adc1_config_channel_atten(CURRENT_SENSE_W, ADC_ATTEN_DB_11);
}

float read_current_u() {
    int val = adc1_get_raw(CURRENT_SENSE_U);
    return ((float)val / 4095.0f) * 3.3f;
}

float read_current_v() {
    int val = adc1_get_raw(CURRENT_SENSE_V);
    return ((float)val / 4095.0f) * 3.3f;
}

float read_current_w() {
    int val = adc1_get_raw(CURRENT_SENSE_W);
    return ((float)val / 4095.0f) * 3.3f;
}
