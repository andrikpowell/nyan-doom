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

#include "doomstat.h"
#include "doomdef.h"
#include "e6y.h"//e6y
#include "m_misc.h"
#include "deh/func.h"
#include "dsda/utility.h"

// ====================================================================
// General utility function(s)
// ====================================================================

FILE *deh_log_file;

int deh_log(const char *s, ...)
{
  int r;
  va_list v;

  if (!deh_log_file) return 0;

  va_start(v,s);
  r = vfprintf(deh_log_file, s, v);
  va_end(v);

  return r;
}

// e6y: for compatibility with BOOM deh parser
int deh_strcasecmp(const char *str1, const char *str2)
{
  if (!raven && prboom_comp[PC_BOOM_DEH_PARSER].state &&
      compatibility_level >= boom_compatibility_compatibility &&
      compatibility_level <= boom_202_compatibility)
  {
    return strcmp(str1, str2);
  }
  else
  {
    return strcasecmp(str1, str2);
  }
}

dboolean deh_strcasestr(const char *haystack, const char *needle)
{
    size_t nlen = strlen(needle);

    if (!nlen) return true;

    for (; *haystack; ++haystack)
        if (!strncasecmp(haystack, needle, nlen))
            return true;

    return false;
}

const char * deh_getBitsDelims(void)
{
  if (!raven && prboom_comp[PC_BOOM_DEH_PARSER].state &&
      compatibility_level >= boom_compatibility_compatibility &&
      compatibility_level <= boom_202_compatibility)
  {
    return "+";
  }
  else
  {
    return ",+| \t\f\r";
  }
}

// killough 10/98: new functions, to allow processing DEH files in-memory
// (e.g. from wads)

// killough 10/98: emulate IO whether input really comes from a file or not

char *dehfgets(char *buf, size_t n, DEHFILE *fp)
{
  if (!fp->lump)                                     // If this is a real file,
    return (fgets)(buf, n, fp->f);                   // return regular fgets
  if (!n || !*fp->inp || fp->size <= 0)                // If no more characters
    return NULL;
  if (n == 1)
    fp->size--, *buf = *fp->inp++;
  else
  {                                                // copy buffer
    char *p = buf;
    while (n > 1 && *fp->inp && fp->size &&
           (n--, fp->size--, *p++ = *fp->inp++) != '\n')
      ;
    *p = 0;
  }
  return buf;                                        // Return buffer pointer
}

int dehfeof(DEHFILE *fp)
{
  return !fp->lump ? feof(fp->f) : !*fp->inp || fp->size <= 0;
}

int dehfgetc(DEHFILE *fp)
{
  return !fp->lump ? fgetc(fp->f) : fp->size > 0 ?
    fp->size--, *fp->inp++ : EOF;
}

long dehftell(DEHFILE *fp)
{
  return !fp->lump ? ftell(fp->f) : (fp->inp - fp->lump);
}

int dehfseek(DEHFILE *fp, long offset)
{
  if (!fp->lump)
    return fseek(fp->f, offset, SEEK_SET);
  else
  {
    long total = (fp->inp - fp->lump) + fp->size;
    offset = BETWEEN(0, total, offset);
    fp->inp = fp->lump + offset;
    fp->size = total - offset;
    return 0;
  }
}

char *deh_sfx_name(const char *name)
{
  dsda_string_t str;

  dsda_StringPrintF(&str, "%s%s", raven ? "" : "ds", name);

  return str.string;
}

uint64_t deh_translate_bits(uint64_t value, const struct deh_flag_s *flags)
{
  int i;
  uint64_t result = 0;

  for (i = 0; flags[i].name; ++i)
  {
    if (value & 1)
      result |= flags[i].value;

    value >>= 1;
  }

  return result;
}

// ====================================================================
// dehReformatStr
// Purpose: Convert a string into a continuous string with embedded
//          linefeeds for "\n" sequences in the source string
// Args:    string -- the string to convert
// Returns: the converted string (converted in a static buffer)
//
char *dehReformatStr(char *string)
{
  static char buff[DEH_BUFFERMAX]; // only processing the changed string,
  //  don't need double buffer
  char *s, *t;

  s = string;  // source
  t = buff;    // target
  // let's play...

  while (*s)
  {
    if (*s == '\n')
      ++s, *t++ = '\\', *t++ = 'n', *t++ = '\\', *t++='\n';
    else
      *t++ = *s++;
  }
  *t = '\0';
  return buff;
}

// ====================================================================
// lfstrip
// Purpose: Strips CR/LF off the end of a string
// Args:    s -- the string to work on
// Returns: void -- the string is modified in place
//
// killough 10/98: only strip at end of line, not entire string

void lfstrip(char *s)  // strip the \r and/or \n off of a line
{
  char *p = s + strlen(s);
  while (p > s && (*--p == '\r' || *p == '\n'))
    *p = 0;
}

// ====================================================================
// rstrip
// Purpose: Strips trailing blanks off a string
// Args:    s -- the string to work on
// Returns: void -- the string is modified in place
//
void rstrip(char *s)  // strip trailing whitespace
{
  char *p = s + strlen(s);         // killough 4/4/98: same here
  while (p > s && isspace(*--p)) // break on first non-whitespace
    *p = '\0';
}

// ====================================================================
// ptr_lstrip
// Purpose: Points past leading whitespace in a string
// Args:    s -- the string to work on
// Returns: char * pointing to the first nonblank character in the
//          string.  The original string is not changed.
//
char *ptr_lstrip(char *p)  // point past leading whitespace
{
  while (isspace(*p))
    p++;
  return p;
}

// ====================================================================
// deh_GetData
// Purpose: Get a key and data pair from a passed string
// Args:    s -- the string to be examined
//          k -- a place to put the key
//          l -- pointer to a long integer to store the number
//          strval -- a pointer to the place in s where the number
//                    value comes from.  Pass NULL to not use this.
// Notes:   Expects a key phrase, optional space, equal sign,
//          optional space and a value, mostly an int but treated
//          as a long just in case.  The passed pointer to hold
//          the key must be DEH_MAXKEYLEN in size.

dboolean deh_GetData(char *s, char *k, uint64_t *l, char **strval)
{
  char *t;  // current char
  int val; // to hold value of pair
  char buffer[DEH_MAXKEYLEN];  // to hold key in progress
  // e6y: Correction of wrong processing of Bits parameter if its value is equal to zero
  // No more desync on HACX demos.
  dboolean okrc = 1;  // assume good unless we have problems
  int i;  // iterator

  *buffer = '\0';
  val = 0;  // defaults in case not otherwise set
  for (i = 0, t = s; *t && i < DEH_MAXKEYLEN; t++, i++)
  {
    if (*t == '=') break;
    buffer[i] = *t;  // copy it
  }
  buffer[--i] = '\0';  // terminate the key before the '='
  if (!*t)  // end of string with no equal sign
  {
    okrc = false;
  }
  else
  {
    if (!*++t)
    {
      val = 0;  // in case "thiskey =" with no value
      okrc = false;
    }
    // we've incremented t
    // e6y: Correction of wrong processing of Bits parameter if its value is equal to zero
    // No more desync on HACX demos.
    // Old code: e6y val = strtol(t,NULL,0);  // killough 8/9/98: allow hex or octal input
    if (!M_StrToInt(t, &val))
    {
      val = 0;
      okrc = 2;
    }
  }

  // go put the results in the passed pointers
  *l = val;  // may be a faked zero

  // if spaces between key and equal sign, strip them
  strcpy(k, ptr_lstrip(buffer));  // could be a zero-length string

  if (strval != NULL) // pass NULL if you don't want this back
    *strval = t;      // pointer, has to be somewhere in s,
  // even if pointing at the zero byte.

  return(okrc);
}

dboolean deh_StringEndsWith(const char *s, const char *suffix)
{
  size_t sl, su;
  if (!s || !suffix) return false;
  sl = strlen(s);
  su = strlen(suffix);
  if (sl < su) return false;
  return !strcasecmp(s + sl - su, suffix);
}
