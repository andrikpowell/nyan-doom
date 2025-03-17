//
// Copyright(C) 2023 by Ryan Krafnick
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
//	DSDA Skill Info
//

#include "doomstat.h"
#include "lprintf.h"
#include "w_wad.h"

#include "dsda/args.h"
#include "dsda/configuration.h"
#include "dsda/mapinfo/doom/parser.h"
#include "dsda/skilldef.h"
#include "dsda/preferences.h"
#include "dsda/text_color.h"
#include "dsda/utility.h"

#include "skill_info.h"

skill_info_t skill_info;

const skill_info_t doom_skill_infos[5] = {
  {
    .ammo_factor = FRACUNIT * 2,
    .damage_factor = FRACUNIT / 2,
    .spawn_filter = 1,
    .key = 'i',
    .name = "I'm too young to die.",
    .pic_name = "M_JKILL",
    .flags = SI_EASY_BOSS_BRAIN
  },
  {
    .spawn_filter = 2,
    .key = 'h',
    .name = "Hey, not too rough.",
    .pic_name = "M_ROUGH",
    .flags = SI_EASY_BOSS_BRAIN
  },
  {
    .spawn_filter = 3,
    .key = 'h',
    .name = "Hurt me plenty.",
    .pic_name = "M_HURT",
    .flags = 0
  },
  {
    .spawn_filter = 4,
    .key = 'u',
    .name = "Ultra-Violence.",
    .pic_name = "M_ULTRA",
    .flags = 0
  },
  {
    .ammo_factor = FRACUNIT * 2,
    .spawn_filter = 5,
    .key = 'n',
    .name = "Nightmare!",
    .pic_name = "M_NMARE",
    .respawn_time = 12,
    .flags = SI_FAST_MONSTERS | SI_INSTANT_REACTION | SI_MUST_CONFIRM
  },
};

const skill_info_t uvplus_skill_infos[1] = {
  {
    .spawn_filter = 4,
    .key = 'u',
    .name = "Ultra-Violence Plus.",
    .pic_name = "M_UVPLUS",
    .flags = SI_SPAWN_MULTI
  },
};

const skill_info_t heretic_skill_infos[5] = {
  {
    .ammo_factor = FRACUNIT * 3 / 2,
    .damage_factor = FRACUNIT / 2,
    .spawn_filter = 1,
    .name = "THOU NEEDETH A WET-NURSE",
    .flags = SI_AUTO_USE_HEALTH
  },
  {
    .spawn_filter = 2,
    .name = "YELLOWBELLIES-R-US",
    .flags = 0
  },
  {
    .spawn_filter = 3,
    .name = "BRINGEST THEM ONETH",
    .flags = 0
  },
  {
    .spawn_filter = 4,
    .name = "THOU ART A SMITE-MEISTER",
    .flags = 0
  },
  {
    .ammo_factor = FRACUNIT * 3 / 2,
    .spawn_filter = 5,
    .name = "BLACK PLAGUE POSSESSES THEE",
    .flags = SI_FAST_MONSTERS | SI_INSTANT_REACTION
  },
};

const skill_info_t hexen_skill_infos[5] = {
  {
    .ammo_factor = FRACUNIT * 3 / 2,
    .damage_factor = FRACUNIT / 2,
    .spawn_filter = 1,
    .flags = SI_AUTO_USE_HEALTH
  },
  {
    .spawn_filter = 2,
    .flags = 0
  },
  {
    .spawn_filter = 3,
    .flags = 0
  },
  {
    .spawn_filter = 4,
    .flags = 0
  },
  {
    .ammo_factor = FRACUNIT * 3 / 2,
    .spawn_filter = 5,
    .flags = SI_FAST_MONSTERS | SI_INSTANT_REACTION
  },
};

int num_skills;
skill_info_t* skill_infos;

static void dsda_CopyFactor(fixed_t* dest, const char* source) {
  // We will compute integers with these,
  // and common human values (i.e., not powers of 2) are rounded down.
  // Add 1 (= 1/2^16) to yield expected results.
  // Otherwise, 0.2 * 15 would be 2 instead of 3.

  if (source)
    *dest = dsda_StringToFixed(source) + 1;
}

void dsda_CopySkillInfo(int i, const doom_mapinfo_skill_t* info) {
  memset(&skill_infos[i], 0, sizeof(skill_infos[i]));

  dsda_CopyFactor(&skill_infos[i].ammo_factor, info->ammo_factor);
  dsda_CopyFactor(&skill_infos[i].damage_factor, info->damage_factor);
  dsda_CopyFactor(&skill_infos[i].armor_factor, info->armor_factor);
  dsda_CopyFactor(&skill_infos[i].health_factor, info->health_factor);
  dsda_CopyFactor(&skill_infos[i].monster_health_factor, info->monster_health_factor);
  dsda_CopyFactor(&skill_infos[i].friend_health_factor, info->friend_health_factor);

  skill_infos[i].respawn_time = info->respawn_time;
  skill_infos[i].spawn_filter = info->spawn_filter;
  skill_infos[i].key = info->key;

  if (info->must_confirm)
    skill_infos[i].must_confirm = Z_Strdup(info->must_confirm);

  if (info->name)
    skill_infos[i].name = Z_Strdup(info->name);

  if (info->pic_name)
    skill_infos[i].pic_name = Z_Strdup(info->pic_name);

  if (info->text_color)
    skill_infos[i].text_color = dsda_ColorNameToIndex(info->text_color);

  skill_infos[i].flags = info->flags;
}

void dsda_InitSkills(void) {
  int i = 0;
  int j;
  int original_skill_list;
  dboolean clear_skills;
  doom_mapinfo_t mapinfo;

  // Check for / parse new skill lumps
  dsda_LoadSkillDef();

  mapinfo = skilldef ? skilldef_info : doom_mapinfo;

  clear_skills = (mapinfo.num_skills && mapinfo.skills_cleared);

  num_skills = (clear_skills ? 0 : 5) + (int)mapinfo.num_skills + uvplus - doom_v11;

  original_skill_list = doom_v11 ? 4 : 5;

  skill_infos = Z_Calloc(num_skills, sizeof(*skill_infos));

  if (!clear_skills) {
    const skill_info_t* original_skill_infos;

    original_skill_infos = hexen   ? hexen_skill_infos   :
                           heretic ? heretic_skill_infos :
                                     doom_skill_infos;

    for (i = 0; i < original_skill_list; ++i)
      skill_infos[i] = original_skill_infos[i];

    if (uvplus)
      skill_infos[5] = uvplus_skill_infos[0];
  }

  for (j = 0; j < mapinfo.num_skills; ++j) {
    if (!stricmp(mapinfo.skills[j].unique_id, "baby")) {
      dsda_CopySkillInfo(0, &mapinfo.skills[j]);
      --i;
      --num_skills;
    }
    else if (!stricmp(mapinfo.skills[j].unique_id, "easy")) {
      dsda_CopySkillInfo(1, &mapinfo.skills[j]);
      --i;
      --num_skills;
    }
    else if (!stricmp(mapinfo.skills[j].unique_id, "normal")) {
      dsda_CopySkillInfo(2, &mapinfo.skills[j]);
      --i;
      --num_skills;
    }
    else if (!stricmp(mapinfo.skills[j].unique_id, "hard")) {
      dsda_CopySkillInfo(3, &mapinfo.skills[j]);
      --i;
      --num_skills;
    }
    else if (!stricmp(mapinfo.skills[j].unique_id, "nightmare")) {
      dsda_CopySkillInfo(4, &mapinfo.skills[j]);
      --i;
      --num_skills;
    }
    else
      dsda_CopySkillInfo(i + j, &mapinfo.skills[j]);
  }
}

void dsda_RefreshPistolStart(void)
{
  if (allow_incompatibility || in_game)
    if(dsda_IntConfig(dsda_config_always_pistol_start) && !dsda_IntConfig(dsda_config_pistol_start))
      dsda_UpdateIntConfig(dsda_config_always_pistol_start, 0, true);
  dsda_TrackGameFlags();
}

void dsda_RefreshAlwaysPistolStart(void)
{
  if (allow_incompatibility || in_game)
    if(dsda_IntConfig(dsda_config_always_pistol_start) && !dsda_IntConfig(dsda_config_pistol_start))
      dsda_UpdateIntConfig(dsda_config_pistol_start, 1, true);
  dsda_TrackGameFlags();
}

static void dsda_ResetGameFlags(void)
{
  respawnparm = (allow_incompatibility ? dsda_IntConfig(dsda_config_respawn_monsters) : dsda_Flag(dsda_arg_respawn));
  fastparm = (allow_incompatibility ? dsda_IntConfig(dsda_config_fast_monsters) : dsda_Flag(dsda_arg_fast));
  nomonsters = (allow_incompatibility ? dsda_IntConfig(dsda_config_no_monsters) : dsda_Flag(dsda_arg_nomonsters));
  coop_spawns = (allow_incompatibility ? dsda_IntConfig(dsda_config_coop_spawns) : dsda_Flag(dsda_arg_coop_spawns));
  dsda_TrackGameFlags();
}

int track_pistolstart;
int track_respawnparm;
int track_fastparm;
int track_nomonsters;
int track_coop_spawns;

void dsda_TrackGameFlags(void)
{
  track_pistolstart = dsda_IntConfig(dsda_config_pistol_start);
  track_respawnparm = dsda_IntConfig(dsda_config_respawn_monsters);
  track_fastparm = dsda_IntConfig(dsda_config_fast_monsters);
  track_nomonsters = dsda_IntConfig(dsda_config_no_monsters);
  track_coop_spawns = dsda_IntConfig(dsda_config_coop_spawns);
}

void dsda_RefreshGameSkill(void) {
  void G_RefreshFastMonsters(void);

  if (allow_incompatibility)
    dsda_ResetGameFlags();

  skill_info = skill_infos[gameskill];

  if (respawnparm && !skill_info.respawn_time)
    skill_info.respawn_time = 12;

  if (fastparm)
    skill_info.flags |= SI_FAST_MONSTERS;

  if (coop_spawns)
    skill_info.flags |= SI_SPAWN_MULTI;

  G_RefreshFastMonsters();
}

void dsda_UpdateGameSkill(int skill) {
  if (skill > num_skills - 1)
    skill = num_skills - 1;

  gameskill = skill;
  dsda_RefreshGameSkill();
}

void dsda_AlterGameFlags(void)
{
  if (!allow_incompatibility || !in_game)
    return;

  dsda_RefreshGameSkill();
}

void dsda_LoadSkillDef(void) {
  int p;

  //if (started_demo)
    //return;

  if (raven || doom_v11 || netgame || dsda_UseMapinfo())
    return;

  if (W_LumpNameExists("M_UVPLUS"))
    uvplus = true;
  
  if (!W_LumpNameExists("SKILLDEF"))
    return;

  skilldef = true;
  uvplus = false;

  p = -1;
  while ((p = W_ListNumFromName("SKILLDEF", p)) >= 0) {
    const unsigned char* lump = (const unsigned char *) W_LumpByNum(p);
    dsda_LoadSkillDefLump(lump, W_LumpLength(p), I_Error);
  }
}
