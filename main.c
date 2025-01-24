#include <libdragon.h>
#include <t3d/t3d.h>
#include <t3d/t3dmath.h>
#include <t3d/t3dmodel.h>


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

void libdragon_logo(void)
{
    const color_t RED = RGBA32(221, 46, 26, 255);
    const color_t WHITE = RGBA32(255, 255, 255, 255);

    sprite_t *d1 = sprite_load("rom:/dragon1.sprite");
    sprite_t *d2 = sprite_load("rom:/dragon2.sprite");
    sprite_t *d3 = sprite_load("rom:/dragon3.sprite");
    sprite_t *d4 = sprite_load("rom:/dragon4.sprite");
    wav64_t music;
    wav64_open(&music, "rom:/dragon.wav64");
    mixer_ch_set_limits(0, 0, 48000, 0);

    display_init(RESOLUTION_640x480, DEPTH_16_BPP, 2, GAMMA_NONE, ANTIALIAS_RESAMPLE);

    float angle1=0, angle2=0, angle3=0;
    float scale1=0, scale2=0, scale3=0, scroll4=0;
    uint32_t ms0=0;
    int anim_part=0;
    const int X0 = 10, Y0 = 30; // translation offset of the animation (simplify centering)

    void reset() {
        ms0 = get_ticks_ms();
        anim_part = 0;

        angle1 = 3.2f;
        angle2 = 1.9f;
        angle3 = 0.9f;
        scale1 = 0.0f;
        scale2 = 0.4f;
        scale3 = 0.8f;
        scroll4 = 400;
        wav64_play(&music, 0);
    }

    reset();
    while (1) {
        mixer_try_play();
        
        // Calculate animation part:
        // 0: rotate dragon head
        // 1: rotate dragon body and tail, scale up
        // 2: scroll dragon logo
        // 3: fade out 
        uint32_t tt = get_ticks_ms() - ms0;
        if (tt < 1000) anim_part = 0;
        else if (tt < 1500) anim_part = 1;
        else if (tt < 4000) anim_part = 2;
        else if (tt < 5000) anim_part = 3;
        else break;

        // Update animation parameters using quadratic ease-out
        angle1 -= angle1 * 0.04f; if (angle1 < 0.010f) angle1 = 0;
        if (anim_part >= 1) {
            angle2 -= angle2 * 0.06f; if (angle2 < 0.01f) angle2 = 0;
            angle3 -= angle3 * 0.06f; if (angle3 < 0.01f) angle3 = 0;
            scale2 -= scale2 * 0.06f; if (scale2 < 0.01f) scale2 = 0;
            scale3 -= scale3 * 0.06f; if (scale3 < 0.01f) scale3 = 0;
        }
        if (anim_part >= 2) {
            scroll4 -= scroll4 * 0.08f;
        }

        // Update colors for fade out effect
        color_t red = RED;
        color_t white = WHITE;
        if (anim_part >= 3) {
            red.a = 255 - (tt-4000) * 255 / 1000;
            white.a = 255 - (tt-4000) * 255 / 1000;
        }

        #if 0
        // Debug: re-run logo animation on button press
        joypad_poll();
        joypad_buttons_t btn = joypad_get_buttons_pressed(JOYPAD_PORT_1);
        if (btn.a) reset();
        #endif

        surface_t *fb = display_get();
        rdpq_attach_clear(fb, NULL);

        // To simulate the dragon jumping out, we scissor the head so that
        // it appears as it moves.
        if (angle1 > 1.0f) {
            // Initially, also scissor horizontally, 
            // so that the head tail is not visible on the right.
            rdpq_set_scissor(0, 0, X0+300, Y0+240);    
        } else {
            rdpq_set_scissor(0, 0, 640, Y0+240);
        }

        // Draw dragon head
        rdpq_set_mode_standard();
        rdpq_mode_alphacompare(1);
        rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY);
        rdpq_mode_combiner(RDPQ_COMBINER1((0,0,0,PRIM),(TEX0,0,PRIM,0)));
        rdpq_set_prim_color(red);
        rdpq_sprite_blit(d1, X0+216, Y0+205, &(rdpq_blitparms_t){ 
            .theta = angle1, .scale_x = scale1+1, .scale_y = scale1+1,
            .cx = 176, .cy = 171,
        });

        // Restore scissor to standard
        rdpq_set_scissor(0, 0, 640, 480);

        // Draw a black rectangle with alpha gradient, to cover the head tail
        rdpq_mode_combiner(RDPQ_COMBINER_SHADE);
        rdpq_mode_dithering(DITHER_NOISE_NOISE);
        float vtx[4][6] = {
            //  x,      y,    r,g,b,a
            { X0+0,   Y0+180, 0,0,0,0 },
            { X0+200, Y0+180, 0,0,0,0 },
            { X0+200, Y0+240, 0,0,0,1 },
            { X0+0,   Y0+240, 0,0,0,1 },
        };
        rdpq_triangle(&TRIFMT_SHADE, vtx[0], vtx[1], vtx[2]);
        rdpq_triangle(&TRIFMT_SHADE, vtx[0], vtx[2], vtx[3]);

        if (anim_part >= 1) {
            // Draw dragon body and tail
            rdpq_set_mode_standard();
            rdpq_mode_alphacompare(1);
            rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY);
            rdpq_mode_combiner(RDPQ_COMBINER1((0,0,0,PRIM),(TEX0,0,PRIM,0)));

            // Fade them in
            color_t color = red;
            color.r *= 1-scale3; color.g *= 1-scale3; color.b *= 1-scale3;
            rdpq_set_prim_color(color);

            rdpq_sprite_blit(d2, X0+246, Y0+230, &(rdpq_blitparms_t){ 
                .theta = angle2, .scale_x = 1-scale2, .scale_y = 1-scale2,
                .cx = 145, .cy = 113,
            });

            rdpq_sprite_blit(d3, X0+266, Y0+256, &(rdpq_blitparms_t){ 
                .theta = -angle3, .scale_x = 1-scale3, .scale_y = 1-scale3,
                .cx = 91, .cy = 24,
            });
        }

        // Draw scrolling logo
        if (anim_part >= 2) {
            rdpq_set_prim_color(white);
            rdpq_sprite_blit(d4, X0 + 161 + (int)scroll4, Y0 + 182, NULL);
        }

        rdpq_detach_show();
    }

    wait_ms(500); // avoid immediate switch to next screen
    rspq_wait();
    sprite_free(d1);
    sprite_free(d2);
    sprite_free(d3);
    sprite_free(d4);
    wav64_close(&music);
    display_close();
}



typedef struct {
  color_t color;
  T3DVec3 dir;
} DirLight;

xm64player_t player;

inline int randr(int min, int max){
  int range = min - max;
  return (rand() % range) + min;
}

int main()
{
	debug_init_isviewer();
	debug_init_usblog();
	asset_init_compression(2);

  dfs_init(DFS_DEFAULT_LOCATION);
  rdpq_init();
  audio_init(32000, 6);
  mixer_init(32);
  joypad_init();

  libdragon_logo();
  n64brew_logo();

  display_init((resolution_t){.width = 480, .height = 320, .interlaced = true}, DEPTH_16_BPP, 3, GAMMA_NONE, FILTERS_RESAMPLE_ANTIALIAS_DEDITHER);
  t3d_init((T3DInitParams){});
  T3DViewport viewport = t3d_viewport_create();
  timer_init();
  // Now allocate a fixed-point matrix, this is what t3d uses internally.
  T3DMat4FP* modelMatFP_real;
  T3DMat4FP* modelMat_mugFP_real;
  T3DMat4FP* modelMatFP = malloc_uncached(sizeof(T3DMat4FP));
  T3DMat4FP* modelMat_mugFP = malloc_uncached(sizeof(T3DMat4FP));
  T3DMat4FP* modelMatFP2 = malloc_uncached(sizeof(T3DMat4FP));
  T3DMat4FP* modelMat_mugFP2 = malloc_uncached(sizeof(T3DMat4FP));
  T3DMat4FP* modelMatFP3 = malloc_uncached(sizeof(T3DMat4FP));
  T3DMat4FP* modelMat_mugFP3 = malloc_uncached(sizeof(T3DMat4FP));
  T3DMat4FP* modelMat_t3dlogoFP = malloc_uncached(sizeof(T3DMat4FP));
  T3DMat4FP* lightMatFP = malloc_uncached(sizeof(T3DMat4FP) * 4);

  xm64player_open(&player, "rom:/right_on_time.xm64");
  xm64player_set_loop(&player, true);
  xm64player_play(&player, 2);

  T3DVec3 camPos = {{60.0f,15.0f,0}};
  T3DVec3 camTarget = {{-90,15,0}};

  DirLight dirLights[4] = {
    {.color = {0xFF, 0x00, 0x00, 0xFF}, .dir = {{ 1.0f,  1.0f, 0.0f}}},
    {.color = {0x00, 0xFF, 0x00, 0xFF}, .dir = {{-1.0f,  1.0f, 0.0f}}},
    {.color = {0x00, 0x00, 0xFF, 0xFF}, .dir = {{ 0.0f, -1.0f, 0.0f}}},
    {.color = {0x50, 0x50, 0x50, 0xFF}, .dir = {{ 0.0f,  0.0f, 1.0f}}}
  };
  uint8_t colorAmbient[4] = {0xFF, 0xFF, 0xFF, 0xFF};

  T3DVec3 lightDirVec = {{1.0f, 1.0f, 0.0f}};
  t3d_vec3_norm(&lightDirVec);

  sprite_t* flamespr = sprite_load("rom:/flame.rgba32.sprite");

  T3DModel *model = t3d_model_load("rom:/brewlogo_282.t3dm");
  T3DModel *modelflame = t3d_model_load("rom:/brewlogo_flame_282.t3dm");
  T3DModel *modelmug = t3d_model_load("rom:/brewlogo_mug_282.t3dm");
  T3DModel *modelt3d = t3d_model_load("rom:/brewlogo_t3dlogo_282.t3dm");
  T3DMaterial* mat = t3d_model_get_material(model, "f3d_material_lights");
  if(mat){
    mat->blendMode = RDPQ_BLENDER_ADDITIVE;
    mat->otherModeMask |= SOM_Z_WRITE;
    mat->otherModeValue &= !SOM_Z_WRITE;
    mat->otherModeMask |= SOM_SAMPLE_BILINEAR;
    mat->otherModeValue |= SOM_SAMPLE_BILINEAR;
  }
  mat = t3d_model_get_material(model, "f3d_material_pinetree");
  if(mat){
    mat->otherModeMask |= SOM_Z_WRITE;
    mat->otherModeValue &= !SOM_Z_WRITE;
  }
  T3DModelIter it = t3d_model_iter_create(model, T3D_CHUNK_TYPE_MATERIAL);
  while(t3d_model_iter_next(&it))
  {
    it.material->otherModeMask |= SOM_ZMODE_INTERPENETRATING;
    it.material->otherModeValue |= SOM_ZMODE_INTERPENETRATING;
  }
  it = t3d_model_iter_create(modelmug, T3D_CHUNK_TYPE_MATERIAL);
  while(t3d_model_iter_next(&it))
  {
    it.material->otherModeMask |= SOM_ZMODE_INTERPENETRATING;
    it.material->otherModeValue |= SOM_ZMODE_INTERPENETRATING;
  }

  rspq_block_begin();
  t3d_model_draw(modelmug);
  t3d_matrix_pop(1);
  t3d_matrix_push(modelMat_t3dlogoFP);
  t3d_model_draw(modelt3d);
  t3d_matrix_pop(1);
  t3d_model_draw(model);
  t3d_matrix_pop(1);
  rspq_block_t *dplModel = rspq_block_end();

  float rotAngle = 0.0f;
  float lightCountTimer = 0.5f;
  int flameframe = 0;
  double time = TICKS_TO_MS(timer_ticks()) - 13000;
  double timedynamic = TICKS_TO_MS(timer_ticks()) - 13000;
  int frozen = 0;
  int mtxframe = 0;

  for(;;)
  {
    // ======== Draw ======== //
    rdpq_attach(display_get(), display_get_zbuf());
    t3d_frame_start(); // call this once per frame at the beginning of your draw function
    t3d_viewport_attach(&viewport);
    t3d_screen_clear_depth();
    float deltatime = display_get_delta_time() * 1000.0f;

    mtxframe++;
    if(mtxframe > 2) mtxframe = 0;
    switch(mtxframe){
      case 0:
      {
        modelMatFP_real = modelMatFP;
        modelMat_mugFP_real = modelMat_mugFP;
        break;
      }
      case 1:
      {
        modelMatFP_real = modelMatFP2;
        modelMat_mugFP_real = modelMat_mugFP2;
        break;
      }
      case 2:
      {
        modelMatFP_real = modelMatFP3;
        modelMat_mugFP_real = modelMat_mugFP3;
        break;
      }
    }

    joypad_poll();
    joypad_buttons_t buttons = joypad_get_buttons_pressed(JOYPAD_PORT_1);
    if(buttons.a) frozen++;
    if(frozen > 3) frozen = 0;

    if(frozen == 1)  time = 45000;
    else if(frozen == 2)  time = 80000;
    else if(frozen == 3)  time = 5000;
    else time += deltatime;
    timedynamic += deltatime;

    mixer_try_play();
    // ======== Update ======== //
    rotAngle = timedynamic / 600.0f;
    float modelScale = 0.08f;
    lightCountTimer += 0.003f;
    camTarget.y = sin(timedynamic / 4000.0f)*5+15;
    camPos.x = cos(-time / 20000.0f)*40;
    camPos.z = sin(-time / 20000.0f)*40;
    flameframe++;
    if(flameframe > 48) flameframe = 0;

    t3d_viewport_set_projection(&viewport, T3D_DEG_TO_RAD(75.0f), 6.0f, 80.0f);
    t3d_viewport_look_at(&viewport, &camPos, &camTarget, &(T3DVec3){{0,1,0}});

    // Model Matrix
    t3d_mat4fp_from_srt_euler(modelMatFP_real,
      (float[3]){modelScale, modelScale, modelScale},
      (float[3]){0.0f, 0*0.2f, 0},
      (float[3]){0,0,0}
    );
    t3d_mat4fp_from_srt_euler(modelMat_mugFP_real,
      (float[3]){1, 1, 1},
      (float[3]){0.0f, rotAngle*0.2f, 0},
      (float[3]){0,0,0}
    );
    t3d_mat4fp_from_srt_euler(modelMat_t3dlogoFP,
      (float[3]){1, 1, 1},
      (float[3]){0.0f, rotAngle*0.2f, 0},
      (float[3]){-12.7477 * 64, -20, 8.54531 * 64}
    );
    mixer_try_play();
    // ======== Update ======== //
    rotAngle = timedynamic / 600.0f;
    modelScale = 0.08f;
    lightCountTimer += 0.003f;
    camTarget.y = sin(timedynamic / 4000.0f)*5+15;
    camPos.x = cos(-time / 20000.0f)*40;
    camPos.z = sin(-time / 20000.0f)*40;

    t3d_viewport_set_projection(&viewport, T3D_DEG_TO_RAD(75.0f), 6.0f, 80.0f);
    t3d_viewport_look_at(&viewport, &camPos, &camTarget, &(T3DVec3){{0,1,0}});

    // Model Matrix
    t3d_mat4fp_from_srt_euler(modelMatFP_real,
      (float[3]){modelScale, modelScale, modelScale},
      (float[3]){0.0f, 0*0.2f, 0},
      (float[3]){0,0,0}
    );
    t3d_mat4fp_from_srt_euler(modelMat_mugFP_real,
      (float[3]){1, 1, 1},
      (float[3]){0.0f, rotAngle*0.2f, 0},
      (float[3]){0,0,0}
    );
    t3d_mat4fp_from_srt_euler(modelMat_t3dlogoFP,
      (float[3]){1, 1, 1},
      (float[3]){0.0f, rotAngle*0.2f, 0},
      (float[3]){-12.7477 * 64, -20, 8.54531 * 64}
    );

    t3d_light_set_count(0);
    color_t* colamb = (color_t*)colorAmbient;
    *colamb = RGBA32(randr(220, 255), randr(220, 255), randr(220, 255), 0xFF);
    t3d_light_set_ambient(colorAmbient);
    rdpq_sync_pipe(); // Hardware crashes otherwise
    rdpq_sync_tile(); // Hardware crashes otherwise
    rdpq_mode_antialias(AA_STANDARD);

    t3d_matrix_push_pos(1);
    t3d_matrix_pop(1);
    t3d_matrix_push(modelMatFP_real);
    t3d_matrix_push(modelMat_mugFP_real);
    rspq_block_run(dplModel);

    t3d_matrix_push(modelMatFP_real);
    T3DObject* obj = t3d_model_get_object_by_index(modelflame, 0);
    t3d_model_draw_material(obj->material, NULL);
    rdpq_mode_mipmap(MIPMAP_NONE, 1);
    surface_t flame = sprite_get_pixels(flamespr);
    surface_t flamesingle = surface_make_sub(&flame, (flameframe % 7) * 32, (flameframe / 7) * 32, 32,32);
    rdpq_tex_upload(TILE0, &flamesingle, NULL);

    t3d_model_draw_object(obj, NULL);
    t3d_matrix_pop(1);

    mixer_try_play();

    mixer_try_play();
    // ======== Update ======== //
    rotAngle = timedynamic / 600.0f;
    modelScale = 0.08f;
    lightCountTimer += 0.003f;
    camTarget.y = sin(timedynamic / 4000.0f)*5+15;
    camPos.x = cos(-time / 20000.0f)*40;
    camPos.z = sin(-time / 20000.0f)*40;

    t3d_viewport_set_projection(&viewport, T3D_DEG_TO_RAD(75.0f), 6.0f, 80.0f);
    t3d_viewport_look_at(&viewport, &camPos, &camTarget, &(T3DVec3){{0,1,0}});

    // Model Matrix
    t3d_mat4fp_from_srt_euler(modelMatFP_real,
      (float[3]){modelScale, modelScale, modelScale},
      (float[3]){0.0f, 0*0.2f, 0},
      (float[3]){0,0,0}
    );
    t3d_mat4fp_from_srt_euler(modelMat_mugFP_real,
      (float[3]){1, 1, 1},
      (float[3]){0.0f, rotAngle*0.2f, 0},
      (float[3]){0,0,0}
    );
    t3d_mat4fp_from_srt_euler(modelMat_t3dlogoFP,
      (float[3]){1, 1, 1},
      (float[3]){0.0f, rotAngle*0.2f, 0},
      (float[3]){-12.7477 * 64, -20, 8.54531 * 64}
    );

    rdpq_detach_show();

    mixer_try_play();
  }

  t3d_destroy();
  return 0;
}

