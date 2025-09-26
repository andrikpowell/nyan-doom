//
// Copyright(C) 2025 by Andrik Powell
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
//	DSDA Big Artifact Bar HUD Component
//

#include "base.h"

#include "big_artifact_bar.h"

typedef struct {
  dsda_patch_component_t component;
} local_component_t;

static local_component_t* local;

void dsda_InitBigArtifactBarHC(int x_offset, int y_offset, int vpt, int* args, int arg_count, void** data) {
  *data = Z_Calloc(1, sizeof(local_component_t));
  local = *data;

  dsda_InitPatchHC(&local->component, x_offset, y_offset, vpt);
}

void dsda_UpdateBigArtifactBarHC(void* data) {
  local = data;
}

void dsda_DrawBigArtifactBarHC(void* data) {
  extern void DrawInventoryBarTranslucent(int x, int y, int vpt);

  local = data;

  DrawInventoryBarTranslucent(local->component.x, local->component.y, local->component.vpt);
}
