// Library Imports
#include <libdragon.h>
#include <t3d/t3d.h>
#include <t3d/t3dmath.h>
#include <t3d/t3dmodel.h>

// Project Imports
#include "n64brew_logo.c"
#include "libdragon_logo.c"
#include "brew_christmas.c"

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
  brew_christmas();

  return 0;
}
