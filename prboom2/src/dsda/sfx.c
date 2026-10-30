//
// Copyright(C) 2021 by Ryan Krafnick
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
//	DSDA SFX
//

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "doomdef.h"
#include "doomtype.h"
#include "lprintf.h"

#include "dsda/configuration.h"

#include "sfx.h"

sfxinfo_t* S_sfx;
int num_sfx;
static int sfx_capacity;
static int deh_soundnames_size;
static char** deh_soundnames;
static byte* sfx_state;

static void dsda_ResetSFX(int from, int to) {
  int i;

  for (i = from; i < to; ++i) {
    if (hexen)
    {
      S_sfx[i].priority = 256;
      S_sfx[i].pitch = 0;
      S_sfx[i].numchannels = 2;
    }
    else if (heretic)
    {
      S_sfx[i].priority = 32;
      S_sfx[i].pitch = -1;
      S_sfx[i].numchannels = 2;
    }
    else // Doom
    {
      S_sfx[i].priority = 127;
      S_sfx[i].pitch = -1;
    }
  }
}

static void dsda_PrepAllocation(void) {
  static int first_allocation = true;

  if (first_allocation) {
    sfxinfo_t* source = S_sfx;

    first_allocation = false;
    S_sfx = malloc(num_sfx * sizeof(*S_sfx));
    memcpy(S_sfx, source, num_sfx * sizeof(*S_sfx));
  }
}

static void dsda_EnsureCapacity(int limit) {
  while (limit >= sfx_capacity) {
    int old_capacity = sfx_capacity;

    dsda_PrepAllocation();

    sfx_capacity *= 2;

    S_sfx = realloc(S_sfx, sfx_capacity * sizeof(*S_sfx));
    memset(S_sfx + old_capacity, 0,
      (sfx_capacity - old_capacity) * sizeof(*S_sfx));

    if (sfx_state) {
      sfx_state = realloc(sfx_state, sfx_capacity * sizeof(*sfx_state));
      memset(sfx_state + old_capacity, 0,
        (sfx_capacity - old_capacity) * sizeof(*sfx_state));
    }

    dsda_ResetSFX(old_capacity, sfx_capacity);
  }
}

static void dsda_EnsureIndex(int index) {
  dsda_EnsureCapacity(index);

  // Extend sfx array
  if (index >= num_sfx)
    num_sfx = index + 1;
}

int dsda_GetDehSFXIndex(const char* key, size_t length) {
  int i;
  const char *name;

  for (i = 1; i < num_sfx; ++i) {
    if (!S_sfx[i].name)
      continue;

    // doom has 2-char offset "ds" for dehacked names
    name = raven ? S_sfx[i].name : S_sfx[i].name + 2;

    if (strlen(name) == length &&
        !strnicmp(name, key, length) &&
        !sfx_state[i])
    {
      sfx_state[i] = true; // sfx has been edited
      return i;
    }
  }

  return -1;
}

int dsda_GetOriginalSFXIndex(const char* key) {
  int i;
  int length = raven ? 8 : 6;
  const char* c;

  for (i = 1; deh_soundnames[i]; ++i)
    if (!strncasecmp(deh_soundnames[i], key, length))
      return i;

  // is it a number?
  for (c = key; *c; c++)
    if (!isdigit(*c))
      return -1;

  i = atoi(key);
  dsda_EnsureIndex(i);

  return i;
}

static sfxinfo_t* dsda_SFXAtIndex(int index) {
  dsda_EnsureIndex(index);

  return &S_sfx[index];
}

sfxinfo_t* dsda_GetDehSFX(int index) {
  return dsda_SFXAtIndex(index);
}

sfxinfo_t* dsda_NewSFX(int* index) {
  *index = num_sfx;

  return dsda_SFXAtIndex(*index);
}

void dsda_InitializeSFX(sfxinfo_t* source, int count) {
  int i;
  extern int hexen;

  num_sfx = count;
  sfx_capacity = count;
  deh_soundnames_size = num_sfx + 1;

  S_sfx = source;

  if (hexen) return;

  deh_soundnames = malloc(deh_soundnames_size * sizeof(*deh_soundnames));
  for (i = 1; i < num_sfx; i++)
    if (S_sfx[i].name != NULL)
      deh_soundnames[i] = strdup(heretic ? S_sfx[i].name : S_sfx[i].name + 2);
    else
      deh_soundnames[i] = NULL;
  deh_soundnames[0] = NULL;
  deh_soundnames[num_sfx] = NULL;

  sfx_state = calloc(num_sfx, sizeof(*sfx_state));
}

void dsda_FreeDehSFX(void) {
  int i;

  if (deh_soundnames)
    for (i = 0; i < deh_soundnames_size; i++)
      if (deh_soundnames[i])
        free(deh_soundnames[i]);

  free(deh_soundnames);
  free(sfx_state);

  deh_soundnames = NULL;
  sfx_state = NULL;
}

static int dsda_parallel_sfx_active;
static int dsda_parallel_sfx_limit;
static int dsda_parallel_sfx_window;

void dsda_InitParallelSFXFilter(void) {
  dsda_parallel_sfx_active = dsda_IntConfig(dsda_config_parallel_sfx_active);
  dsda_parallel_sfx_limit = dsda_IntConfig(dsda_config_parallel_sfx_limit);
  dsda_parallel_sfx_window = dsda_IntConfig(dsda_config_parallel_sfx_window);
}

dboolean dsda_BlockSFX(sfxinfo_t *sfx) {
  extern int gametic;

  if (!dsda_parallel_sfx_active) return false;

  if (gametic - sfx->parallel_tic >= dsda_parallel_sfx_window) {
    sfx->parallel_tic = gametic;
    sfx->parallel_count = 0;
  }

  ++sfx->parallel_count;

  return sfx->parallel_count > dsda_parallel_sfx_limit;
}

#define SFX_NOT_FOUND (-1)

static int dsda_FindSFX(const char *name) {
  int i;

  for (i = 1; i < num_sfx; ++i)
    if (S_sfx[i].tagname && !stricmp(S_sfx[i].tagname, name))
      return i;

  return SFX_NOT_FOUND;
}

dboolean dsda_SFXExists(const char *name) {
  return dsda_FindSFX(name) != SFX_NOT_FOUND;
}

void dsda_UpdateSFX(const char *name, const char *lump) {
  int index = dsda_FindSFX(name);

  if (index != SFX_NOT_FOUND)
  {
    S_sfx[index].name = strdup(lump);
    S_sfx[index].data = NULL;
    S_sfx[index].lumpnum = -1;
  }
}

int dsda_AppendSFX(const char *name, const char *lump) {
  int index = num_sfx;

  dsda_PrepAllocation();
  dsda_EnsureIndex(index);
  memset(&S_sfx[index], 0, sizeof(S_sfx[index]));
  dsda_ResetSFX(index, index + 1);

  S_sfx[index].name = strdup(lump);
  S_sfx[index].tagname = strdup(name);
  S_sfx[index].lumpnum = -1;

  return index;
}
