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
//	DEH Shared Functions
//

#ifndef __DEH_FUNC__
#define __DEH_FUNC__

#include "m_file.h"

extern FILE *deh_log_file;

extern int deh_log(const char *s, ...);
// e6y: for compatibility with BOOM deh parser
extern int deh_strcasecmp(const char *str1, const char *str2);
extern dboolean deh_strcasestr(const char *haystack, const char *needle);
extern const char * deh_getBitsDelims(void);

#define DEH_BUFFERMAX 1024 // input buffer area size, hardcodedfor now
#define DEH_MAXKEYLEN 32 // as much of any key as we'll look at

typedef struct {
  /* cph 2006/08/06 -
   * if lump != NULL, lump is the start of the lump,
   * inp is the current read pos. */
  const byte *inp, *lump;
  long size;
  /* else, !lump, and f is the file being read */
  FILE* f;
} DEHFILE;

struct deh_flag_s {
  const char *name; // CPhipps - const*
  uint64_t value;
};

typedef struct
{
  const char *key;       // a mnemonic block code name // CPhipps - const*
  void (*const fptr)(DEHFILE *, char *); // handler
} deh_block;

extern uint64_t deh_translate_bits(uint64_t value, const struct deh_flag_s *flags);

extern char *dehfgets(char *buf, size_t n, DEHFILE *fp);
extern int dehfeof(DEHFILE *fp);
extern int dehfgetc(DEHFILE *fp);
extern long dehftell(DEHFILE *fp);
extern int dehfseek(DEHFILE *fp, long offset);
extern char *deh_sfx_name(const char *name);

// Function prototypes
extern void    lfstrip(char *);     // strip the \r and/or \n off of a line
extern void    rstrip(char *);      // strip trailing whitespace
extern char *  ptr_lstrip(char *);  // point past leading whitespace
extern dboolean deh_GetData(char *, char *, uint64_t *, char **);
extern char *  dehReformatStr(char *);

extern dboolean deh_StringEndsWith(const char *s, const char *suffix);
extern dboolean deh_procStringSub(char *key, char *lookfor, char *newstring);

#endif
