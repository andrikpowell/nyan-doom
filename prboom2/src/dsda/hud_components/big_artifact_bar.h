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

#ifndef __DSDA_HUD_COMPONENT_BIG_ARTIFACT_BAR__
#define __DSDA_HUD_COMPONENT_BIG_ARTIFACT_BAR__

void dsda_InitBigArtifactBarHC(int x_offset, int y_offset, int vpt_flags, int* args, int arg_count, void** data);
void dsda_UpdateBigArtifactBarHC(void* data);
void dsda_DrawBigArtifactBarHC(void* data);

#endif
