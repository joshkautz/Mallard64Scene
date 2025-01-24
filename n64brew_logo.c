#include <libdragon.h>

// Easing function for quadratic ease-out
//  t = current time
//  b = start value
//  c = change in value
//  d = duration
static float ease_out_quad(float t, float b, float c, float d)
{
    t /= d;
    if (t>1) t = 1;
    return -c * t*(t-2) + b;
}

void n64brew_logo(void)
{
    const color_t VAN_DYKE = RGBA32(0x48, 0x3C, 0x3F, 0xFF);

    display_init(RESOLUTION_640x480, DEPTH_16_BPP, 3, GAMMA_NONE, FILTERS_DISABLED);
    t3d_init((T3DInitParams){});
    T3DModel *brew = t3d_model_load("rom:/brew_logo.t3dm");
    sprite_t *logo = sprite_load("rom:/brew_logo.sprite");

    T3DViewport viewport = t3d_viewport_create();
    T3DVec3 camPos = {{0, 0.0f, 50.0f}};
    T3DVec3 camTarget = {{0, 0.0f, 0}};
    uint8_t colorAmbient[4] = {0xff, 0xff, 0xff, 0xFF};
    float scale = 0.08f;

    T3DMat4FP *mtx = malloc_uncached(sizeof(T3DMat4FP));
    rspq_syncpoint_t sync = 0;

    float mt0 = get_ticks_ms();
    float angle = T3D_DEG_TO_RAD(-90.0f);
    float fade_white = 0.0f;
    int anim_part = 0;
    while (1)
    {
        float tt = get_ticks_ms() - mt0;
        if (tt < 1500) {
            anim_part = 0;
            angle += 0.015f;
        } else if (tt < 3500) {
            anim_part = 1;
            tt -= 1500;
            camTarget.v[0] = ease_out_quad(tt, 0.0f, -125.0f, 2000.0f);
            camTarget.v[1] = ease_out_quad(tt, 0.0f, -5.0f, 2000.0f);
            camPos.v[1] = ease_out_quad(tt, 0.0f, -5.0f, 2000.0f);
            camPos.v[2] = ease_out_quad(tt, 50.0f, 110.0f, 2000.0f);
            angle += 0.015f;
        } else if (tt < 3800) {
            anim_part = 2;
            fade_white = (tt-3500) / 300.0f;
            if (fade_white > 1.0f) fade_white = 1.0f;
            angle += 0.015f;
        } else if (tt < 6500) {
            anim_part = 3;
            fade_white = 1.0f - (tt-3800) / 300.0f;
            if (fade_white < 0.0f) fade_white = 0.0f;
        } else if (tt < 7500) {
            anim_part = 4;
            fade_white = (tt-6500) / 1000.0f;
        } else {
            break;
        }
        
        rdpq_attach(display_get(), display_get_zbuf());
        rdpq_clear(VAN_DYKE);
        rdpq_clear_z(ZBUF_MAX);

        if (anim_part >= 3) {
            rdpq_set_mode_copy(true);
            rdpq_sprite_blit(logo, 55, 156, NULL);
        }

        if (anim_part < 3) {
            t3d_frame_start();
            t3d_viewport_set_projection(&viewport, T3D_DEG_TO_RAD(85.0f), 4.0f, 160.0f);
            t3d_viewport_look_at(&viewport, &camPos, &camTarget, &(T3DVec3){{0,1,0}});
            t3d_viewport_attach(&viewport);
            t3d_light_set_ambient(colorAmbient);
            t3d_light_set_count(0);
            
            if (sync) rspq_syncpoint_wait(sync);
            t3d_mat4fp_from_srt_euler(mtx,
                (float[3]){scale, scale, scale},
                (float[3]){0.0f, angle*0.8f, 0},
                (float[3]){0, 0, 0}
            );
            t3d_matrix_push(mtx);
            t3d_model_draw(brew);
            t3d_matrix_pop(1);
            sync = rspq_syncpoint_new();
            rdpq_sync_pipe();
        }
        
        if (anim_part >= 2 && fade_white > 0.0f) {
            rdpq_set_mode_standard();
            if (anim_part == 4)
                rdpq_set_prim_color(RGBA32(0,0,0,255*fade_white));
            else
                rdpq_set_prim_color(RGBA32(255,255,255,255*fade_white));
            rdpq_mode_combiner(RDPQ_COMBINER_FLAT);
            rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY);
            rdpq_fill_rectangle(0, 0, 640, 480);
        }

        rdpq_detach_show();
    }

    wait_ms(1000);
    rspq_wait();
    t3d_model_free(brew);
    t3d_destroy();
    display_close();
}