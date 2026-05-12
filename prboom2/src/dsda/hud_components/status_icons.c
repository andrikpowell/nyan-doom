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

#include "status_icons.h"
#include "powerups.h"

#define PATCH_SIZE 9
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

static void drawPowerupStatusIcon(int *x, int *y, int powerup, const char *lumpname, int color)
{
    dboolean from_pwad = false;
    int flags = local->component.vpt;

    if (!lumpname)
        return;

    if (W_PWADLumpNameExists2(lumpname))
        from_pwad++;

    if (!from_pwad && color != CR_DEFAULT)
        flags |= VPT_COLOR;

    V_DrawMenuNamePatch(*x, *y, lumpname, color, flags);

    if (local->vertical)
        *y += local->reverse ? -(patch_vertical_spacing + PATCH_SIZE)
                             :  (patch_vertical_spacing + PATCH_SIZE);
    else
        *x += local->reverse ? -(patch_spacing + PATCH_SIZE)
                             :  (patch_spacing + PATCH_SIZE);
}

static int *powerup_index;

static void dsda_DrawComponent(void) {
    player_t* player;
    int x, y;
    int *idx;
    int n = 0;

    player = &players[displayplayer];

    x = local->component.x;
    y = local->component.y;
    idx = powerup_index;

    dsda_SortPowerups(player, idx, &n, POWERUP_STATUS_ICON, local->reverse);

    for (int i = 0; i < n; ++i)
    {
        const dsda_powerup_t *pwr = &powerups[idx[i]];
        int tics = dsda_NormalizePowerupTics(pwr, pwr->tics(player));
        const char *lump = dsda_PowerupIconLump(pwr);

        if (!lump || tics == 0)
            continue;

        drawPowerupStatusIcon(&x, &y, tics, lump, dsda_PowerupIcon(pwr, tics));
    }
}

void dsda_InitStatusIconsHC(int x_offset, int y_offset, int vpt, int* args, int arg_count, void** data) {
    *data = Z_Calloc(1, sizeof(local_component_t));
    local = *data;

    powerup_index = Z_Malloc(dsda_powerup_count * sizeof(*powerup_index));

    local->vertical = arg_count > 0 ? !!args[0] : false;
    local->reverse  = arg_count > 1 ? !!args[1] : false;

    dsda_InitPatchHC(&local->component, x_offset, y_offset, vpt);
}

void dsda_UpdateStatusIconsHC(void* data) {
    local = data;
}

void dsda_DrawStatusIconsHC(void* data) {
    local = data;

    dsda_DrawComponent();
}
