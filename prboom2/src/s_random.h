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

#ifndef __S_RANDOM__
#define __S_RANDOM__

extern dboolean CurrentRandomMusic;
extern dboolean RandomMusicLoad;
extern int last_random[6];

extern int S_RandomMusic(void);
void S_ResetRandomMusic(void);
void S_PlayRandomMusic(dboolean cheat);
void S_GetRandomMusic(void);
void S_ToggleRandomMusic(void);
void S_LevelLoadRandomMusic(void);

#endif
