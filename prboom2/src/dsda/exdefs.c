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

#include <string.h>

#include "doomstat.h"
#include "lprintf.h"
#include "m_json.h"
#include "w_wad.h"
#include "z_zone.h"

#include "dsda/args.h"
#include "dsda/sfx.h"

#include "exdefs.h"

#define EXDEFS_MAX_DEPTH 64

static int exdefs_include_path[EXDEFS_MAX_DEPTH];

static void exdefs_ParseSounds(int lumpnum, json_t *data)
{
  int i;
  json_t *sound_exdefs = JS_GetObject(data, "sounds");

  if (!sound_exdefs)
    return;

  if (!JS_IsArray(sound_exdefs))
  {
    lprintf(LO_ERROR, "%.8s: EXDEFS sounds is not an array\n", W_LumpName(lumpnum));
    return;
  }

  for (i = 0; i < JS_GetArraySize(sound_exdefs); ++i)
  {
    json_t *entry = JS_GetArrayItem(sound_exdefs, i);
    const char *name = JS_GetStringValue(entry, "name");
    const char *lump = JS_GetStringValue(entry, "lump");

    if (JS_IsObject(entry) && name && lump)
    {
      if (dsda_SFXExists(name))
        dsda_UpdateSFX(name, lump);
      else
        dsda_AppendSFX(name, lump);
    }
    else
      lprintf(LO_ERROR, "%.8s: invalid EXDEFS sound entry %d\n", W_LumpName(lumpnum), i);
  }
}

static dboolean exdefs_LumpInIncludePath(int lumpnum, int depth)
{
  int i;

  for (i = 0; i < depth; ++i)
    if (exdefs_include_path[i] == lumpnum)
      return true;

  return false;
}

static dboolean exdefs_CheckValid(int lumpnum, json_t *json)
{
  const char *type;
  version_t version;
  const version_t max_version = {1, 0, 0};

  if (!JS_IsObject(json))
  {
    lprintf(LO_ERROR, "%.8s: EXDEFS root is not an object\n", W_LumpName(lumpnum));
    return false;
  }

  type = JS_GetStringValue(json, "type");
  if (!type || strcmp(type, "exdefs"))
  {
    lprintf(LO_ERROR, "%.8s: EXDEFS has an invalid type\n", W_LumpName(lumpnum));
    return false;
  }

  if (!JS_GetVersion(json, &version))
  {
    lprintf(LO_ERROR, "%.8s: EXDEFS has no valid version\n", W_LumpName(lumpnum));
    return false;
  }

  if (M_CompareVersions(&version, &max_version) > 0)
  {
    lprintf(LO_ERROR, "%.8s: unsupported EXDEFS version %d.%d.%d\n", W_LumpName(lumpnum), version.major, version.minor, version.revision);
    return false;
  }

  return true;
}

static void dsda_ParseExDefsLump(int lumpnum, int depth)
{
  int i;
  int count;
  json_t *json;
  json_t *data;
  json_t *includes;

  if (depth >= EXDEFS_MAX_DEPTH)
  {
    lprintf(LO_ERROR, "%.8s: EXDEFS include depth exceeds %d\n", W_LumpName(lumpnum), EXDEFS_MAX_DEPTH);
    return;
  }

  if (exdefs_LumpInIncludePath(lumpnum, depth))
  {
    lprintf(LO_ERROR, "%.8s: recursive EXDEFS include ignored\n", W_LumpName(lumpnum));
    return;
  }

  exdefs_include_path[depth] = lumpnum;
  json = JS_OpenOptions(lumpnum, true);
  if (!json)
    return;

  if (!exdefs_CheckValid(lumpnum, json))
  {
    JS_CloseOptions(lumpnum);
    return;
  }

  data = JS_GetObject(json, "data");
  if (!JS_IsObject(data))
  {
    lprintf(LO_ERROR, "%.8s: EXDEFS has no data object\n", W_LumpName(lumpnum));
    JS_CloseOptions(lumpnum);
    return;
  }

  includes = JS_GetObject(data, "include");
  if (includes && !JS_IsArray(includes))
  {
    lprintf(LO_ERROR, "%.8s: EXDEFS include is not an array\n", W_LumpName(lumpnum));
    JS_CloseOptions(lumpnum);
    return;
  }

  count = includes ? JS_GetArraySize(includes) : 0;

  // We set up sounds before parsing all later EXDEFS lumps
  exdefs_ParseSounds(lumpnum, data);

  for (i = 0; i < count; ++i)
  {
    int included_lump;
    const char *lumpname;
    json_t *entry = JS_GetArrayItem(includes, i);

    if (!JS_IsObject(entry))
    {
      lprintf(LO_ERROR, "%.8s: EXDEFS include entry %d is not an object\n", W_LumpName(lumpnum), i);
      continue;
    }

    // Skip enhanced EXDEFS
    if (JS_GetObject(entry, "ifoption"))
      continue;

    lumpname = JS_GetStringValue(entry, "lumpname");
    if (!lumpname || !*lumpname)
    {
      lprintf(LO_ERROR, "%.8s: EXDEFS include entry %d has no lumpname\n", W_LumpName(lumpnum), i);
      continue;
    }

    included_lump = W_CheckNumForName(lumpname);
    if (included_lump == LUMP_NOT_FOUND)
    {
      lprintf(LO_ERROR, "%.8s: EXDEFS include %.8s was not found\n", W_LumpName(lumpnum), lumpname);
      continue;
    }

    dsda_ParseExDefsLump(included_lump, depth + 1);
  }

  JS_CloseOptions(lumpnum);
}

void dsda_ParseExDefs(void)
{
  int lumpnum;

  if (!raven || !hexen)
    return;

  if (demorecording && !dsda_Flag(dsda_arg_exdefs))
    return;

  lprintf(LO_INFO, "dsda_ParseExDefs: Loading EXDEFS.\n");

  lumpnum = W_CheckNumForName("EXDEFS");

  if (lumpnum != LUMP_NOT_FOUND)
  {
    dsda_ParseExDefsLump(lumpnum, 0);
    exdefs = true;
  }
}
