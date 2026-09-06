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
//  DSDA GAMEINFO
//

#include <string.h>

extern "C" {
#include "d_main.h"
#include "w_wad.h"
#include "lprintf.h"
#include "z_zone.h"
#include "doomstat.h"
#include "v_video.h"
}

#include "scanner.h"

#include "gameinfo.h"

struct gameinfo_t {
  dsda_startup_type_t startup_type;
  char *startup_song;
  char *startup_title;
  dsda_gameinfo_color_t startup_foreground;
  dsda_gameinfo_color_t startup_background;
  dboolean startup_colors_set;
  dboolean loaded;
};

static gameinfo_t gameinfo;

static void dsda_ParseGameInfoLine(Scanner &scanner, dboolean parse_iwad_only) {

  if (!scanner.CheckString()) {
    scanner.GetNextToken();
    scanner.SkipLine();
    return;
  }

  if (!stricmp(scanner.string, "IWAD")) {
    scanner.MustGetToken('=');
    scanner.MustGetString();

    if (iwadlump)
      Z_Free(iwadlump);

    iwadlump = Z_Strdup(scanner.string);
  }
  // On initial pass, do not read anything but IWAD.
  // On second pass, we then have access to nyan-doom.wad for STARTUP colours.
  else if (parse_iwad_only) {
    scanner.SkipLine();
  }
  else if (!stricmp(scanner.string, "STARTUPSONG")) {
    scanner.MustGetToken('=');
    scanner.MustGetString();

    Z_Free(gameinfo.startup_song);
    gameinfo.startup_song = Z_Strdup(scanner.string);
  }
  else if (!stricmp(scanner.string, "STARTUPTITLE")) {
    scanner.MustGetToken('=');
    scanner.MustGetString();

    Z_Free(gameinfo.startup_title);
    gameinfo.startup_title = Z_Strdup(scanner.string);
  }
  else if (!stricmp(scanner.string, "STARTUPTYPE")) {
    scanner.MustGetToken('=');
    scanner.MustGetString();

    if (!stricmp(scanner.string, "Doom"))
      gameinfo.startup_type = dsda_startup_doom;
    else if (!stricmp(scanner.string, "Heretic"))
      gameinfo.startup_type = dsda_startup_heretic;
    else if (!stricmp(scanner.string, "Hexen"))
      gameinfo.startup_type = dsda_startup_hexen;
    else if (!stricmp(scanner.string, "Strife"))
      gameinfo.startup_type = dsda_startup_strife;
    else
      lprintf(LO_WARN, "GAMEINFO: Unknown STARTUPTYPE '%s'.\n", scanner.string);
  }
  else if (!stricmp(scanner.string, "STARTUPCOLORS")) {
    scanner.MustGetToken('=');
    scanner.MustGetString();
    V_ZDoomGetColor(scanner.string, &gameinfo.startup_foreground.r, &gameinfo.startup_foreground.g, &gameinfo.startup_foreground.b);

    scanner.MustGetToken(',');
    scanner.MustGetString();
    V_ZDoomGetColor(scanner.string, &gameinfo.startup_background.r, &gameinfo.startup_background.g, &gameinfo.startup_background.b);

    gameinfo.startup_colors_set = true;
  }
}

// [AR] Called twice: first to determine the IWAD, then to parse the full GAMEINFO.
// STARTUP colors are parsed later because they may require X11R6RGB from nyan-doom.wad.
void dsda_LoadGameInfo(void) {
  dboolean parse_iwad_only = !MainLumpCache;
  int lump;

  if (gameinfo.loaded)
    return;

  lump = W_CheckNumForName("GAMEINFO");

  if (lump == LUMP_NOT_FOUND)
    return;

  if (!parse_iwad_only)
    gameinfo.loaded = true;

  Scanner scanner((const char*) W_LumpByNum(lump), W_LumpLength(lump));

  scanner.SetErrorCallback(I_Error);

  while (scanner.TokensLeft())
    dsda_ParseGameInfoLine(scanner, parse_iwad_only);
}

// these colour default values are the same as ZDoom
static const dsda_gameinfo_color_t startup_doom_fg    = { 168,   0,   0 }; // Dark red    (#A80000)
static const dsda_gameinfo_color_t startup_doom_bg    = { 168, 168, 168 }; // Grey        (#A8A8A8)
static const dsda_gameinfo_color_t startup_heretic_fg = { 252, 252,   0 }; // Yellow      (#FCFC00)
static const dsda_gameinfo_color_t startup_heretic_bg = { 168,   0,   0 }; // Dark red    (#A80000)
static const dsda_gameinfo_color_t startup_hexen_fg   = { 240, 240, 240 }; // Light grey  (#F0F0F0)
static const dsda_gameinfo_color_t startup_hexen_bg   = { 107,  60,  24 }; // Brown       (#6B3C18)

void dsda_SetGameInfoStartupDefaults(const char *gameinfo_title) {
  if (!gameinfo.loaded)
    return;

  if (!gameinfo.startup_title && gameinfo_title)
    gameinfo.startup_title = Z_Strdup(gameinfo_title);

  if (!gameinfo.startup_colors_set) {
    gameinfo.startup_foreground = hexen ? startup_hexen_fg : heretic ? startup_heretic_fg : startup_doom_fg;
    gameinfo.startup_background = hexen ? startup_hexen_bg : heretic ? startup_heretic_bg : startup_doom_bg;
    gameinfo.startup_colors_set = true;
  }
}

dsda_startup_type_t dsda_GameInfoStartupType(void) {
  return gameinfo.startup_type;
}

const char *dsda_GameInfoStartupSong(void) {
  return gameinfo.startup_song;
}

const char *dsda_GameInfoStartupTitle(void) {
  return gameinfo.startup_title;
}

int dsda_GameInfoStartupColors(dsda_gameinfo_color_t *foreground, dsda_gameinfo_color_t *background) {
  if (!gameinfo.startup_colors_set)
    return false;

  *foreground = gameinfo.startup_foreground;
  *background = gameinfo.startup_background;
  return true;
}
