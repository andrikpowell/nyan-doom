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

#include <string.h>

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

#include "nyanskill.h"


#define STR_DUP(x) { x = Z_Strdup(scanner.string); }
//#define STR_DUP(x) { Z_Free(x); x = Z_Strdup(scanner.string); }

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

static void dsda_CopyFactor(fixed_t* dest, const char* source) {
  // We will compute integers with these,
  // and common human values (i.e., not powers of 2) are rounded down.
  // Add 1 (= 1/2^16) to yield expected results.
  // Otherwise, 0.2 * 15 would be 2 instead of 3.

  if (source)
    *dest = dsda_StringToFixed(source) + 1;
}

void dsda_ParseNyanSKillLine(Scanner &scanner) {
  scanner.MustGetString();

  scanner.MustGetToken('{');
  while (!scanner.CheckToken('}')) {
    scanner.MustGetToken(TK_Identifier);

    if (scanner.StringMatch("AmmoFactor")) {
      char *value;
      SCAN_FLOAT_STRING(value);
      dsda_CopyFactor(&doom_skill_infos[5].ammo_factor, value);
    }
    else if (scanner.StringMatch("DamageFactor")) {
      char *value;
      SCAN_FLOAT_STRING(value);
      dsda_CopyFactor(&doom_skill_infos[5].damage_factor, value);
    }
    else if (scanner.StringMatch("ArmorFactor")) {
      char *value;
      SCAN_FLOAT_STRING(value);
      dsda_CopyFactor(&doom_skill_infos[5].armor_factor, value);
    }
    else if (scanner.StringMatch("HealthFactor")) {
      char *value;
      SCAN_FLOAT_STRING(value);
      dsda_CopyFactor(&doom_skill_infos[5].health_factor, value);
    }
    else if (scanner.StringMatch("MonsterHealth")) {
      char *value;
      SCAN_FLOAT_STRING(value);
      dsda_CopyFactor(&doom_skill_infos[5].monster_health_factor, value);
    }
    else if (scanner.StringMatch("FriendlyHealth")) {
      char *value;
      SCAN_FLOAT_STRING(value);
      dsda_CopyFactor(&doom_skill_infos[5].friend_health_factor, value);
    }
    else if (scanner.StringMatch("RespawnTime")) {
      SCAN_INT(doom_skill_infos[5].respawn_time);
    }
    else if (scanner.StringMatch("SpawnFilter")) {
      SCAN_INT(doom_skill_infos[5].spawn_filter);
    }
    else if (scanner.StringMatch("Key")) {
      scanner.MustGetToken('=');
      scanner.MustGetString();
      doom_skill_infos[5].key = tolower(scanner.string[0]);
    }
    else if (scanner.StringMatch("MustConfirm")) {
      if (scanner.CheckToken('=')) {
        scanner.MustGetString();
        STR_DUP(doom_skill_infos[5].must_confirm);
      }

      doom_skill_infos[5].flags |= SI_MUST_CONFIRM;
    }
    else if (!stricmp(scanner.string, "Name")) {
      SCAN_STRING(doom_skill_infos[5].name);
    }
    else if (!stricmp(scanner.string, "PicName")) {
      SCAN_STRING(doom_skill_infos[5].pic_name);
    }
    else if (scanner.StringMatch("SpawnMulti")) {
      doom_skill_infos[5].flags |= SI_SPAWN_MULTI;
    }
    else if (scanner.StringMatch("FastMonsters")) {
      doom_skill_infos[5].flags |= SI_FAST_MONSTERS;
    }
    else if (scanner.StringMatch("InstantReaction")) {
      doom_skill_infos[5].flags |= SI_INSTANT_REACTION;
    }
    else if (scanner.StringMatch("NoPain")) {
      doom_skill_infos[5].flags |= SI_NO_PAIN;
    }
    else if (scanner.StringMatch("PlayerRespawn")) {
      doom_skill_infos[5].flags |= SI_PLAYER_RESPAWN;
    }
    else if (scanner.StringMatch("EasyBossBrain")) {
      doom_skill_infos[5].flags |= SI_EASY_BOSS_BRAIN;
    }
    else {
      dsda_SkipValue(scanner);
    }
  }
}

void dsda_ResetNyanSkill(void)
{
  doom_skill_infos[5].key = 'x',
  doom_skill_infos[5].name = "New Difficulty.",
  doom_skill_infos[5].pic_name = "M_ULTRA",
  doom_skill_infos[5].flags = 0;
}

void dsda_LoadNyanSkill(void) {
  int lump;

  lump = W_CheckNumForName("NYANSKLL");

  if (lump == LUMP_NOT_FOUND)
    return;

  // Reset New Skill
  dsda_ResetNyanSkill();

  Scanner scanner((const char*) W_LumpByNum(lump), W_LumpLength(lump));

  scanner.SetErrorCallback(I_Error);

  while (scanner.TokensLeft())
  {
    scanner.MustGetToken(TK_Identifier);

    if (scanner.StringMatch("skill")) {
      dsda_ParseNyanSKillLine(scanner);
    }
    else {
      dsda_SkipValue(scanner);
    }
  }
}
