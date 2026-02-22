#include "camera.h"
#include "input.h"
#include "HandmadeMath.h"

#define SOKOL_IMPL
#define SOKOL_GLCORE
#define STB_IMAGE_IMPLEMENTATION
#include "sokol_app.h"
#include "sokol_gfx.h"
#include "sokol_log.h"
#include "sokol_glue.h"
#include "stb_image.h"
#include "pyramid.glsl.h"


#include <string.h>

static struct {
    sg_pipeline    pip;
    sg_bindings    bind;
    sg_pass_action pass_action;
    sg_image       img;
    sg_view        img_view;
    sg_sampler     smp;

    Camera     camera;
    InputState input;
} state;

static void init(void) {
    sg_setup(&(sg_desc){
        .environment = sglue_environment(),
        .logger.func = slog_func,
    });

    float vertices[] = {
        // position            // uv
        // Front face
         0.0f,  0.5f,  0.0f,  0.5f, 0.5f,
        -0.5f, -0.5f,  0.5f,  0.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 1.0f,
        // Right face
         0.0f,  0.5f,  0.0f,  0.5f, 0.5f,
         0.5f, -0.5f,  0.5f,  1.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
        // Back face
         0.0f,  0.5f,  0.0f,  0.5f, 0.5f,
         0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
        // Left face
         0.0f,  0.5f,  0.0f,  0.5f, 0.5f,
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 1.0f,
    };
    state.bind.vertex_buffers[0] = sg_make_buffer(&(sg_buffer_desc){
        .data  = SG_RANGE(vertices),
        .label = "pyramid-vertices"
    });

    int width, height, channels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char *pixels = stbi_load("data/textures/obamna.png", &width, &height, &channels, 4);
    state.img = sg_make_image(&(sg_image_desc){
        .width  = width,
        .height = height,
        .data.mip_levels[0] = { .ptr = pixels, .size = (size_t)(width * height * 4) },
        .label  = "pyramid texture"
    });
    stbi_image_free(pixels);

    state.img_view = sg_make_view(&(sg_view_desc){
        .texture.image = state.img,
        .label         = "pyramid-texture-view",
    });
    state.smp = sg_make_sampler(&(sg_sampler_desc){
        .min_filter = SG_FILTER_LINEAR,
        .mag_filter = SG_FILTER_LINEAR,
        .wrap_u     = SG_WRAP_REPEAT,
        .wrap_v     = SG_WRAP_REPEAT,
        .label      = "pyramid-sampler",
    });

    state.bind.views[VIEW_tex]   = state.img_view;
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
            .compare       = SG_COMPAREFUNC_LESS_EQUAL,
            .write_enabled = true,
        },
        .cull_mode    = SG_CULLMODE_BACK,
        .face_winding = SG_FACEWINDING_CCW,
        .label        = "pyramid-pipeline"
    });

    state.pass_action = (sg_pass_action){
        .colors[0] = { .load_action = SG_LOADACTION_CLEAR, .clear_value = {0.1f, 0.1f, 0.1f, 1.0f} },
        .depth     = { .load_action = SG_LOADACTION_CLEAR, .clear_value = 1.0f },
    };

    camera_init(&state.camera, HMM_V3(0.0f, 1.0f, 3.0f), HMM_PI);
}

static void frame(void) {
    float dt     = (float)sapp_frame_duration();
    float aspect = (float)sapp_width() / (float)sapp_height();

    // Update
    camera_move(&state.camera, state.input.move, dt);
    camera_look(&state.camera, state.input.mouse_dx, state.input.mouse_dy);

    // Build MVP — model is identity for now
    HMM_Mat4 proj  = camera_projection(&state.camera, aspect);
    HMM_Mat4 view  = camera_view(&state.camera);
    HMM_Mat4 model = HMM_M4D(1.0f);
    HMM_Mat4 mvp   = HMM_MulM4(HMM_MulM4(proj, view), model);

    vs_params_t vs_params;
    memcpy(vs_params.mvp, mvp.Elements, sizeof(vs_params.mvp));

    // Draw
    sg_begin_pass(&(sg_pass){ .action = state.pass_action, .swapchain = sglue_swapchain() });
    sg_apply_pipeline(state.pip);
    sg_apply_bindings(&state.bind);
    sg_apply_uniforms(UB_vs_params, &SG_RANGE(vs_params));
    sg_draw(0, 12, 1);
    sg_end_pass();
    sg_commit();

    // Must be last — clears per-frame mouse delta
    input_end_frame(&state.input);
}

// translates sokol_events into calls to engine modules.
static void event(const sapp_event *e) {
    switch (e->type) {
        case SAPP_EVENTTYPE_KEY_DOWN:
            if (e->key_code == SAPP_KEYCODE_ESCAPE) {
                sapp_lock_mouse(false);
                input_mouse_unlock(&state.input);
            }
            input_key_down(&state.input, e->key_code);
            break;
        case SAPP_EVENTTYPE_KEY_UP:
            input_key_up(&state.input, e->key_code);
            break;
        case SAPP_EVENTTYPE_MOUSE_DOWN:
            sapp_lock_mouse(true);
            input_mouse_lock(&state.input);
            break;
        case SAPP_EVENTTYPE_MOUSE_MOVE:
            if (state.input.mouse_locked)
                input_mouse_move(&state.input, e->mouse_dx, e->mouse_dy);
            break;
        default: break;
    }
}

static void cleanup(void) {
    sg_shutdown();
}

sapp_desc sokol_main(int argc, char *argv[]) {
    (void)argc; (void)argv;
    return (sapp_desc){
        .init_cb      = init,
        .frame_cb     = frame,
        .cleanup_cb   = cleanup,
        .event_cb     = event,
        .width        = 640,
        .height       = 480,
        .window_title = "3d engine",
        .icon.sokol_default = true,
        .logger.func  = slog_func,
    };
}
