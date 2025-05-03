#include "encoder.h"
#include "esp_log.h"
#include <cmath>
#include <algorithm>

#define TAG "Encoder"
#define ENCODER_RESOLUTION 4096

Encoder::Encoder(int pin, int offset, int group)
    : encoderPin(pin), offset(offset), group_id(group) {}

void Encoder::init()
{
    mcpwm_capture_timer_config_t timer_config = {
        .group_id = group_id,
        .clk_src = MCPWM_CAPTURE_CLK_SRC_APB,
        .resolution_hz = 80'000'000,
        .flags = {.allow_pd = false}
    };
    ESP_ERROR_CHECK(mcpwm_new_capture_timer(&timer_config, &cap_timer));

    mcpwm_capture_channel_config_t ch_config = {
        .gpio_num = encoderPin,
        .intr_priority = 0,
        .prescale = 1,
        .flags = {
            .pos_edge = true,
            .neg_edge = true,
            .pull_up = false,
            .pull_down = true,
            .invert_cap_signal = false,
            .io_loop_back = false
        }
    };
    ESP_ERROR_CHECK(mcpwm_new_capture_channel(cap_timer, &ch_config, &cap_channel));

    mcpwm_capture_event_callbacks_t cbs = {
        .on_cap = capture_callback
    };
    ESP_ERROR_CHECK(mcpwm_capture_channel_register_event_callbacks(cap_channel, &cbs, this));
    ESP_ERROR_CHECK(mcpwm_capture_timer_enable(cap_timer));
    ESP_ERROR_CHECK(mcpwm_capture_channel_enable(cap_channel));
    ESP_ERROR_CHECK(mcpwm_capture_timer_start(cap_timer));
}

bool Encoder::capture_callback(mcpwm_cap_channel_handle_t chan, const mcpwm_capture_event_data_t *edata, void *user_data)
{
    auto* encoder = static_cast<Encoder*>(user_data);
    if (!encoder) return false;

    uint32_t current_time = edata->cap_value;

    portENTER_CRITICAL_ISR(&encoder->spinlock);

    if (edata->cap_edge == MCPWM_CAP_EDGE_POS) {
        encoder->prev_rising_edge = encoder->last_rising_edge;
        encoder->last_rising_edge = current_time;
        encoder->low_time = encoder->last_rising_edge - encoder->prev_rising_edge;
    } else if (edata->cap_edge == MCPWM_CAP_EDGE_NEG) {
        if (encoder->last_rising_edge != 0 && current_time > encoder->last_rising_edge) {
            encoder->last_falling_edge = current_time;
            encoder->high_time = encoder->last_falling_edge - encoder->last_rising_edge;
        }
    }

    portEXIT_CRITICAL_ISR(&encoder->spinlock);

    return true;
}

float Encoder::getAngle()
{
    uint32_t p, h;
    portENTER_CRITICAL(&spinlock);
    p = high_time + low_time;
    h = high_time;
    portEXIT_CRITICAL(&spinlock);

    if (p == 0) return 0.0f;
    ESP_LOGI(TAG, "Period: %u, High Time: %u", (unsigned)p, (unsigned)h);
    float duty = static_cast<float>(h) / static_cast<float>(p);
    // duty = std::clamp(duty, 0.0f, 1.0f);
    float angle = duty * 360.0f;
    return fmodf(angle + offset + 360.0f, 360.0f);
}

int Encoder::getRawPosition()
{
    return static_cast<int>((getAngle() / 360.0f) * ENCODER_RESOLUTION) % ENCODER_RESOLUTION;
}
