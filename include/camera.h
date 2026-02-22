#ifndef CAMERA_H
#define CAMERA_H

#include "HandmadeMath.h"
#include <stdint.h>

#define FOV        (HMM_PI / 4.0f)
#define MOVE_SPEED 4.0f
#define LOOK_SPEED 0.002f
#define PITCH_MAX  1.5f

typedef struct {
    HMM_Vec3 position;
    float    yaw;
    float    pitch;
} Camera;

HMM_Mat4 camera_view(Camera *cam);
void     camera_move(Camera *cam, uint32_t move_flags, float dt);
void     camera_look(Camera *cam, float dx, float dy);

#endif // CAMERA_H
