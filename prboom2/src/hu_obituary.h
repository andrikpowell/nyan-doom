//
// Copyright(C) 2026 by Andrik Powell
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
//	DSDA Obituaries
//

#ifndef __DSDA_OBITUARY__
#define __DSDA_OBITUARY__

typedef enum
{
    MOD_None,
    MOD_Slime,
    MOD_Lava,
    MOD_Falling,
    MOD_Crush,
    MOD_Telefrag,
    MOD_Melee,
} method_t;

void dsda_InitObituaries(void);
void dsda_Obituary(mobj_t *target, struct mobj_s *inflictor, struct mobj_s *source, method_t mod);

#endif