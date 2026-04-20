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
//	DEH Version
//

#ifndef __HHE_VERSION__
#define __HHE_VERSION__

#include "deh/func.h"

typedef enum
{
    deh_hhe_1_0,
    deh_hhe_1_2,
    deh_hhe_1_3,
    deh_hhe_num_versions
} deh_hhe_version_t;

extern deh_hhe_version_t deh_hhe_version;
extern dboolean hhe_version_set;

extern const char *hhe_versions[];
extern int hhe_numversions;

extern void HHE_SuggestHereticVersion(deh_hhe_version_t version);
extern int HHE_DetectVersionFromOffsets(DEHFILE *fpin, const char *label);
extern void HHE_ApplyVersion10Patch(void);
extern void HHE_GetVersionFromArgOrLump(void);

extern void HHE_PrintValidVersion(const char *reason);

#endif
