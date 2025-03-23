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
//	NYAN Random Music
//

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "doomstat.h"
#include "g_game.h"
#include "m_misc.h"
#include "s_sound.h"
#include "w_wad.h"
#include "lprintf.h"

#include "dsda/utility.h"

#include "s_randommus.h"

dboolean CurrentRandomMusic;
dboolean RandomMusicLoad;
int last_random[6];

int S_RandomMusic(void)
{
  int epsd, map, random_map, found_map;
  char *mapname;

  DO_ONCE
  srand ( time(NULL) );
  END_ONCE

  found_map = false;

  while (!found_map)
  {
    random_map = rand() % 99;
    epsd = (random_map / 10) % 10;
    map = (gamemode == commercial) ? random_map : random_map % 10;
    mapname = VANILLA_MAP_LUMP_NAME(epsd, map);

    if (W_LumpNameExists(mapname))
    {
      int already_played = 0;

      for (int i = 0; i < 6; i++)
        if (random_map == last_random[i])
          already_played++;

      if (!already_played)
      {
        for (int j = 4; j >= 0; j--)
          last_random[j+1] = last_random[j];
        last_random[0] = random_map;
        found_map = true;
      }
    }
  }
  
  return random_map;
}

void S_ResetRandomMusic(void)
{
  CurrentRandomMusic = false;
  RandomMusicLoad = false;
}

void S_PlayRandomMusic(dboolean cheat)
{
  int epsd = gameepisode;
  int map = gamemap;

  while (epsd == gameepisode && map == gamemap)
  {
    int random_map = CurrentRandomMusic = S_RandomMusic();
    epsd = (random_map / 10) % 10;
    map = (gamemode == commercial) ? random_map : random_map % 10;
  }

  dsda_ChangeMusic(epsd, map, true, cheat);
}

void S_GetRandomMusic(void)
{
  int epsd = gameepisode;
  int map = gamemap;

  if (dsda_IntConfig(nyan_config_play_random_music))
  {
    epsd = (CurrentRandomMusic / 10) % 10;
    map = (gamemode == commercial) ? CurrentRandomMusic : CurrentRandomMusic % 10;
  }

  dsda_ChangeMusic(epsd, map, true, false);
}

void S_ToggleRandomMusic(void)
{
  if (in_game)
    if (dsda_IntConfig(nyan_config_play_random_music))
    {
      S_PlayRandomMusic(false);
      doom_printf("Playing random music");
    }
    else
    {
      dsda_ChangeMusic(gameepisode, gamemap, false, false);
      doom_printf("Playing default music");
    }
}

void S_LevelLoadRandomMusic(void)
{
  S_PlayRandomMusic(false);
}
