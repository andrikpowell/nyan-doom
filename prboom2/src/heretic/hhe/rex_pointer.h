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
//	HHE [CODEPTR] Parser
//

#ifndef __HHE_REX_PTR__
#define __HHE_REX_PTR__

#include "deh/func.h"
#include "heretic/hhe/version.h"
#include "info.h"

typedef struct {
  int offsets[deh_hhe_num_versions];
  actionf_t cptr;  // actual pointer to the subroutine
  const char *lookup;  // mnemonic lookup string to be specified in REX
  // CPhipps - const*
} hhe_rexptr;

extern const hhe_rexptr hhe_rexptrs[];
extern int hhe_numrexptr;

#endif
