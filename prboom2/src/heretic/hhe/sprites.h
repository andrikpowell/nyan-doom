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
//	HHE Strings
//

#ifndef __HHE_SPRITES__
#define __HHE_SPRITES__

#include "heretic/hhe/version.h"

typedef struct {
  int offsets[deh_hhe_num_versions];
  const char default_string[5];   // "IMPX" + '\0'
} hhe_sprite_str_t;

extern const hhe_sprite_str_t hhe_sprites[];

extern int hhe_numsprites;

#endif
