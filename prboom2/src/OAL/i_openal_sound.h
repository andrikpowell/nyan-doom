//
// Copyright(C) 2023 Roman Fomin
// Copyright(C) 2023 ceski
// Copyright(C) 2026 by Andrik Powell
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// DESCRIPTION:
//	OpenAL sound effect sources
//

#ifndef I_OPENAL_SOUND_H
#define I_OPENAL_SOUND_H

#include "al.h"

#include "doomtype.h"

dboolean I_OAL_InitSound(int channel_count);
void I_OAL_ShutdownSound(void);
void I_OAL_SetResampler(void);
void I_OAL_ResetSource2D(int channel);

ALuint I_OAL_CacheSound(int sound_id, const void *data, size_t length, int bits, int sample_rate);
dboolean I_OAL_StartSound(int channel, int sound_id, const void *data, size_t length,
                          int bits, int sample_rate, dboolean loop, float pitch);
void I_OAL_StopSound(int channel);
dboolean I_OAL_SoundIsPlaying(int channel);
void I_OAL_SetGain(int channel, float gain);
void I_OAL_SetVolume(int channel, int volume);
void I_OAL_SetPan(int channel, int separation);

#endif
