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
//	DSDA Small Berserk HUD Component
//

#include "base.h"

#include "sml_berserk.h"

typedef struct {
    dsda_patch_component_t component;
} local_component_t;

static local_component_t* local;

static const char* dsda_BerserkName(player_t* player) {
    if (player->powers[pw_strength])
        if (chex) { return "CHXPPSTR"; }
        else { return "STFPPSTR"; }
    else
        return NULL;
}

void drawBerserkIcon(player_t* player, int* x, int* y, const char* (*hasBerserk)(player_t*)) {
    const char* name;
    int color = hasBerserk(player) ? unityedition ? CR_GREEN : CR_RED : CR_DEFAULT;
    int flags = local->component.vpt;

    if (W_PWADLumpNameExists("STFPPSTR") || chex)
        color = CR_DEFAULT;

    if (color != CR_DEFAULT)
        flags |= VPT_TRANS;  

    name = hasBerserk(player);

    if (name)
        V_DrawNamePatch(*x, *y, FG, name, color, flags);
}

static void dsda_DrawComponent(void) {
    player_t* player;
    int x, y;

    player = &players[displayplayer];

    x = local->component.x;
    y = local->component.y;
    if (!raven)
        drawBerserkIcon(player, &x, &y, dsda_BerserkName);
}

void dsda_InitSmlBerserkHC(int x_offset, int y_offset, int vpt, int* args, int arg_count, void** data) {
    *data = Z_Calloc(1, sizeof(local_component_t));
    local = *data;

    dsda_InitPatchHC(&local->component, x_offset, y_offset, vpt);
}

void dsda_UpdateSmlBerserkHC(void* data) {
    local = data;
}

void dsda_DrawSmlBerserkHC(void* data) {
    local = data;

    dsda_DrawComponent();
}
