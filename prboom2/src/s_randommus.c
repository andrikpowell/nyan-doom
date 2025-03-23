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

#include "doomstat.h"
#include "g_game.h"
#include "m_misc.h"
#include "s_sound.h"
#include "lprintf.h"

#include "s_randommus.h"

dboolean CurrentRandomMusic;
dboolean RandomMusicLoad;

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
