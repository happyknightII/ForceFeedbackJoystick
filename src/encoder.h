#pragma once

#include "driver/mcpwm_.h"
#include "driver/mcpwm_cap.h"
#include "driver/mcpwm_timer.h"
#include "esp_log.h"
#include "esp_attr.h"
#include <map>
#include <cmath>

Encoder::Encoder(int pin, int offset, mcpwm_unit_t unit, mcpwm_capture_channel_id_t channel)
    : encoderPin(pin), offset(offset), mcpwm_unit(unit), capture_channel(channel)
{
}

// Implementations only (NOT the full class definition)
void Encoder::init()
{
    mcpwm_capture_config_t capture_config = {
        .cap_edge = MCPWM_BOTH_EDGE,
        .cap_prescale = 0,
        .capture_cb = capture_callback,
        .user_data = this,
    };

    mcpwm_capture_enable_channel(mcpwm_unit, capture_channel, &capture_config);
}

float Encoder::getAngle()
{
    return fmodf(angle_degrees + offset, 360.0f);
}

int Encoder::getRawPosition()
{
    return static_cast<int>((getAngle() / 360.0f) * ENCODER_RESOLUTION) % ENCODER_RESOLUTION;
}