// Library Imports
#include <libdragon.h>
#include <t3d/t3d.h>
#include <t3d/t3dmath.h>
#include <t3d/t3dmodel.h>

// Project Imports
#include "n64brew_logo.c"
#include "libdragon_logo.c"

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

  xm64player_open(&player, "rom:/right_on_time.xm64");
  xm64player_set_loop(&player, true);
  xm64player_play(&player, 2);

  T3DVec3 camPos = {{60.0f,15.0f,0}};
  T3DVec3 camTarget = {{-90,15,0}};

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

