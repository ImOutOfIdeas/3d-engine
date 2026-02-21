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

// Configuration
#define ROTATION_SPEED 1.0f
#define CAMERA_DISTANCE 3.0f
#define FOV HMM_PI / 4.0f

// Application state
static struct {
    sg_pipeline pip;
    sg_bindings bind;
    sg_pass_action pass_action;

    sg_image img;
    sg_view img_view;
    sg_sampler smp;

    float rotation_angle;
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

    // Load the obamna texture with stb_image
    int width, height, channels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* pixels = stbi_load("data/textures/obamna.png", &width, &height, &channels, 4);

    state.img = sg_make_image(&(sg_image_desc){
        .width = width,
        .height = height,
        .data.mip_levels[0] = {
            .ptr = pixels,
            .size = (size_t)(width * height * 4),
        },
        .label = "pyramid texture"
    });

    stbi_image_free(pixels);

    // Create a texture view for the image
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

    // clear the color of the frame and the depth buffer
    state.pass_action = (sg_pass_action) {
        .colors[0] = { .load_action=SG_LOADACTION_CLEAR, .clear_value={0.1f, 0.1f, 0.1f, 1.0f } },
        .depth = { .load_action=SG_LOADACTION_CLEAR, .clear_value=1.0f },
    };
}

void frame(void) {
    vs_params_t vs_params;
    float cam_x;
    float cam_z;
    float aspect;
    HMM_Mat4 projection;
    HMM_Mat4 view;

    state.rotation_angle += (float)sapp_frame_duration() * ROTATION_SPEED;
                                                           
    // Projection
    aspect = (float)sapp_width() / (float)sapp_height();
    projection = HMM_Perspective_RH_NO(FOV, aspect, 0.1f, 10.0f);

    // View
    cam_x = cosf(state.rotation_angle) * CAMERA_DISTANCE;
    cam_z = sinf(state.rotation_angle) * CAMERA_DISTANCE;
    view = HMM_LookAt_RH(HMM_V3(cam_x, 1.0f, cam_z), HMM_V3(0.0f,0.0f,0.0f), HMM_V3(0.0f,1.0f,0.0f)); 

    // Combine the view and projection 
    HMM_Mat4 mvp = HMM_MulM4(projection, view);
    
    // Copy the model-ish view projection into vs_params to get passed into the shader as a uniform
    memcpy(vs_params.mvp, mvp.Elements, sizeof(vs_params.mvp));

    sg_begin_pass(&(sg_pass){ .action = state.pass_action, .swapchain = sglue_swapchain() });
    sg_apply_pipeline(state.pip);
    sg_apply_bindings(&state.bind);
    
    // Send the mvp to the shader using the bindslot in the generated file
    sg_apply_uniforms(UB_vs_params, &SG_RANGE(vs_params));
    sg_draw(0, 12, 1);
    sg_end_pass();
    sg_commit();
}

void cleanup(void) {
    sg_shutdown();
}

sapp_desc sokol_main(int argc, char* argv[]) {
    (void)argc; (void)argv;
    return (sapp_desc){
        .init_cb = init,
        .frame_cb = frame,
        .cleanup_cb = cleanup,
        .event_cb = NULL,
        .width = 640,
        .height = 480,
        .window_title = "Obamna",
        .icon.sokol_default = true,
        .logger.func = slog_func,
    };
}
