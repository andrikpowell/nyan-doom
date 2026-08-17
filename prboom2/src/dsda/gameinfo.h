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

#ifndef __GAMEINFO__
#define __GAMEINFO__

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  dsda_startup_default,
  dsda_startup_doom,    // skip because this requires support for PNG and other formats
  dsda_startup_heretic,
  dsda_startup_hexen,
  dsda_startup_strife,  // skip because we don't have Strife support
  dsda_startup_none     // [internal only] don't show startup
} dsda_startup_type_t;

typedef struct {
  int r;
  int g;
  int b;
} dsda_gameinfo_color_t;

void dsda_LoadGameInfo(void);
dsda_startup_type_t dsda_GameInfoStartupType(void);
const char *dsda_GameInfoStartupSong(void);
const char *dsda_GameInfoStartupTitle(void);
int dsda_GameInfoStartupColors(dsda_gameinfo_color_t *foreground, dsda_gameinfo_color_t *background);
void dsda_SetGameInfoStartupDefaults(const char *gameinfo_title);

#ifdef __cplusplus
}
#endif

#endif
