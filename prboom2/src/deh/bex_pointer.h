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
//	DEH [CODEPTR] Parser
//

#ifndef __DEH_BEX_PTR__
#define __DEH_BEX_PTR__

#include "deh/func.h"
#include "info.h"

#define TI_MISC1 0x0001
#define TI_MISC2 0x0002
#define TI_ARGSSHIFT 2
#define TI_ARGS1 0x0004
#define TI_ARGS2 0x0008
#define TI_ARGS3 0x0010
#define TI_ARGS4 0x0020
#define TI_ARGS5 0x0040
#define TI_ARGS6 0x0080
#define TI_ARGS7 0x0100
#define TI_ARGS8 0x0200

typedef struct {
  actionf_t cptr;  // actual pointer to the subroutine
  const char *lookup;  // mnemonic lookup string to be specified in BEX
  // CPhipps - const*
  short argcount;  // [XA] number of mbf21 args this action uses, if any
  long default_args[MAXSTATEARGS]; // default values for mbf21 args
  short ti_flags; // thing index on these args
} deh_bexptr;

extern const deh_bexptr deh_bexptrs[];
extern int deh_numbexptr;

#endif
