#include <encoder.h>
#include <cmath>
#include "driver/mcpwm.h"
#include "driver/mcpwm_cap.h"
#include "driver/mcpwm_timer.h"
#include "esp_log.h"
#include "esp_attr.h"

class Encoder
{
private:
    static constexpr int ENCODER_RESOLUTION = 4096;
    int encoderPin;
    int offset;
    mcpwm_unit_t mcpwm_unit;
    mcpwm_capture_channel_id_t capture_channel;
    uint32_t last_rising_edge = 0;
    float angle_degrees = 0.0f;

    static std::map<std::pair<mcpwm_unit_t, mcpwm_capture_channel_id_t>, Encoder*> encoder_map;

    // Static ISR callback handling multiple encoders
    static bool IRAM_ATTR capture_callback(mcpwm_unit_t mcpwm_num, mcpwm_capture_channel_id_t cap_channel, const cap_event_data_t *edata, void *user_data)
{
    Encoder* encoder = static_cast<Encoder*>(user_data); // Retrieve instance
    if (encoder) {
        uint32_t current_time = edata->cap_value;
        if (edata->cap_edge == MCPWM_POS_EDGE || edata->cap_edge == MCPWM_NEG_EDGE) {
            encoder->angle_degrees += 360.0f * (current_time - encoder->last_rising_edge) / ENCODER_RESOLUTION;
            encoder->last_rising_edge = current_time;
        }
    }
    return true; // Callback execution succeeded
}



public:
    Encoder(int pin, int offset = 0, mcpwm_unit_t unit = MCPWM_UNIT_0, mcpwm_capture_channel_id_t channel = MCPWM_SELECT_CAP0)
        : encoderPin(pin), offset(offset), mcpwm_unit(unit), capture_channel(channel) 
    {
        encoder_map[{mcpwm_unit, capture_channel}] = this;
    }

    void init()
    {
        mcpwm_capture_config_t capture_config = {
            .cap_edge = MCPWM_BOTH_EDGE,
            .cap_prescale = 0,
            .capture_cb = capture_callback,
            .user_data = this,
        };

        mcpwm_capture_enable_channel(mcpwm_unit, capture_channel, &capture_config);
    }

    float getAngle() { return fmodf(angle_degrees + offset, 360.0f); }

    int getRawPosition() { return static_cast<int>((getAngle() / 360.0f) * ENCODER_RESOLUTION) % ENCODER_RESOLUTION; }
};
