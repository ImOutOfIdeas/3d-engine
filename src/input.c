#include "input.h"
#include <stddef.h>

// Key-flag binding table
static const struct {
    sapp_keycode key;
    uint32_t     flag;
} bindings[] = {
    { SAPP_KEYCODE_W, MOVE_FORWARD },
    { SAPP_KEYCODE_S, MOVE_BACK    },
    { SAPP_KEYCODE_A, MOVE_LEFT    },
    { SAPP_KEYCODE_D, MOVE_RIGHT   },
};
#define NUM_BINDINGS (sizeof(bindings) / sizeof(bindings[0]))

static void set_flag(InputState *input, sapp_keycode key, bool pressed) {
    for (size_t i = 0; i < NUM_BINDINGS; i++) {
        if (bindings[i].key == key) {
            if (pressed) input->move |=  bindings[i].flag;
            else         input->move &= ~bindings[i].flag;
            return;
        }
    }
}

void input_key_down(InputState *input, sapp_keycode key) {
    set_flag(input, key, true);
}

void input_key_up(InputState *input, sapp_keycode key) {
    set_flag(input, key, false);
}

void input_mouse_move(InputState *input, float dx, float dy) {
    // Accumulate rather than replace — multiple events can fire per frame
    input->mouse_dx += dx;
    input->mouse_dy += dy;
}

void input_mouse_lock(InputState *input) {
    input->mouse_locked = true;
}

void input_mouse_unlock(InputState *input) {
    input->mouse_locked  = false;
    // Clear any queued delta so the camera doesn't lurch on re-lock
    input->mouse_dx = 0.0f;
    input->mouse_dy = 0.0f;
}

void input_end_frame(InputState *input) {
    input->mouse_dx = 0.0f;
    input->mouse_dy = 0.0f;
}
