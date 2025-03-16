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
//  NYAN SKILL
//

#ifndef __NYANSKLL_PARSER__
#define __NYANSKLL_PARSER__

#ifdef __cplusplus
extern "C" {
#endif

extern doom_mapinfo_skill_t* nyan_skillinfo;

typedef void (*nyan_skill_errorfunc)(const char *fmt, ...);	// this must not return!

void dsda_LoadNyanSkillLump(const unsigned char* buffer, size_t length, nyan_skill_errorfunc err);

#ifdef __cplusplus
}
#endif

#endif
