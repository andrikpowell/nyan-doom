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

#ifndef __S_RANDOMMUS__
#define __S_RANDOMMUS__

#include "sounds.h"

extern dboolean CurrentRandomMusic;
extern dboolean RandomMusicLoad;

void S_ResetRandomMusic(void);
void S_PlayRandomMusic(dboolean cheat);
void S_GetRandomMusic(void);
void S_ToggleRandomMusic(void);
void S_LevelLoadRandomMusic(void);

#endif
