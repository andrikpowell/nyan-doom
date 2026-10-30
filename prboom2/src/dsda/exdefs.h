//
// Copyright(C) 2026 Andrik Powell
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
//	DSDA EXDEFS Parser
//

#ifndef __DSDA_EXDEFS__
#define __DSDA_EXDEFS__

#include "doomtype.h"

void dsda_ParseExDefs(void);

// Artifact stuff
int dsda_ExDefsArtifactPickup(int mobjtype);                // artifact pickup
const char *dsda_ExDefsArtifactMessage(int artifact);       // artifact pickup message

dboolean dsda_ExDefsArtifactDefined(int artifact);          // artifact inventory def valid?
const char *dsda_ExDefsArtifactDisplayName(int artifact);   // artifact inventory name
const char *dsda_ExDefsArtifactIcon(int artifact);          // artifact inventory icon
int dsda_ExDefsArtifactsMax(void);                          // artifact inventory max

#endif
