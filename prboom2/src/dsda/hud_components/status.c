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
} local_component_t;

static local_component_t* local;

static int patch_vertical_spacing;
static int patch_spacing;

const char* drawBerserkName() {
    return "STFPPSTR";
}

const char* drawArmorName(player_t* player) {
    if (player->armortype < 2) { return "STFPARM1"; }
    else { return "STFPARM2"; }
}

void drawPowerupStatusIcon(player_t* player, int* x, int* y, int powerup, const char* lumpname, int color, int blinking) {
    int flags = local->component.vpt;

    if (W_PWADLumpNameExists(lumpname))
        color = CR_DEFAULT;

    if (color != CR_DEFAULT)
        flags |= VPT_TRANS;

    if ((!blinking && powerup) || (blinking && (powerup > BLINKTHRESHOLD || (powerup & 8))))
        V_DrawNamePatch(*x, *y, FG, lumpname, color, flags);

    x += patch_spacing;
    y += patch_vertical_spacing;

    if (local->vertical)
        *y += PATCH_SIZE;
    else
        *x += PATCH_SIZE;
}

static void dsda_DrawComponent(void) {
    player_t* player;
    int x, y;

    player = &players[displayplayer];

    x = local->component.x;
    y = local->component.y;

    if(raven || !dsda_IntConfig(nyan_config_ex_status_widget))
        return;

    if (player->armortype > 0 && dsda_IntConfig(nyan_config_ex_status_armor))
        drawPowerupStatusIcon(player, &x, &y, player->armortype > 0, drawArmorName(player), CR_DEFAULT, NOT_BLINKING);

    if (player->powers[pw_strength] && dsda_IntConfig(nyan_config_ex_status_berserk))
    {
        int color = player->powers[pw_strength] ? unityedition ? CR_GREEN : CR_RED : CR_DEFAULT;    
        drawPowerupStatusIcon(player, &x, &y, player->powers[pw_strength], "STFPPSTR", color, NOT_BLINKING);
    }

    if (player->powers[pw_allmap] && dsda_IntConfig(nyan_config_ex_status_areamap))
        drawPowerupStatusIcon(player, &x, &y, player->powers[pw_allmap], "STFPMAP", CR_DEFAULT, NOT_BLINKING);

    if (player->backpack && dsda_IntConfig(nyan_config_ex_status_backpack))
        drawPowerupStatusIcon(player, &x, &y, player->backpack, "STFPBPAK", CR_DEFAULT, NOT_BLINKING);

    if (player->powers[pw_ironfeet] && dsda_IntConfig(nyan_config_ex_status_radsuit))
        drawPowerupStatusIcon(player, &x, &y, player->powers[pw_ironfeet], "STFPSUIT", CR_DEFAULT, BLINKING);

    if (player->powers[pw_invisibility] && dsda_IntConfig(nyan_config_ex_status_invis))
        drawPowerupStatusIcon(player, &x, &y, player->powers[pw_invisibility], "STFPINS", CR_DEFAULT, BLINKING);

    if (player->powers[pw_infrared] && dsda_IntConfig(nyan_config_ex_status_liteamp))
        drawPowerupStatusIcon(player, &x, &y, player->powers[pw_infrared], "STFPVIS", CR_DEFAULT, BLINKING);

    if (player->powers[pw_invulnerability] && dsda_IntConfig(nyan_config_ex_status_invuln))
        drawPowerupStatusIcon(player, &x, &y, player->powers[pw_invulnerability], "STFPINV", CR_DEFAULT, BLINKING);
}

void dsda_InitStatusHC(int x_offset, int y_offset, int vpt, int* args, int arg_count, void** data) {
    *data = Z_Calloc(1, sizeof(local_component_t));
    local = *data;

    local->vertical = arg_count > 0 ? !!args[0] : false;

    dsda_InitPatchHC(&local->component, x_offset, y_offset, vpt);
}

void dsda_UpdateStatusHC(void* data) {
    local = data;
}

void dsda_DrawStatusHC(void* data) {
    local = data;

    dsda_DrawComponent();
}
