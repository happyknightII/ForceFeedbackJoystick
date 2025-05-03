#include "driver/mcpwm_cap.h"
#include "freertos/portmacro.h"

struct CaptureData {
    mcpwm_capture_event_data_t event_data;
    uint32_t timestamp;
};

class Encoder {
public:
    Encoder(int pin, int offset = 0, int group = 0);
    void init();
    float getAngle();
    int getRawPosition();

private:
    static bool capture_callback(mcpwm_cap_channel_handle_t chan,
                                 const mcpwm_capture_event_data_t *edata,
                                 void *user_data);

    int encoderPin;
    int offset;
    int group_id;
    bool cycle_valid = false;

    mcpwm_cap_timer_handle_t cap_timer = nullptr;
    mcpwm_cap_channel_handle_t cap_channel = nullptr;

    volatile uint32_t last_rising_edge = 0;
    volatile uint32_t prev_rising_edge = 0;
    volatile uint32_t last_falling_edge = 0;
    // volatile uint32_t period = 1;
    volatile uint32_t high_time = 1;
    volatile uint32_t low_time = 1;

    portMUX_TYPE spinlock = portMUX_INITIALIZER_UNLOCKED;
};
