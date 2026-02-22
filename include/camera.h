#ifndef CAMERA_H
#define CAMERA_H

#include "types.h"
#include "HandmadeMath.h"

// Default values used by camera_init()
#define CAMERA_DEFAULT_FOV        (HMM_PI / 4.0f)
#define CAMERA_DEFAULT_NEAR       0.1f
#define CAMERA_DEFAULT_FAR        100.0f
#define CAMERA_DEFAULT_MOVE_SPEED 4.0f
#define CAMERA_DEFAULT_LOOK_SPEED 0.002f
#define CAMERA_DEFAULT_PITCH_MAX  1.5f

typedef struct {
    // Spatial state
    HMM_Vec3 position;
    float    yaw;
    float    pitch;

    // Per-instance configuration
    float fov;
    float near_plane;
    float far_plane;
    float move_speed;
    float look_speed;
    float pitch_max;
} Camera;

// Initialize a camera with sane defaults at the given position
void     camera_init(Camera *cam, HMM_Vec3 position, float yaw);

// Build the view matrix from camera state
HMM_Mat4 camera_view(const Camera *cam);

// Build the projection matrix. Pass the current framebuffer aspect ratio.
HMM_Mat4 camera_projection(const Camera *cam, float aspect);

// Move relative to the camera's facing direction. flags is a MoveFlags bitmask.
void     camera_move(Camera *cam, MoveFlags move_flags, float dt);

// Rotate the camera by mouse delta (pixels). Call once per frame.
void     camera_look(Camera *cam, float dx, float dy);

#endif // CAMERA_H
