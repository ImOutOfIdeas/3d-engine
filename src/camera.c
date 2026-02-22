#include "camera.h"

void camera_init(Camera *cam, HMM_Vec3 position, float yaw) {
    cam->position   = position;
    cam->yaw        = yaw;
    cam->pitch      = 0.0f;
    cam->fov        = CAMERA_DEFAULT_FOV;
    cam->near_plane = CAMERA_DEFAULT_NEAR;
    cam->far_plane  = CAMERA_DEFAULT_FAR;
    cam->move_speed = CAMERA_DEFAULT_MOVE_SPEED;
    cam->look_speed = CAMERA_DEFAULT_LOOK_SPEED;
    cam->pitch_max  = CAMERA_DEFAULT_PITCH_MAX;
}

HMM_Mat4 camera_view(const Camera *cam) {
    // Derive forward vector from yaw and pitch
    HMM_Vec3 forward = {{
        cosf(cam->pitch) * sinf(cam->yaw),
        sinf(cam->pitch),
        cosf(cam->pitch) * cosf(cam->yaw),
    }};
    HMM_Vec3 target = HMM_AddV3(cam->position, forward);
    return HMM_LookAt_RH(cam->position, target, HMM_V3(0.0f, 1.0f, 0.0f));
}

HMM_Mat4 camera_projection(const Camera *cam, float aspect) {
    return HMM_Perspective_RH_NO(cam->fov, aspect, cam->near_plane, cam->far_plane);
}

void camera_move(Camera *cam, MoveFlags flags, float dt) {
    // Movement is on the horizontal plane — pitch doesn't affect direction
    HMM_Vec3 forward = HMM_V3( sinf(cam->yaw), 0.0f,  cosf(cam->yaw));
    HMM_Vec3 right   = HMM_V3(-cosf(cam->yaw), 0.0f,  sinf(cam->yaw));
    float    speed   = cam->move_speed * dt;

    if (flags & MOVE_FORWARD) cam->position = HMM_AddV3(cam->position, HMM_MulV3F(forward,  speed));
    if (flags & MOVE_BACK)    cam->position = HMM_AddV3(cam->position, HMM_MulV3F(forward, -speed));
    if (flags & MOVE_RIGHT)   cam->position = HMM_AddV3(cam->position, HMM_MulV3F(right,    speed));
    if (flags & MOVE_LEFT)    cam->position = HMM_AddV3(cam->position, HMM_MulV3F(right,   -speed));
}

void camera_look(Camera *cam, float dx, float dy) {
    cam->yaw   -= dx * cam->look_speed;
    cam->pitch -= dy * cam->look_speed;

    if (cam->pitch >  cam->pitch_max) cam->pitch =  cam->pitch_max;
    if (cam->pitch < -cam->pitch_max) cam->pitch = -cam->pitch_max;
}
