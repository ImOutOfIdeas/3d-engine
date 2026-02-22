#ifndef INPUT_H
#define INPUT_H

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    MOVE_FORWARD = 1 << 0,
    MOVE_BACK    = 1 << 1,
    MOVE_LEFT    = 1 << 2,
    MOVE_RIGHT   = 1 << 3,
} MoveFlags;

typedef struct {
    uint32_t move;
    bool     mouse_locked;
} InputState;

// These are called from main.c's sokol event handler
// key_code matches sokol's sapp_keycode values
void input_key_down(InputState *input, int key_code);
void input_key_up(InputState *input, int key_code);
void input_mouse_down(InputState *input);
void input_mouse_unlock(InputState *input);

#endif // INPUT_H
