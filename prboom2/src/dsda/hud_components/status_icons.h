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

#ifndef __DSDA_HUD_COMPONENT_STATUS_ICONS__
#define __DSDA_HUD_COMPONENT_STATUS_ICONS__

void dsda_InitStatusIconsHC(int x_offset, int y_offset, int vpt_flags, int* args, int arg_count, void** data);
void dsda_UpdateStatusIconsHC(void* data);
void dsda_DrawStatusIconsHC(void* data);

#endif
