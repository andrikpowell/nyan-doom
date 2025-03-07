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
//	DSDA Small Armor HUD Component
//

#include "base.h"

#include "sml_armor.h"

typedef struct {
    dsda_patch_component_t component;
} local_component_t;

static local_component_t* local;

static void dsda_DrawComponent(void) {
    player_t* player;
    int x, y;
    int lump;
    int armor;

    player = &players[displayplayer];
    x = local->component.x;
    y = local->component.y;

    if (!raven) {
        armor = player->armorpoints[ARMOR_ARMOR];
        if (armor <= 0) {
            lump = 0;
        }
        else {
            if (chex) {
                if (player->armortype < 2)
                    lump = W_CheckNumForName("CHXPARM1");
                else
                    lump = W_CheckNumForName("CHXPARM2");
                V_DrawNumPatch(x, y+2, FG, lump, CR_DEFAULT, local->component.vpt);

            }
            else {
                if (player->armortype < 2)
                    lump = W_CheckNumForName("STFPARM1");
                else
                    lump = W_CheckNumForName("STFPARM2");
                V_DrawNumPatch(x, y, FG, lump, CR_DEFAULT, local->component.vpt);
            }
        }
    }
}

void dsda_InitSmlArmorHC(int x_offset, int y_offset, int vpt, int* args, int arg_count, void** data) {
    *data = Z_Calloc(1, sizeof(local_component_t));
    local = *data;

    dsda_InitPatchHC(&local->component, x_offset, y_offset, vpt);
}

void dsda_UpdateSmlArmorHC(void* data) {
    local = data;
}

void dsda_DrawSmlArmorHC(void* data) {
    local = data;

    dsda_DrawComponent();
}
