//
// Copyright(C) 2022 by Ryan Krafnick
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
//	DSDA MapInfo Legacy
//

#include "doomstat.h"
#include "deh/strings.h"
#include "g_game.h"
#include "m_misc.h"
#include "p_setup.h"
#include "r_data.h"
#include "s_sound.h"
#include "sounds.h"
#include "w_wad.h"
#include "d_englsh.h"

#include "dsda/map_format.h"
#include "dsda/mapinfo.h"
#include "dsda/preferences.h"

#include "legacy.h"

int dsda_LegacyNameToMap(int* found, const char* name, int* episode, int* map) {
  char name_upper[9];
  int episode_from_name = -1;
  int map_from_name = -1;

  if (strlen(name) > 8) {
    *found = false;

    return true;
  }

  strncpy(name_upper, name, 8);
  name_upper[8] = 0;
  M_Strupr(name_upper);

  if (gamemode != commercial) {
    if (sscanf(name_upper, "E%dM%d", &episode_from_name, &map_from_name) != 2) {
      *found = false;

      return true;
    }
  }
  else {
    if (sscanf(name_upper, "MAP%d", &map_from_name) != 1) {
      *found = false;

      return true;
    }

    episode_from_name = 1;
  }

  *found = true;

  *episode = episode_from_name;
  *map = map_from_name;

  return true;
}

int dsda_LegacyFirstMap(int* episode, int* map) {
  int i, j, lump;

  *episode = 1;
  *map = 1;

  if (gamemode == commercial) {
    for (i = 1; i < 33; i++) {
      lump = W_CheckNumForName(VANILLA_MAP_LUMP_NAME(1, i));

      if (lump != LUMP_NOT_FOUND && lumpinfo[lump].source == source_pwad) {
        *map = i;

        return true;
      }
    }
  }
  else
    for (i = 1; i < 5; i++)
      for (j = 1; j < 10; j++) {
        lump = W_CheckNumForName(VANILLA_MAP_LUMP_NAME(i, j));

        if (lump != LUMP_NOT_FOUND && lumpinfo[lump].source == source_pwad) {
          *episode = i;
          *map = j;

          return true;
        }
      }

  return true;
}

int dsda_LegacyNewGameMap(int* episode, int* map) {
  return true;
}

int dsda_LegacyResolveWarp(int* args, int arg_count, int* episode, int* map) {
  if (gamemode == commercial)
  {
    if (arg_count) {
      *episode = 1;
      *map = args[0];
    }
  }
  else if (arg_count) {
    *episode = args[0];

    if (arg_count > 1)
      *map = args[1];
    else
      *map = 1;
  }

  return true;
}

int dsda_LegacyNextMap(int* episode, int* map) {
  static byte doom2_next[33] = {
    2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
    12, 13, 14, 15, 31, 17, 18, 19, 20, 21,
    22, 23, 24, 25, 26, 27, 28, 29, 30, 1,
    32, 16, 3
  };
  static byte doom_next[4][9] = {
    { 12, 13, 19, 15, 16, 17, 18, 21, 14 },
    { 22, 23, 24, 25, 29, 27, 28, 31, 26 },
    { 32, 33, 34, 35, 36, 39, 38, 41, 37 },
    { 42, 49, 44, 45, 46, 47, 48, 11, 43 }
  };
  static byte heretic_next[6][9] = {
    { 12, 13, 14, 15, 16, 19, 18, 21, 17 },
    { 22, 23, 24, 29, 26, 27, 28, 31, 25 },
    { 32, 33, 34, 39, 36, 37, 38, 41, 35 },
    { 42, 43, 44, 49, 46, 47, 48, 51, 45 },
    { 52, 53, 59, 55, 56, 57, 58, 61, 54 },
    { 62, 63, 11, 11, 11, 11, 11, 11, 11 }, // E6M4-E6M9 shouldn't be accessible
  };

  // next arrays are 0-based, unlike gameepisode and gamemap
  *episode = gameepisode - 1;
  *map = gamemap - 1;

  if (heretic) {
    int next;

    if (gamemode == shareware)
      heretic_next[0][7] = 11;

    if (gamemode == registered)
      heretic_next[2][7] = 11;

    next = heretic_next[BETWEEN(0, 5, *episode)][BETWEEN(0, 8, *map)];
    *episode = next / 10;
    *map = next % 10;
  }
  else if (gamemode == commercial) {
    // secret level
    doom2_next[14] = (haswolflevels ? 31 : 16);

    if (bfgedition && allow_incompatibility) {
      if (gamemission == pack_nerve) {
        doom2_next[3] = 9;
        doom2_next[7] = 1;
        doom2_next[8] = 5;
      }
      else
        doom2_next[1] = 33;
    }

    *episode = 1;
    *map = doom2_next[BETWEEN(0, 32, *map)];
  }
  else {
    int next;

    // shareware doom has only episode 1
    doom_next[0][7] = (gamemode == shareware ? 11 : 21);

    doom_next[2][7] = // the fourth episode for pre-ultimate complevels is not allowed
      ((gamemode == registered) || (compatibility_level < ultdoom_compatibility) ? 11 : 41);

    next = doom_next[BETWEEN(0, 3, *episode)][BETWEEN(0, 9, *map)];
    *episode = next / 10;
    *map = next % 10;
  }

  return true;
}

int dsda_LegacyPrevMap(int* episode, int* map) {
  static byte doom2_prev[33] = {
    1, 1, 2, 3, 4, 5, 6, 7, 8, 9,
    10, 11, 12, 13, 14, 32, 16, 17, 18, 19,
    20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
    15, 31, 2
  };
  static byte doom_prev[4][9] = {
    { 11, 11, 12, 19, 14, 15, 16, 17, 13 },
    { 18, 21, 22, 23, 24, 29, 26, 27, 25 },
    { 28, 31, 32, 33, 34, 35, 39, 37, 36 },
    { 38, 41, 49, 43, 44, 45, 46, 47, 42 }
  };
  static byte heretic_prev[6][9] = {
    { 11, 11, 12, 13, 14, 15, 19, 17, 16 },
    { 18, 21, 22, 23, 29, 25, 26, 27, 24 },
    { 28, 31, 32, 33, 39, 35, 36, 37, 34 },
    { 38, 41, 42, 43, 49, 45, 46, 47, 44 },
    { 48, 51, 52, 59, 54, 55, 56, 57, 53 },
    { 58, 61, 62, 63, 63, 63, 63, 63, 63 }, // E6M4-E6M9 shouldn't be accessible
  };

  // next arrays are 0-based, unlike gameepisode and gamemap
  *episode = gameepisode - 1;
  *map = gamemap - 1;

  if (heretic) {
    int prev;

    prev = heretic_prev[BETWEEN(0, 5, *episode)][BETWEEN(0, 8, *map)];
    *episode = prev / 10;
    *map = prev % 10;
  }
  else if (gamemode == commercial) {
    // secret level
    doom2_prev[15] = (haswolflevels ? 32 : 15);

    if (bfgedition && allow_incompatibility) {
      if (gamemission == pack_nerve) {
        doom2_prev[4] = 9;
        doom2_prev[8] = 4;
      }
      else
        doom2_prev[2] = 33;
    }

    *episode = 1;
    *map = doom2_prev[BETWEEN(0, 32, *map)];
  }
  else {
    int prev;

    prev = doom_prev[BETWEEN(0, 3, *episode)][BETWEEN(0, 9, *map)];
    *episode = prev / 10;
    *map = prev % 10;
  }

  return true;
}

int dsda_LegacyShowNextLocBehaviour(int* behaviour) {
  if (
    gamemode != commercial &&
    (gamemap == 8 || (chex_exe && gamemap == 5))
  )
    *behaviour = WI_SHOW_NEXT_DONE;
  else
    *behaviour = WI_SHOW_NEXT_LOC;

  if (dsda_FinaleShortcut())
    *behaviour = WI_SHOW_NEXT_DONE;

  return true;
}

int dsda_LegacySkipDrawShowNextLoc(int* skip) {
  *skip = false;

  if (dsda_FinaleShortcut())
    *skip = true;

  return true;
}

void dsda_LegacyUpdateMapInfo(void) {
  // nothing to do right now
}

void dsda_LegacyUpdateLastMapInfo(void) {
  // nothing to do right now
}

void dsda_LegacyUpdateNextMapInfo(void) {
  // nothing to do right now
}

static int dsda_CannotCLEV(int episode, int map) {
  char* next;

  if (
    episode < 1 ||
    map < 0 ||
    ((gamemode == retail || gamemode == registered) && (episode > 9 || map > 9)) ||
    (gamemode == shareware && (episode > 1 || map > 9)) ||
    (gamemode == commercial && (episode > 1 || map > 99)) ||
    (gamemission == pack_nerve && map > 9)
  ) return true;

  // Catch invalid maps
  next = VANILLA_MAP_LUMP_NAME(episode, map);
  if (!W_LumpNameExists(next)) {
    doom_printf("IDCLEV target not found: %s", next);
    return true;
  }

  return false;
}

int dsda_LegacyResolveCLEV(int* clev, int* episode, int* map) {
  if (dsda_CannotCLEV(*episode, *map))
    *clev = false;
  else {
    if (chex_exe)
      *episode = 1;

    *clev = true;
  }

  return true;
}

int dsda_LegacyResolveINIT(int* init) {
  *init = false;

  return true;
}

const char* dsda_HereticReplaceMusicName(int music_index) {
  if (S_music[music_index].name_rpl != NULL)
  {
    if (W_LumpNameExists(S_music[music_index].name_rpl))
      return S_music[music_index].name_rpl;
  }

  return S_music[music_index].name;
}

int dsda_LegacyMusicIndexToLumpNum(int* lump, int music_index) {
  char name[9];
  const char* format;
  const char* music_name;

  format = raven ? "%s" : "d_%s";
  music_name = S_music[music_index].name;
  
  // Heretic reuses music lumps
  // This allows lumps like "MUS_E4M1" to be used
  if (heretic)
    music_name = dsda_HereticReplaceMusicName(music_index, format);

  snprintf(name, sizeof(name), format, music_name);

  *lump = W_GetNumForName(name);

  return true;
}

static inline int WRAP(int i, int w)
{
  while (i < 0)
    i += w;

  return i % w;
}

int dsda_LegacyMapMusic(int* music_index, int* music_lump, int episode, int map) {
  *music_lump = -1;

  if (idmusnum != -1)
    *music_index = idmusnum; //jff 3/17/98 reload IDMUS music if not -1
  else {
    if (gamemode == commercial)
      *music_index = mus_runnin + WRAP(map - 1, DOOM_MUSINFO - mus_runnin);
    else {
      static const int spmus[] = {
        mus_e3m4,
        mus_e3m2,
        mus_e3m3,
        mus_e1m5,
        mus_e2m7,
        mus_e2m4,
        mus_e2m6,
        mus_e2m5,
        mus_e1m9
      };

      if (heretic)
        *music_index = heretic_mus_e1m1 +
                       WRAP((episode - 1) * 9 + map - 1,
                            HERETIC_NUMMUSIC - heretic_mus_e1m1);
      else if (episode < 4)
        *music_index = mus_e1m1 +
                       WRAP((episode - 1) * 9 + map - 1, mus_runnin - mus_e1m1);
      else
        *music_index = spmus[WRAP(map - 1, 9)];
    }
  }

  return true;
}

int dsda_LegacyIntermissionMusic(int* music_index, int* music_lump) {
  *music_lump = -1;

  if (gamemode == commercial)
    *music_index = mus_dm2int;
  else
    *music_index = mus_inter;

  return true;
}

int dsda_LegacyInterMusic(int* music_index, int* music_lump) {
  *music_lump = -1;

  switch (gamemode) {
    case shareware:
    case registered:
    case retail:
      *music_index = mus_victor;
      break;
    default:
      *music_index = mus_read_m;
      break;
  }

  return true;
}

extern int UMAPINFO_Text;

// Checks whether intermission text matches original text and if new text is provided from PWAD.
// used for the skip intermission config option.
int dsda_LegacyCheckInterText(void)
{
    int SkipText = false;
    int MapLump = 0;

    // Disable check for ZDoom (MAPINFO)
    if (netgame || map_format.zdoom || dsda_UseMapinfo())
      return false;

    // Disable check if UMAPINFO has text
    if (UMAPINFO_Text)
      return false;

    switch (gamemode)
    {
        // DOOM 1 - E1, E3 or E4, but each nine missions
        case shareware:
        case registered:
        case retail:
        {
            switch (gameepisode)
            {
            case 1:
                MapLump = W_GetNumForName("E1M8");
                if (!strcmp(E1TEXT, s_E1TEXT)) SkipText = true;
                break;
            case 2:
                MapLump = W_GetNumForName("E2M8");
                if (!strcmp(E2TEXT, s_E2TEXT)) SkipText = true;
                break;
            case 3:
                MapLump = W_GetNumForName("E3M8");
                if (!strcmp(E3TEXT, s_E3TEXT)) SkipText = true;
                break;
            case 4:
                MapLump = W_GetNumForName("E4M8");
                if (!strcmp(E4TEXT, s_E4TEXT)) SkipText = true;
                break;
            default:
                break;
            }
            break;
        }

        // DOOM II and missions packs with E1, M34
        case commercial:
        {
            switch (gamemap)
            {
            case 6:
                MapLump = W_GetNumForName("MAP06");
                if (gamemission == pack_tnt) {
                    if (!strcmp(T1TEXT, s_T1TEXT))
                        SkipText = true;
                }
                else if (gamemission == pack_plut) {
                    if (!strcmp(P1TEXT, s_P1TEXT))
                        SkipText = true;
                }
                else {
                    if (!strcmp(C1TEXT, s_C1TEXT))
                        SkipText = true;
                }
                break;
            case 11:
                MapLump = W_GetNumForName("MAP11");
                if (gamemission == pack_tnt) {
                    if (!strcmp(T2TEXT, s_T2TEXT))
                        SkipText = true;
                }
                else if (gamemission == pack_plut) {
                    if (!strcmp(P2TEXT, s_P2TEXT))
                        SkipText = true;
                }
                else {
                    if (!strcmp(C2TEXT, s_C2TEXT))
                        SkipText = true;
                }
                break;
            case 20:
                MapLump = W_GetNumForName("MAP20");
                if (gamemission == pack_tnt) {
                    if (!strcmp(T3TEXT, s_T3TEXT))
                        SkipText = true;
                }
                else if (gamemission == pack_plut) {
                    if (!strcmp(P3TEXT, s_P3TEXT))
                        SkipText = true;
                }
                else {
                    if (!strcmp(C3TEXT, s_C3TEXT))
                        SkipText = true;
                }
                break;
            case 30:
                MapLump = W_GetNumForName("MAP30");
                if (gamemission == pack_tnt) {
                    if (!strcmp(T4TEXT, s_T4TEXT))
                        SkipText = true;
                }
                else if (gamemission == pack_plut) {
                    if (!strcmp(P4TEXT, s_P4TEXT))
                        SkipText = true;
                }
                else {
                    if (!strcmp(C4TEXT, s_C4TEXT))
                        SkipText = true;
                }
                break;
            case 15:
                MapLump = W_GetNumForName("MAP15");
                if (gamemission == pack_tnt) {
                    if (!strcmp(T5TEXT, s_T5TEXT))
                        SkipText = true;
                }
                else if (gamemission == pack_plut) {
                    if (!strcmp(P5TEXT, s_P5TEXT))
                        SkipText = true;
                }
                else {
                    if (!strcmp(C5TEXT, s_C5TEXT))
                        SkipText = true;
                }
                break;
            case 31:
                MapLump = W_GetNumForName("MAP31");
                if (gamemission == pack_tnt) {
                    if (!strcmp(T6TEXT, s_T6TEXT))
                        SkipText = true;
                }
                else if (gamemission == pack_plut) {
                    if (!strcmp(P6TEXT, s_P6TEXT))
                        SkipText = true;
                }
                else {
                    if (!strcmp(C6TEXT, s_C6TEXT))
                        SkipText = true;
                }
                break;
            default:
                break;
            }
            if (gamemission == pack_nerve && gamemap == 8)
            {
                MapLump = W_GetNumForName("MAP08");
                if (!strcmp(C6TEXT, s_C6TEXT)) SkipText = true;
            }
            break;
        }

        default:
            break;
    }

    // Only skip text if map before story screen is replaced by PWAD
    if (!W_PWADLumpNumExists2(MapLump))
      return false;

    return SkipText;
}

int dsda_LegacyStartFinale(void) {
  return true;
}

int dsda_LegacyFTicker(void) {
  return true;
}

void dsda_LegacyFDrawer(void) {
  return;
}

int dsda_LegacyBossAction(mobj_t* mo) {
  return false;
}

int dsda_LegacyMapLumpName(const char** name, int episode, int map) {
  *name = VANILLA_MAP_LUMP_NAME(episode, map);

  return true;
}

int dsda_LegacyMapAuthor(const char** author) {
  *author = NULL;

  return true;
}

int dsda_LegacyGenericMapname(dsda_string_t* str, int epsd, int map) {
  extern char** mapnames[];
  extern char** mapnames2[];
  extern char** mapnamesp[];
  extern char** mapnamest[];
  extern char* og_mapnames[];
  extern char* og_mapnames2[];
  extern char* og_mapnamesp[];
  extern char* og_mapnamest[];

  if (raven) return false;

  if (map > 0 && epsd > 0) {
    char* mapname = VANILLA_MAP_LUMP_NAME(epsd, map);

    if (W_PWADLumpNameExists2(mapname)) {
      int classic_mapname = 0;

      switch (gamemode) {
        case shareware:
        case registered:
        case retail:
          if (epsd < 6 && map < 10)
          {
            if (*mapnames[(epsd - 1) * 9 + map - 1] == og_mapnames[(epsd - 1) * 9 + map - 1])
              classic_mapname++;
          }
          break;

        default:
          if (gamemission == pack_tnt && map < 33)
          {
            if (*mapnamest[map - 1] == og_mapnamest[map - 1])
              classic_mapname++;
          }
          else if (gamemission == pack_plut && map < 33)
          {
            if (*mapnamesp[map - 1] == og_mapnamesp[map - 1])
              classic_mapname++;
          }
          else if (map < 34)
          {
            if (*mapnames2[map - 1] == og_mapnames2[map - 1])
              classic_mapname++;
          }
          break;
      }

      if (classic_mapname > 0)
      {
        if (gamemode == commercial)
          sprintf(mapname, "level %i", map);
        dsda_StringCat(str, mapname);
        return true;
      }
    }
  }

  return false;
}

int dsda_LegacyMapTitle(dsda_string_t* str, int epsd, int map, int override) {
  extern char** mapnames[];
  extern char** mapnames2[];
  extern char** mapnamesp[];
  extern char** mapnamest[];
  extern char * const *LevelNames[];

  dsda_InitString(str, NULL);

  if (override || gamestate == GS_LEVEL)
  {
    if(dsda_IntConfig(nyan_config_ignore_default_map_names) && dsda_LegacyGenericMapname(str, epsd, map))
      return true;

    if (map > 0 && epsd > 0) {
      if (heretic) {
        if (epsd < 6 && map < 10)
          dsda_StringCat(str, *LevelNames[(epsd - 1) * 9 + map - 1]);
      }
      else {
        switch (gamemode) {
          case shareware:
          case registered:
          case retail:
            // Chex.exe always uses the episode 1 level title
            // eg. E2M1 gives the title for E1M1
            if (chex_exe && map < 10)
              dsda_StringCat(str, *mapnames[map - 1]);
            else if (epsd < 6 && map < 10)
              dsda_StringCat(str, *mapnames[(epsd - 1) * 9 + map - 1]);
            break;

          default:  // Ty 08/27/98 - modified to check mission for TNT/Plutonia
            if (gamemission == pack_tnt && map < 33)
              dsda_StringCat(str, *mapnamest[map - 1]);
            else if (gamemission == pack_plut && map < 33)
              dsda_StringCat(str, *mapnamesp[map - 1]);
            else if (map < 34)
              dsda_StringCat(str, *mapnames2[map - 1]);
            break;
        }
      }
    }
  }

  if (!str->string)
    dsda_StringCat(str, VANILLA_MAP_LUMP_NAME(epsd, map));

  return true;
}

int dsda_LegacyHUTitle(dsda_string_t* str) {
  dsda_LegacyMapTitle(str, gameepisode, gamemap, false);
  return true;
}

int dsda_LegacySkyTexture(int skynum, int* sky) {
  if (heretic) {
    static const char *sky_lump_names[5] = {
        "SKY1", "SKY2", "SKY3", "SKY1", "SKY3"
    };

    if (gameepisode < 6)
      *sky = R_TextureNumForName(sky_lump_names[gameepisode - 1]);
    else
      *sky = R_TextureNumForName("SKY1");
  }
  else if (gamemode == commercial) {
    *sky = R_TextureNumForName ("SKY3");
    if (gamemap < 12)
      *sky = R_TextureNumForName ("SKY1");
    else
      if (gamemap < 21)
        *sky = R_TextureNumForName ("SKY2");
  }
  else {
    switch (gameepisode) {
      case 1:
        *sky = R_TextureNumForName ("SKY1");
        break;
      case 2:
        *sky = R_TextureNumForName ("SKY2");
        break;
      case 3:
        *sky = R_TextureNumForName ("SKY3");
        break;
      case 4: // Special Edition sky
        *sky = R_TextureNumForName ("SKY4");
        break;
      default:
        *sky = R_TextureNumForName ("SKY1");
        break;
    }
  }

  return true;
}

int dsda_LegacyPrepareInitNew(void) {
  return true;
}

void dsda_LegacyParTime(int* partime, dboolean* modified) {
  extern int deh_pars;

  if (gamemode == commercial) {
    if (gamemap >= 1 && gamemap <= 34) {
      *partime = TICRATE * cpars[gamemap - 1];
      *modified = deh_pars;
    }
  }
  else {
    if (gameepisode >= 1 && gameepisode <= 4 && gamemap >= 1 && gamemap <= 9) {
      *partime = TICRATE * pars[gameepisode][gamemap];
      *modified = deh_pars;
    }
  }
}

int dsda_LegacyPrepareIntermission(int* result) {
  if (gamemode != commercial)
    if (gamemap == 9) {
      int i;

      for (i = 0; i < g_maxplayers; i++)
        players[i].didsecret = true;
    }

  wminfo.didsecret = players[consoleplayer].didsecret;

  // wminfo.next is 0 biased, unlike gamemap
  if (gamemode == commercial) {
    if (secretexit)
      switch(gamemap) {
        case 15:
          wminfo.next = 30;
          break;
        case 31:
          wminfo.next = 31;
          break;
        case 2:
          if (bfgedition && allow_incompatibility)
            wminfo.next = 32;
          break;
        case 4:
          if (gamemission == pack_nerve && allow_incompatibility)
            wminfo.next = 8;
          break;
      }
    else
      switch(gamemap) {
        case 31:
        case 32:
          wminfo.next = 15;
          break;
        case 33:
          if (bfgedition && allow_incompatibility)
          {
            wminfo.next = 2;
            break;
          }
          // fallthrough
        default:
          wminfo.next = gamemap;
      }

    if (gamemission == pack_nerve && allow_incompatibility && gamemap == 9)
      wminfo.next = 4;
  }
  else {
    if (secretexit)
      wminfo.next = 8; // go to secret level
    else if (gamemap == 9) {
      // returning from secret level
      if (heretic)
      {
        static int after_secret[5] = { 6, 4, 4, 4, 3 };
        wminfo.next = after_secret[gameepisode - 1];
      }
      else
        switch (gameepisode) {
          case 1:
            wminfo.next = 3;
            break;
          case 2:
            wminfo.next = 5;
            break;
          case 3:
            wminfo.next = 6;
            break;
          case 4:
            wminfo.next = 2;
            break;
        }
    }
    else
      wminfo.next = gamemap; // go to next level
  }

  dsda_LegacyParTime(&wminfo.partime, &wminfo.modified_partime);

  if (map_format.zdoom)
    if (leave_data.map > 0)
      wminfo.next = leave_data.map - 1;

  *result = 0;

  return true;
}

int dsda_LegacyPrepareFinale(int* result) {
  *result = 0;

  if (gamemode == commercial && gamemission != pack_nerve) {
    switch (gamemap) {
      case 15:
      case 31:
        if (!secretexit)
          break;
        // fallthrough
      case 6:
      case 11:
      case 20:
      case 30:
        *result = WD_START_FINALE;
        break;
    }
  }
  else if (gamemission == pack_nerve && allow_incompatibility && gamemap == 8)
    *result = WD_START_FINALE;
  else if (gamemap == 8)
    *result = WD_VICTORY;
  else if (gamemap == 5 && chex_exe)
    *result = WD_VICTORY;

  if (dsda_FinaleShortcut())
    *result = WD_START_FINALE;

  return true;
}

void dsda_LegacyLoadMapInfo(void) {
  return;
}

int dsda_LegacyExitPic(const char** exit_pic) {
  *exit_pic = NULL;

  return true;
}

int dsda_LegacyEnterPic(const char** enter_pic) {
  *enter_pic = NULL;

  return true;
}

int dsda_LegacyBorderTexture(const char** border_texture) {
  if (heretic)
    *border_texture = gamemode == shareware ? "FLOOR04" : "FLAT513"; // "FLOOR30"
  else
    *border_texture = gamemode == commercial ? "GRNROCK" : "FLOOR7_2";

  return true;
}

int dsda_LegacyPrepareEntering(void) {
  extern const char *el_levelname;
  extern const char *el_levelpic;
  extern const char *el_author;
  extern int el_classic;

  el_levelname = NULL;
  el_levelpic = NULL;
  el_author = NULL;
  el_classic = true;

  return true;
}

int dsda_LegacyPrepareFinished(void) {
  extern const char *lf_levelname;
  extern const char *lf_levelpic;
  extern const char *lf_author;
  extern int lf_classic;

  lf_levelname = NULL;
  lf_levelpic = NULL;
  lf_author = NULL;
  lf_classic = true;

  return true;
}

int dsda_LegacyMapLightning(int* lightning) {
  *lightning = false;

  return true;
}

int dsda_LegacyApplyFadeTable(void) {
  return true;
}

int dsda_LegacyMapFadeTable(void) {
  return true;
}

int dsda_LegacyMapCluster(int* cluster, int map) {
  *cluster = -1;

  return true;
}

int dsda_LegacySky1Texture(short* texture) {
  *texture = -1;

  return true;
}

int dsda_LegacySky2Texture(short* texture) {
  *texture = -1;

  return true;
}

int dsda_LegacyGravity(fixed_t* gravity) {
  *gravity = FRACUNIT;

  return true;
}

int dsda_LegacyAirControl(fixed_t* air_control) {
  dboolean dsda_AllowJumping(void);

  *air_control = dsda_AllowJumping() ? (FRACUNIT >> 8) : 0;

  return true;
}

int dsda_LegacyInitSky(void) {
  return true;
}

int dsda_LegacyMapFlags(map_info_flags_t* flags) {
  *flags = MI_INTERMISSION |
           MI_ACTIVATE_OWN_DEATH_SPECIALS |
           MI_LAX_MONSTER_ACTIVATION |
           MI_MISSILES_ACTIVATE_IMPACT_LINES |
           MI_SHOW_AUTHOR;

  return true;
}

int dsda_LegacyMapColorMap(int* colormap) {
  *colormap = 0;

  return true;
}
