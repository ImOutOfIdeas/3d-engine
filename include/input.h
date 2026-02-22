#ifndef INPUT_H
#define INPUT_H

#include "types.h"
#include <stdbool.h>
#include "sokol_app.h"

typedef struct {
    // Bitmask of currently held movement keys
    MoveFlags move;

    // Accumulated mouse delta for this frame (cleared by input_end_frame)
    float mouse_dx;
    float mouse_dy;

    // Whether the mouse is currently locked (click to lock, Escape to unlock)
    bool mouse_locked;
} InputState;

// Call from sokol's event handler
void input_key_down   (InputState *input, sapp_keycode key);
void input_key_up     (InputState *input, sapp_keycode key);
void input_mouse_move(InputState *input, float dx, float dy);
void input_mouse_lock (InputState *input);
void input_mouse_unlock(InputState *input);

// Call at the END of each frame to clear per-frame deltas
void input_end_frame(InputState *input);

#endif // INPUT_H
