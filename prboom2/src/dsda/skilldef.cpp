//
// Copyright(C) 2025 by Andrik Powell
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
//  NYAN SKILL
//

#include <cstring>
#include <cstdio>
#include <vector>
#include <string>

extern "C" {
#include "d_main.h"
#include "g_game.h"
#include "w_wad.h"
#include "lprintf.h"
#include "z_zone.h"
#include "skill_info.h"
#include "dsda/utility.h"
}

#include "scanner.h"

#include "mapinfo/doom/parser.h"
#include "skilldef.h"

static std::vector<doom_mapinfo_skill_t> new_skills;

static void dsda_SkipValue(Scanner &scanner) {
  if (scanner.CheckToken('=')) {
    do {
      scanner.GetNextToken();
    } while (scanner.CheckToken(','));

    return;
  }

  if (scanner.CheckToken('{'))
  {
    int brace_count = 1;

    while (scanner.TokensLeft()) {
      if (scanner.CheckToken('}')) {
        --brace_count;
      }
      else if (scanner.CheckToken('{')) {
        ++brace_count;
      }

      if (!brace_count)
        break;

      scanner.GetNextToken();
    }

    return;
  }
}

#define STR_DUP(x) { Z_Free(x); x = Z_Strdup(scanner.string); }
#define RESET_STR(x) { Z_Free(x); x = NULL; }

// The scanner drops the sign when scanning, and we need it back
static void dsda_FloatString(Scanner &scanner, char* &str) {
  if (scanner.decimal >= 0) {
    STR_DUP(str);
  }
  else {
    str = (char*) Z_Realloc(str, strlen(scanner.string) + 2);
    str[0] = '-';
    str[1] = '\0';
    strcat(str, scanner.string);
  }
}

#define SCAN_INT(x)  { scanner.MustGetToken('='); \
                       scanner.MustGetInteger(); \
                       x = scanner.number; }

#define SCAN_FLOAT(x) { scanner.MustGetToken('='); \
                        scanner.MustGetFloat(); \
                        x = scanner.decimal; }

#define SCAN_STRING(x) { scanner.MustGetToken('='); \
                         scanner.MustGetString(); \
                         STR_DUP(x); }

#define SCAN_FLOAT_STRING(x) { scanner.MustGetToken('='); \
                               scanner.MustGetFloat(); \
                               dsda_FloatString(scanner, x); }

static void dsda_FreeSkill(doom_mapinfo_skill_t &skill) {
  Z_Free(skill.unique_id);
  Z_Free(skill.ammo_factor);
  Z_Free(skill.damage_factor);
  Z_Free(skill.armor_factor);
  Z_Free(skill.health_factor);
  Z_Free(skill.monster_health_factor);
  Z_Free(skill.friend_health_factor);
  Z_Free(skill.must_confirm);
  Z_Free(skill.name);
  Z_Free(skill.pic_name);
  Z_Free(skill.text_color);
}

static void dsda_ParseSkills(Scanner &scanner) {
  doom_mapinfo_skill_t skill = { 0 };

  scanner.MustGetString();
  STR_DUP(skill.unique_id);

  scanner.MustGetToken('{');
  while (!scanner.CheckToken('}')) {
    scanner.MustGetToken(TK_Identifier);

    if (scanner.StringMatch("AmmoFactor")) {
      SCAN_FLOAT_STRING(skill.ammo_factor);
    }
    else if (scanner.StringMatch("DamageFactor")) {
      SCAN_FLOAT_STRING(skill.damage_factor);
    }
    else if (scanner.StringMatch("ArmorFactor")) {
      SCAN_FLOAT_STRING(skill.armor_factor);
    }
    else if (scanner.StringMatch("HealthFactor")) {
      SCAN_FLOAT_STRING(skill.health_factor);
    }
    else if (scanner.StringMatch("MonsterHealth")) {
      SCAN_FLOAT_STRING(skill.monster_health_factor);
    }
    else if (scanner.StringMatch("FriendlyHealth")) {
      SCAN_FLOAT_STRING(skill.friend_health_factor);
    }
    else if (scanner.StringMatch("RespawnTime")) {
      SCAN_INT(skill.respawn_time);
    }
    else if (scanner.StringMatch("SpawnFilter")) {
      SCAN_INT(skill.spawn_filter);
    }
    else if (scanner.StringMatch("Key")) {
      scanner.MustGetToken('=');
      scanner.MustGetString();
      skill.key = tolower(scanner.string[0]);
    }
    else if (scanner.StringMatch("MustConfirm")) {
      if (scanner.CheckToken('=')) {
        scanner.MustGetString();
        STR_DUP(skill.must_confirm);
      }

      skill.flags |= DSI_MUST_CONFIRM;
    }
    else if (scanner.StringMatch("Name")) {
      SCAN_STRING(skill.name);
    }
    else if (scanner.StringMatch("PicName")) {
      SCAN_STRING(skill.pic_name);
    }
    else if (scanner.StringMatch("TextColor")) {
      SCAN_STRING(skill.text_color);
    }
    else if (scanner.StringMatch("SpawnMulti")) {
      skill.flags |= DSI_SPAWN_MULTI;
    }
    else if (scanner.StringMatch("FastMonsters")) {
      skill.flags |= DSI_FAST_MONSTERS;
    }
    else if (scanner.StringMatch("InstantReaction")) {
      skill.flags |= DSI_INSTANT_REACTION;
    }
    else if (scanner.StringMatch("NoPain")) {
      skill.flags |= DSI_NO_PAIN;
    }
    else if (scanner.StringMatch("DefaultSkill")) {
      skill.flags |= DSI_DEFAULT_SKILL;
    }
    else if (scanner.StringMatch("PlayerRespawn")) {
      skill.flags |= DSI_PLAYER_RESPAWN;
    }
    else if (scanner.StringMatch("EasyBossBrain")) {
      skill.flags |= DSI_EASY_BOSS_BRAIN;
    }
    else {
      dsda_SkipValue(scanner);
    }
  }

  for (auto &old_skill : new_skills)
    if (!stricmp(old_skill.unique_id, skill.unique_id)) {
      dsda_FreeSkill(old_skill);
      old_skill = skill;

      return;
    }

  new_skills.push_back(skill);
}

doom_mapinfo_t skilldef_info;

void dsda_LoadSkillDefLump(const unsigned char* buffer, size_t length, skilldef_errorfunc err) {
  Scanner scanner((const char*) buffer, length);
  scanner.SetErrorCallback(err);

  while (scanner.TokensLeft())
  {
    scanner.MustGetToken(TK_Identifier);

    if (scanner.StringMatch("skill")) {
      dsda_ParseSkills(scanner);
    }
    else if (scanner.StringMatch("clearskills")) {
      for (auto &skill : new_skills)
        dsda_FreeSkill(skill);
      new_skills.clear();
      skilldef_info.skills_cleared = true;
    }
    else {
      dsda_SkipValue(scanner);
    }
  }

  skilldef_info.num_skills = new_skills.size();
  if(skilldef_info.num_skills > 0) {
    skilldef_info.skills = &new_skills[0];
  } else {
    skilldef_info.skills = NULL;
  }
}
