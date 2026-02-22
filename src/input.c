#include "input.h"
#include "sokol_app.h"
#include <stddef.h>

static const struct {
    int      key_code;
    uint32_t flag;
} bindings[] = {
    { SAPP_KEYCODE_W, MOVE_FORWARD },
    { SAPP_KEYCODE_S, MOVE_BACK    },
    { SAPP_KEYCODE_A, MOVE_LEFT    },
    { SAPP_KEYCODE_D, MOVE_RIGHT   },
};
#define NUM_BINDINGS (sizeof(bindings) / sizeof(bindings[0]))

static void set_flag(InputState *input, int key_code, bool pressed) {
    for (size_t i = 0; i < NUM_BINDINGS; i++) {
        if (bindings[i].key_code == key_code) {
            if (pressed) input->move |=  bindings[i].flag;
            else         input->move &= ~bindings[i].flag;
            return;
        }
    }
}

void input_key_down(InputState *input, int key_code) { set_flag(input, key_code, true);  }
void input_key_up  (InputState *input, int key_code) { set_flag(input, key_code, false); }
void input_mouse_down  (InputState *input) { input->mouse_locked = true;  }
void input_mouse_unlock(InputState *input) { input->mouse_locked = false; }
