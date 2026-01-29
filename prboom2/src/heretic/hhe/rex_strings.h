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
//	HHE [STRINGS] Parser
//

#ifndef __HHE_REX_STR__
#define __HHE_REX_STR__

#include "heretic/hhe/version.h"

// CPhipps - make strings pointed to const
typedef struct {
  int offsets[deh_hhe_num_versions];
  const char **ppstr;  // doubly indirect pointer to string
  const char *lookup;  // pointer to lookup string name
  const char *orig;
} hhe_strs;

extern int hhe_numstrlookup;
extern hhe_strs hhe_strlookup[];

#endif
