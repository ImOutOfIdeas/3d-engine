#include "camera.h"
#include "input.h"
#include <math.h>

HMM_Mat4 camera_view(Camera *cam) {
    HMM_Vec3 forward = {{
        cosf(cam->pitch) * sinf(cam->yaw),
        sinf(cam->pitch),
        cosf(cam->pitch) * cosf(cam->yaw),
    }};
    HMM_Vec3 target = HMM_AddV3(cam->position, forward);
    return HMM_LookAt_RH(cam->position, target, HMM_V3(0.0f, 1.0f, 0.0f));
}

void camera_move(Camera *cam, uint32_t flags, float dt) {
    HMM_Vec3 forward = HMM_V3( sinf(cam->yaw), 0.0f,  cosf(cam->yaw));
    HMM_Vec3 right   = HMM_V3(-cosf(cam->yaw), 0.0f,  sinf(cam->yaw));
    float    speed   = MOVE_SPEED * dt;

    if (flags & MOVE_FORWARD) cam->position = HMM_AddV3(cam->position, HMM_MulV3F(forward,  speed));
    if (flags & MOVE_BACK)    cam->position = HMM_AddV3(cam->position, HMM_MulV3F(forward, -speed));
    if (flags & MOVE_RIGHT)   cam->position = HMM_AddV3(cam->position, HMM_MulV3F(right,    speed));
    if (flags & MOVE_LEFT)    cam->position = HMM_AddV3(cam->position, HMM_MulV3F(right,   -speed));
}

void camera_look(Camera *cam, float dx, float dy) {
    cam->yaw   -= dx * LOOK_SPEED;
    cam->pitch -= dy * LOOK_SPEED;

    if (cam->pitch >  PITCH_MAX) cam->pitch =  PITCH_MAX;
    if (cam->pitch < -PITCH_MAX) cam->pitch = -PITCH_MAX;
}
