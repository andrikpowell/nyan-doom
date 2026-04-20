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
//	HHE Thing
//

#ifndef __HHE_THING__
#define __HHE_THING__

#include "m_file.h"
#include "deh/func.h"

extern const struct deh_flag_s hhe_mobjflags[];
extern const struct deh_flag_s hhe_mobjflags2[];

extern uint64_t hhe_stringToMobjFlags(char *strval);
extern uint64_t hhe_stringToMobjFlags2(char *strval);

#endif
