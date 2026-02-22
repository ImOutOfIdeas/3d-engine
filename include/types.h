#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>

// Movement direction bitmask, used by input and camera
typedef enum {
    MOVE_FORWARD = 1 << 0,
    MOVE_BACK    = 1 << 1,
    MOVE_LEFT    = 1 << 2,
    MOVE_RIGHT   = 1 << 3,
} MoveFlags;

#endif // TYPES_H
