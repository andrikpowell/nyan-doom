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
//	DSDA Mobj Info
//

#include <stdlib.h>
#include <string.h>

#include "sounds.h"
#include "p_map.h"

#include "dsda/map_format.h"

#include "mobjinfo.h"

mobjinfo_t* mobjinfo;
int num_mobj_types;
int mobj_types_zero;
byte* edited_mobjinfo_bits;
byte* edited_mobjinfo_bloodcolor;
static int mobjinfo_capacity;

static void dsda_ResetMobjInfo(int from, int to) {
  int i;
  int visibility = raven ? hexen ? VF_HEXEN :
                                   VF_HERETIC :
                                   VF_DOOM;

  for (i = from; i < to; ++i) {
    mobjinfo[i].droppeditem = MT_NULL;
    mobjinfo[i].infighting_group = IG_DEFAULT;
    mobjinfo[i].projectile_group = PG_DEFAULT;
    mobjinfo[i].splash_group = SG_DEFAULT;
    mobjinfo[i].altspeed = NO_ALTSPEED;
    mobjinfo[i].meleerange = MELEERANGE;
    mobjinfo[i].visibility = visibility;
  }
}

static void dsda_EnsureCapacity(int limit) {
  while (limit >= mobjinfo_capacity) {
    int old_capacity = mobjinfo_capacity;

    mobjinfo_capacity *= 2;

    mobjinfo = Z_Realloc(mobjinfo, mobjinfo_capacity * sizeof(*mobjinfo));
    memset(mobjinfo + old_capacity, 0, (mobjinfo_capacity - old_capacity) * sizeof(*mobjinfo));

    if (edited_mobjinfo_bits)
    {
      edited_mobjinfo_bits = Z_Realloc(edited_mobjinfo_bits, mobjinfo_capacity * sizeof(*edited_mobjinfo_bits));
      memset(edited_mobjinfo_bits + old_capacity, 0, (mobjinfo_capacity - old_capacity) * sizeof(*edited_mobjinfo_bits));
    }

    if (edited_mobjinfo_bloodcolor)
    {
      edited_mobjinfo_bloodcolor = Z_Realloc(edited_mobjinfo_bloodcolor, mobjinfo_capacity * sizeof(*edited_mobjinfo_bloodcolor));
      memset(edited_mobjinfo_bloodcolor + old_capacity, 0, (mobjinfo_capacity - old_capacity) * sizeof(*edited_mobjinfo_bloodcolor));
    }

    dsda_ResetMobjInfo(old_capacity, mobjinfo_capacity);
  }
}

static void dsda_EnsureIndex(int index) {
  dsda_EnsureCapacity(index);

  if (index >= num_mobj_types)
    num_mobj_types = index + 1;
}

static deh_index_hash_t deh_mobj_index_hash;

int dsda_FindDehMobjIndex(int index) {
  return dsda_FindDehIndex(index, &deh_mobj_index_hash);
}

int dsda_GetDehMobjIndex(int index) {
  int result = dsda_GetDehIndex(index, &deh_mobj_index_hash);

  dsda_EnsureIndex(result);

  return result;
}

// Dehacked has the index off by 1
int dsda_TranslateDehMobjIndex(int index) {
  return dsda_GetDehMobjIndex(index - 1) + 1;
}

dsda_deh_mobjinfo_t dsda_GetDehMobjInfo(int index) {
  dsda_deh_mobjinfo_t deh_mobjinfo;

  dsda_EnsureIndex(index);

  deh_mobjinfo.info = &mobjinfo[index];
  deh_mobjinfo.edited_bits = &edited_mobjinfo_bits[index];
  deh_mobjinfo.edited_bloodcolor = &edited_mobjinfo_bloodcolor[index];

  return deh_mobjinfo;
}

void dsda_InitializeMobjInfo(int zero, int count) {
  extern dboolean hexen;

  num_mobj_types = count;
  mobjinfo_capacity = count;
  mobj_types_zero = zero;

  mobjinfo = Z_Calloc(mobjinfo_capacity, sizeof(*mobjinfo));

  if (hexen) return;

  deh_mobj_index_hash.start_index = count;
  deh_mobj_index_hash.end_index = num_mobj_types;
  edited_mobjinfo_bits = Z_Calloc(mobjinfo_capacity, sizeof(*edited_mobjinfo_bits));
  edited_mobjinfo_bloodcolor = Z_Calloc(mobjinfo_capacity, sizeof(*edited_mobjinfo_bloodcolor));
}

static int dsda_AppendMobjInfo(void) {
  int index = num_mobj_types;

  dsda_EnsureIndex(index);
  memset(&mobjinfo[index], 0, sizeof(mobjinfo[index]));
  dsda_ResetMobjInfo(index, index + 1);

  return index;
}

// Changing the renderer causes a reset that accesses this list,
//   so we can't free it.
void dsda_FreeDehMobjInfo(void) {
  // free(edited_mobjinfo_bits);
}

int ZMT_MAPSPOT = ZMT_UNDEFINED;
int ZMT_MAPSPOT_GRAVITY = ZMT_UNDEFINED;
int ZMT_TELEPORTDEST2 = ZMT_UNDEFINED;
int ZMT_TELEPORTDEST3 = ZMT_UNDEFINED;
int ZMT_AMBIENTSOUND = ZMT_UNDEFINED;

static mobjinfo_t zmt_mapspot_info = {
  .doomednum = 9001,
  .spawnstate = S_NULL,
  .spawnhealth = 1000,
  .seestate = S_NULL,
  .seesound = sfx_None,
  .reactiontime = 8,
  .attacksound = sfx_None,
  .painstate = S_NULL,
  .painchance = 0,
  .painsound = sfx_None,
  .meleestate = S_NULL,
  .missilestate = S_NULL,
  .deathstate = S_NULL,
  .xdeathstate = S_NULL,
  .deathsound = sfx_None,
  .speed = 0,
  .radius = 20 * FRACUNIT,
  .height = 16 * FRACUNIT,
  .mass = 100,
  .damage = 0,
  .activesound = sfx_None,
  .flags = MF_NOBLOCKMAP | MF_NOSECTOR | MF_NOGRAVITY,
  .raisestate = S_NULL,
  .droppeditem = MT_NULL,
  .crashstate = S_NULL,
  .flags2 = 0,
  .infighting_group = IG_DEFAULT,
  .projectile_group = PG_DEFAULT,
  .splash_group = SG_DEFAULT,
  .ripsound = sfx_None,
  .altspeed = NO_ALTSPEED,
  .meleerange = MELEERANGE,
  .bloodcolor = 0,
  .visibility = VF_ZDOOM,
};

static mobjinfo_t zmt_mapspot_gravity_info = {
  .doomednum = 9013,
  .spawnstate = S_NULL,
  .spawnhealth = 1000,
  .seestate = S_NULL,
  .seesound = sfx_None,
  .reactiontime = 8,
  .attacksound = sfx_None,
  .painstate = S_NULL,
  .painchance = 0,
  .painsound = sfx_None,
  .meleestate = S_NULL,
  .missilestate = S_NULL,
  .deathstate = S_NULL,
  .xdeathstate = S_NULL,
  .deathsound = sfx_None,
  .speed = 0,
  .radius = 20 * FRACUNIT,
  .height = 16 * FRACUNIT,
  .mass = 100,
  .damage = 0,
  .activesound = sfx_None,
  .flags = 0,
  .raisestate = S_NULL,
  .droppeditem = MT_NULL,
  .crashstate = S_NULL,
  .flags2 = MF2_DONTDRAW,
  .infighting_group = IG_DEFAULT,
  .projectile_group = PG_DEFAULT,
  .splash_group = SG_DEFAULT,
  .ripsound = sfx_None,
  .altspeed = NO_ALTSPEED,
  .meleerange = MELEERANGE,
  .bloodcolor = 0,
  .visibility = VF_ZDOOM,
};

static mobjinfo_t zmt_teleportdest2_info = {
  .doomednum = 9044,
  .spawnstate = S_NULL,
  .spawnhealth = 1000,
  .seestate = S_NULL,
  .seesound = sfx_None,
  .reactiontime = 8,
  .attacksound = sfx_None,
  .painstate = S_NULL,
  .painchance = 0,
  .painsound = sfx_None,
  .meleestate = S_NULL,
  .missilestate = S_NULL,
  .deathstate = S_NULL,
  .xdeathstate = S_NULL,
  .deathsound = sfx_None,
  .speed = 0,
  .radius = 20 * FRACUNIT,
  .height = 16 * FRACUNIT,
  .mass = 100,
  .damage = 0,
  .activesound = sfx_None,
  .flags = MF_NOBLOCKMAP | MF_NOSECTOR | MF_NOGRAVITY,
  .raisestate = S_NULL,
  .droppeditem = MT_NULL,
  .crashstate = S_NULL,
  .flags2 = 0,
  .infighting_group = IG_DEFAULT,
  .projectile_group = PG_DEFAULT,
  .splash_group = SG_DEFAULT,
  .ripsound = sfx_None,
  .altspeed = NO_ALTSPEED,
  .meleerange = MELEERANGE,
  .bloodcolor = 0,
  .visibility = VF_ZDOOM,
};

static mobjinfo_t zmt_teleportdest3_info = {
  .doomednum = 9043,
  .spawnstate = S_NULL,
  .spawnhealth = 1000,
  .seestate = S_NULL,
  .seesound = sfx_None,
  .reactiontime = 8,
  .attacksound = sfx_None,
  .painstate = S_NULL,
  .painchance = 0,
  .painsound = sfx_None,
  .meleestate = S_NULL,
  .missilestate = S_NULL,
  .deathstate = S_NULL,
  .xdeathstate = S_NULL,
  .deathsound = sfx_None,
  .speed = 0,
  .radius = 20 * FRACUNIT,
  .height = 16 * FRACUNIT,
  .mass = 100,
  .damage = 0,
  .activesound = sfx_None,
  .flags = MF_NOBLOCKMAP | MF_NOSECTOR,
  .raisestate = S_NULL,
  .droppeditem = MT_NULL,
  .crashstate = S_NULL,
  .flags2 = 0,
  .infighting_group = IG_DEFAULT,
  .projectile_group = PG_DEFAULT,
  .splash_group = SG_DEFAULT,
  .ripsound = sfx_None,
  .altspeed = NO_ALTSPEED,
  .meleerange = MELEERANGE,
  .bloodcolor = 0,
  .visibility = VF_ZDOOM,
};

static mobjinfo_t zmt_ambient_sound = {
  .doomednum = 14064,
  .spawnstate = S_NULL,
  .spawnhealth = 1000,
  .seestate = S_NULL,
  .seesound = sfx_None,
  .reactiontime = 8,
  .attacksound = sfx_None,
  .painstate = S_NULL,
  .painchance = 0,
  .painsound = sfx_None,
  .meleestate = S_NULL,
  .missilestate = S_NULL,
  .deathstate = S_NULL,
  .xdeathstate = S_NULL,
  .deathsound = sfx_None,
  .speed = 0,
  .radius = 20 * FRACUNIT,
  .height = 16 * FRACUNIT,
  .mass = 100,
  .damage = 0,
  .activesound = sfx_None,
  .flags = MF_NOBLOCKMAP | MF_NOSECTOR,
  .raisestate = S_NULL,
  .droppeditem = MT_NULL,
  .crashstate = S_NULL,
  .flags2 = 0,
  .infighting_group = IG_DEFAULT,
  .projectile_group = PG_DEFAULT,
  .splash_group = SG_DEFAULT,
  .ripsound = sfx_None,
  .altspeed = NO_ALTSPEED,
  .meleerange = MELEERANGE,
  .bloodcolor = 0,
  .visibility = VF_DOOM,
};

typedef struct {
  int* index_p;
  mobjinfo_t* mobjinfo_p;
} append_mobjinfo_t;

static append_mobjinfo_t append_mobjinfo[] = {
  { &ZMT_MAPSPOT, &zmt_mapspot_info },
  { &ZMT_MAPSPOT_GRAVITY, &zmt_mapspot_gravity_info },
  { &ZMT_TELEPORTDEST2, &zmt_teleportdest2_info },
  { &ZMT_TELEPORTDEST3, &zmt_teleportdest3_info },
  { &ZMT_AMBIENTSOUND, &zmt_ambient_sound },
};

static int append_mobjinfo_count = sizeof(append_mobjinfo) / sizeof(append_mobjinfo[0]);

void dsda_AppendZDoomMobjInfo(void) {
  int i;
  int index;

  for (i = 0; i < append_mobjinfo_count; ++i) {
    index = dsda_AppendMobjInfo();
    *(append_mobjinfo[i].index_p) = index;
    mobjinfo[index] = *(append_mobjinfo[i].mobjinfo_p);
  }
}
