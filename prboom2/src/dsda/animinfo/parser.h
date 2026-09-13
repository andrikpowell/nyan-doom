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
//	NYAN ANIMINFO Parser
//

#ifndef __NYAN_ANIMINFO_PARSER__
#define __NYAN_ANIMINFO_PARSER__

#include <stddef.h>

#include "dsda/animinfo.h"

typedef void (*animinfo_errorfunc)(const char *fmt, ...); // this must not return!

typedef struct
{
    int lump;

    anim_type_t type;
    int startpic;
    int endpic;
    int tics;
    const anim_frame_t *frames;
    int num_frames;
    int oscillate;

    int widepic;

    // override from animinfo lump
    int animation_override;
    int widepic_override;
} animinfo_entry_t;

int ParseAnimInfo(const unsigned char *buffer, size_t length, animinfo_errorfunc err);
void N_ApplyAnimInfo(const animinfo_entry_t *entry);

#endif
