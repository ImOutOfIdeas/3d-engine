#define SOKOL_IMPL 
#define SOKOL_GLCORE
#define STB_IMAGE_IMPLEMENTATION

#include "HandmadeMath.h"
#include "sokol_app.h"
#include "sokol_gfx.h"
#include "sokol_log.h"
#include "sokol_glue.h"
#include "stb_image.h"
#include "pyramid.glsl.h"

#define FOV         HMM_PI / 4.0f
#define MOVE_SPEED  4.0f
#define LOOK_SPEED  0.002f  // radians per pixel

typedef struct {
    HMM_Vec3 position;
    float yaw;   
    float pitch;  
} Camera;

// Build a view matrix from the camera's position and angles
static HMM_Mat4 camera_view(Camera *cam) {
    HMM_Vec3 forward = {{
        cosf(cam->pitch) * sinf(cam->yaw),
        sinf(cam->pitch),
        cosf(cam->pitch) * cosf(cam->yaw),
    }};
    HMM_Vec3 target = HMM_AddV3(cam->position, forward);
    return HMM_LookAt_RH(cam->position, target, HMM_V3(0.0f, 1.0f, 0.0f));
}

static struct {
    sg_pipeline pip;
    sg_bindings bind;
    sg_pass_action pass_action;
    sg_image img;
    sg_view img_view;
    sg_sampler smp;

    Camera camera;

    bool key_w, key_s, key_a, key_d;
    bool mouse_locked;
} state;

static void init(void) {
    sg_setup(&(sg_desc){
        .environment = sglue_environment(),
        .logger.func = slog_func,
    });

    float vertices[] = {
        // position           // uv
        // Front face 
         0.0f,  0.5f,  0.0f,  0.5f, 0.5f,
        -0.5f, -0.5f,  0.5f,  0.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 1.0f,
        // Right face
         0.0f,  0.5f,  0.0f,   0.5f, 0.5f,
         0.5f, -0.5f,  0.5f,   1.0f, 1.0f,
         0.5f, -0.5f, -0.5f,   1.0f, 0.0f,
        // Back face
         0.0f,  0.5f,  0.0f,   0.5f, 0.5f,
         0.5f, -0.5f, -0.5f,   1.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,   0.0f, 0.0f,
        // Left face
         0.0f,  0.5f,  0.0f,   0.5f, 0.5f,
        -0.5f, -0.5f, -0.5f,   0.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,   0.0f, 1.0f,
    };
    state.bind.vertex_buffers[0] = sg_make_buffer(&(sg_buffer_desc){
        .data = SG_RANGE(vertices),
        .label = "pyramid-vertices"
    });

    int width, height, channels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* pixels = stbi_load("data/textures/obamna.png", &width, &height, &channels, 4);
    state.img = sg_make_image(&(sg_image_desc){
        .width = width,
        .height = height,
        .data.mip_levels[0] = { .ptr = pixels, .size = (size_t)(width * height * 4) },
        .label = "pyramid texture"
    });
    stbi_image_free(pixels);

    state.img_view = sg_make_view(&(sg_view_desc){
        .texture.image = state.img,
        .label = "pyramid-texture-view",
    });
    state.smp = sg_make_sampler(&(sg_sampler_desc){
        .min_filter = SG_FILTER_LINEAR,
        .mag_filter = SG_FILTER_LINEAR,
        .wrap_u = SG_WRAP_REPEAT,
        .wrap_v = SG_WRAP_REPEAT,
        .label = "pyramid-sampler",
    });

    state.bind.views[VIEW_tex] = state.img_view;
    state.bind.samplers[SMP_smp] = state.smp;

    state.pip = sg_make_pipeline(&(sg_pipeline_desc){
        .shader = sg_make_shader(pyramid_shader_desc(sg_query_backend())),
        .layout = {
            .attrs = {
                [ATTR_pyramid_position].format = SG_VERTEXFORMAT_FLOAT3,
                [ATTR_pyramid_texcoord].format = SG_VERTEXFORMAT_FLOAT2,
            }
        },
        .depth = {
            .compare = SG_COMPAREFUNC_LESS_EQUAL,
            .write_enabled = true,
        },
        .cull_mode = SG_CULLMODE_BACK,
        .face_winding = SG_FACEWINDING_CCW,
        .label = "pyramid-pipeline"
    });

    state.pass_action = (sg_pass_action) {
        .colors[0] = { .load_action = SG_LOADACTION_CLEAR, .clear_value = {0.1f, 0.1f, 0.1f, 1.0f} },
        .depth = { .load_action = SG_LOADACTION_CLEAR, .clear_value = 1.0f },
    };

    state.camera.position = HMM_V3(0.0f, 1.0f, 3.0f);
    state.camera.yaw   = HMM_PI;   // facing toward origin (-Z)
    state.camera.pitch = 0.0f;
}

void frame(void) {
    float dt = (float)sapp_frame_duration();

    // Build movement vectors from yaw 
    HMM_Vec3 forward = HMM_V3(sinf(state.camera.yaw), 0.0f, cosf(state.camera.yaw));
    HMM_Vec3 right   = HMM_V3(-cosf(state.camera.yaw), 0.0f, sinf(state.camera.yaw));

    if (state.key_w) state.camera.position = HMM_AddV3(state.camera.position, HMM_MulV3F(forward, MOVE_SPEED * dt));
    if (state.key_s) state.camera.position = HMM_AddV3(state.camera.position, HMM_MulV3F(forward, -MOVE_SPEED * dt));
    if (state.key_d) state.camera.position = HMM_AddV3(state.camera.position, HMM_MulV3F(right,   MOVE_SPEED * dt));
    if (state.key_a) state.camera.position = HMM_AddV3(state.camera.position, HMM_MulV3F(right,  -MOVE_SPEED * dt));

    // Clamp pitch so you can't flip upside down
    if (state.camera.pitch >  1.5f) state.camera.pitch =  1.5f;
    if (state.camera.pitch < -1.5f) state.camera.pitch = -1.5f;

    // Find the Model, View, and Projection matrices
    float aspect = (float)sapp_width() / (float)sapp_height();

    HMM_Mat4 model = HMM_M4D(1.0f); 
    HMM_Mat4 view = camera_view(&state.camera);
    HMM_Mat4 projection = HMM_Perspective_RH_NO(FOV, aspect, 0.1f, 100.0f);


    // Calculate the final MVP
    HMM_Mat4 mvp = HMM_MulM4(HMM_MulM4(projection, view), model);

    vs_params_t vs_params;
    memcpy(vs_params.mvp, mvp.Elements, sizeof(vs_params.mvp));

    sg_begin_pass(&(sg_pass){ .action = state.pass_action, .swapchain = sglue_swapchain() });
    sg_apply_pipeline(state.pip);
    sg_apply_bindings(&state.bind);
    sg_apply_uniforms(UB_vs_params, &SG_RANGE(vs_params));
    sg_draw(0, 12, 1);
    sg_end_pass();
    sg_commit();
}

void event(const sapp_event* e) {
    switch (e->type) {
        // Mouse click locks the cursor for look control
        case SAPP_EVENTTYPE_MOUSE_DOWN:
            sapp_lock_mouse(true);
            state.mouse_locked = true;
            break;

        // Escape unlocks cursor
        case SAPP_EVENTTYPE_KEY_DOWN:
            if (e->key_code == SAPP_KEYCODE_ESCAPE) {
                sapp_lock_mouse(false);
                state.mouse_locked = false;
            }
            if (e->key_code == SAPP_KEYCODE_W) state.key_w = true;
            if (e->key_code == SAPP_KEYCODE_S) state.key_s = true;
            if (e->key_code == SAPP_KEYCODE_A) state.key_a = true;
            if (e->key_code == SAPP_KEYCODE_D) state.key_d = true;
            break;

        case SAPP_EVENTTYPE_KEY_UP:
            if (e->key_code == SAPP_KEYCODE_W) state.key_w = false;
            if (e->key_code == SAPP_KEYCODE_S) state.key_s = false;
            if (e->key_code == SAPP_KEYCODE_A) state.key_a = false;
            if (e->key_code == SAPP_KEYCODE_D) state.key_d = false;
            break;

        case SAPP_EVENTTYPE_MOUSE_MOVE:
            if (state.mouse_locked) {
                state.camera.yaw   -= e->mouse_dx * LOOK_SPEED;
                state.camera.pitch -= e->mouse_dy * LOOK_SPEED;
            }
            break;

        default: break;
    }
}

void cleanup(void) {
    sg_shutdown();
}

sapp_desc sokol_main(int argc, char* argv[]) {
    (void)argc; (void)argv;
    return (sapp_desc){
        .init_cb    = init,
        .frame_cb   = frame,
        .cleanup_cb = cleanup,
        .event_cb   = event,
        .width      = 640,
        .height     = 480,
        .window_title = "3d engine",
        .icon.sokol_default = true,
        .logger.func = slog_func,
    };
}
