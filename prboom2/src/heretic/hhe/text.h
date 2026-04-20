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
//	HHE Text
//

#ifndef __HHE_TEXT__
#define __HHE_TEXT__

#include "info.h"
#include "heretic/hhe/version.h"

extern dboolean hhe_includenotext;
extern dboolean hhe_procStringSub(char *key, char *lookfor, int offset, char *newstring);
extern dboolean HHE_RenameSprite(int spritenum, const char *newname, const char *context, int offset);

typedef struct
{
    unsigned int offsets[deh_hhe_num_versions];
    const char *string;
} hhe_string_t;

extern const hhe_string_t crispystrings[];
extern int hhe_numcrispystrings;
extern const int *unsupported_strings[];

#endif
