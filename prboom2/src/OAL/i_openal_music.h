//
// Copyright(C) 2023 by Roman Fomin
// Copyright(C) 2026 by Andrik Powell
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
//	OpenAL music
//

#ifndef I_OPENAL_MUSIC_H
#define I_OPENAL_MUSIC_H

#include "doomtype.h"

typedef void (*openal_music_callback_t)(unsigned char *buffer, int length);

dboolean I_StartOpenALMusic(int sample_rate, openal_music_callback_t callback);
void I_ConfigureOpenALMusicGain(dboolean enabled, int volume);
void I_ResetOpenALMusicGain(void);
void I_StopOpenALMusic(void);

#endif
