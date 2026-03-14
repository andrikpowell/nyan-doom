//
// Copyright(C) 2024 by Andrik Powell
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// DESCRIPTION:
//	DSDA Status HUD Component
//

#include "base.h"

#include "status.h"

#define PATCH_SIZE 8
#define PATCH_SPACING 2
#define PATCH_VERTICAL_SPACING 2
#define NOT_BLINKING 0
#define BLINKING 1

typedef struct {
    dsda_patch_component_t component;
    dboolean vertical;
    dboolean reverse;
} local_component_t;

static local_component_t* local;

static int patch_vertical_spacing;
static int patch_spacing;

const char* drawArmorName(player_t* player) {
    if (player->armortype < 2) { return "STFPARM1"; }
    else { return "STFPARM2"; }
}

//int getArmorColor(player_t* player) {
//    if (player->armortype >= 2) { return CR_BLUE; }
//    else if (player->armortype == 1) { return CR_GREEN; }
//}

int getBerserkColor(player_t* player) {
    if (unityedition) { return CR_GREEN; }
    else { return CR_RED; }
}

void drawPowerupStatusIcon(player_t* player, int* x, int* y, int powerup, const char* lumpname, int color, int blinking) {
    int flags = local->component.vpt;

    if (W_PWADLumpNameExists2(lumpname))
        color = CR_DEFAULT;

    if (color != CR_DEFAULT)
        flags |= VPT_COLOR;

    if ((!blinking && powerup) || (blinking && (powerup > BLINKTHRESHOLD || (powerup & 8))))
        V_DrawNamePatch(*x, *y, lumpname, color, flags);

    if (local->vertical)
        *y += local->reverse ? -(patch_vertical_spacing + PATCH_SIZE)   : (patch_vertical_spacing + PATCH_SIZE);
    else
        *x += local->reverse ? -(patch_spacing + PATCH_SIZE)            : (patch_spacing + PATCH_SIZE);
}

typedef struct {
  int enabled;
  int powerup;
  const char *lump;
  int color;
  int blinking;
} status_icon_t;

static void dsda_DrawComponent(void) {
    player_t* player;
    int x, y;
    status_icon_t icons[8];
    int n = 0;

    player = &players[displayplayer];

    x = local->component.x;
    y = local->component.y;

    if(raven || !dsda_IntConfig(nyan_config_ex_status_widget))
        return;

    icons[n++] = (status_icon_t){ dsda_IntConfig(nyan_config_ex_status_armor) && player->armortype > 0,
                                  player->armortype > 0, drawArmorName(player), CR_DEFAULT, NOT_BLINKING };

    icons[n++] = (status_icon_t){ dsda_IntConfig(nyan_config_ex_status_berserk) && player->powers[pw_strength],
                                  player->powers[pw_strength], "STFPPSTR", getBerserkColor(player), NOT_BLINKING };

    icons[n++] = (status_icon_t){ dsda_IntConfig(nyan_config_ex_status_areamap) && player->powers[pw_allmap],
                                  player->powers[pw_allmap], "STFPMAP", CR_DEFAULT, NOT_BLINKING };

    icons[n++] = (status_icon_t){ dsda_IntConfig(nyan_config_ex_status_backpack) && player->backpack,
                                  player->backpack, "STFPBPAK", CR_DEFAULT, NOT_BLINKING };

    icons[n++] = (status_icon_t){ dsda_IntConfig(nyan_config_ex_status_radsuit) && player->powers[pw_ironfeet],
                                  player->powers[pw_ironfeet], "STFPSUIT", CR_DEFAULT, BLINKING };

    icons[n++] = (status_icon_t){ dsda_IntConfig(nyan_config_ex_status_invis) && player->powers[pw_invisibility],
                                  player->powers[pw_invisibility], "STFPINS", CR_DEFAULT, BLINKING };

    icons[n++] = (status_icon_t){ dsda_IntConfig(nyan_config_ex_status_liteamp) && player->powers[pw_infrared],
                                  player->powers[pw_infrared], "STFPVIS", CR_DEFAULT, BLINKING };

    icons[n++] = (status_icon_t){ dsda_IntConfig(nyan_config_ex_status_invuln) && player->powers[pw_invulnerability],
                                  player->powers[pw_invulnerability], "STFPINV", CR_DEFAULT, BLINKING };

  if (local->reverse)
  {
    for (int i = n - 1; i >= 0; --i)
      if (icons[i].enabled)
        drawPowerupStatusIcon(player, &x, &y, icons[i].powerup, icons[i].lump, icons[i].color, icons[i].blinking);
  }
  else
  {
    for (int i = 0; i < n; ++i)
      if (icons[i].enabled)
        drawPowerupStatusIcon(player, &x, &y, icons[i].powerup, icons[i].lump, icons[i].color, icons[i].blinking);
  }
}

void dsda_InitStatusHC(int x_offset, int y_offset, int vpt, int* args, int arg_count, void** data) {
    *data = Z_Calloc(1, sizeof(local_component_t));
    local = *data;

    local->vertical = arg_count > 0 ? !!args[0] : false;
    local->reverse  = arg_count > 1 ? !!args[1] : false;

    dsda_InitPatchHC(&local->component, x_offset, y_offset, vpt);
}

void dsda_UpdateStatusHC(void* data) {
    local = data;
}

void dsda_DrawStatusHC(void* data) {
    local = data;

    dsda_DrawComponent();
}
