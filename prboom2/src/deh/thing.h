//
// Copyright(C) 1999-2004 by Jess Haas, Nicolas Kalkhof, Colin Phipps, Florian Schulze
// Copyright(C) 2005-2006 by Florian Schulze, Colin Phipps, Neil Stevens, Andrey Budko
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
//	DEH Thing
//

#ifndef __DEH_THING__
#define __DEH_THING__

#include "m_file.h"
#include "deh/func.h"

extern const struct deh_flag_s deh_mobjflags_standard[];
extern const struct deh_flag_s deh_mobjflags_mbf21[];

extern uint64_t deh_stringToMBF21MobjFlags(char *strval);
extern uint64_t deh_stringToMobjFlags(char *strval);

#endif
