//
// Copyright(C) 2022 by Ryan Krafnick
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
//	DSDA Big Artifact HUD Component
//

#include "base.h"

#include "big_artifact.h"

typedef struct {
  dsda_patch_component_t component;
  dboolean show_empty_box;
} local_component_t;

static local_component_t* local;

void dsda_InitBigArtifactHC(int x_offset, int y_offset, int vpt, int* args, int arg_count, void** data) {
  *data = Z_Calloc(1, sizeof(local_component_t));
  local = *data;

  local->show_empty_box = arg_count > 0 ? !!args[0] : false;

  dsda_InitPatchHC(&local->component, x_offset, y_offset, vpt);
}

void dsda_UpdateBigArtifactHC(void* data) {
  local = data;
}

void dsda_DrawBigArtifactHC(void* data) {
  extern void DrawArtifact(int x, int y, dboolean show_empty_box, int vpt);

  local = data;

  DrawArtifact(local->component.x, local->component.y, local->show_empty_box, local->component.vpt);
}
