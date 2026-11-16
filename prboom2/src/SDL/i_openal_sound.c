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

#include "al.h"
#include "alext.h"

#include <math.h>
#include <string.h>

#include "i_openal_sound.h"
#include "lprintf.h"
#include "z_zone.h"

#define VOL_TO_GAIN(x) ((ALfloat)(x) / 127)

static ALuint *sound_sources;
static int sound_channel_count;
static dboolean soft_source_spatialize;

typedef struct sound_buffer_s
{
  int sound_id;
  ALuint buffer;
  struct sound_buffer_s *next;
} sound_buffer_t;

static sound_buffer_t *sound_buffers;

void I_OAL_SetResampler(void)
{
  LPALGETSTRINGISOFT get_resampler_name;
  ALint i;
  ALint count;

  if (alIsExtensionPresent("AL_SOFT_source_resampler") != AL_TRUE)
    return;

  get_resampler_name = (LPALGETSTRINGISOFT)alGetProcAddress("alGetStringiSOFT");

  if (!get_resampler_name)
    return;

  count = alGetInteger(AL_NUM_RESAMPLERS_SOFT);
  for (i = 0; i < count; ++i)
  {
    if (!strcmp(get_resampler_name(AL_RESAMPLER_NAME_SOFT, i), "Linear"))
    {
      int channel;

      for (channel = 0; channel < sound_channel_count; ++channel)
        alSourcei(sound_sources[channel], AL_SOURCE_RESAMPLER_SOFT, i);

      return;
    }
  }
}

void I_OAL_ResetSource2D(int channel)
{
  if (!sound_sources)
  {
    return;
  }

  alSource3f(sound_sources[channel], AL_POSITION, 0.0f, 0.0f, 0.0f);
  alSource3f(sound_sources[channel], AL_VELOCITY, 0.0f, 0.0f, 0.0f);
  alSourcef(sound_sources[channel], AL_ROLLOFF_FACTOR, 0.0f);
  alSourcei(sound_sources[channel], AL_SOURCE_RELATIVE, AL_TRUE);
  alSourcei(sound_sources[channel], AL_REFERENCE_DISTANCE, 0);
  alSourcei(sound_sources[channel], AL_MAX_DISTANCE, 0);
}

ALuint I_OAL_CacheSound(int sound_id, const void *data, size_t length, int bits, int sample_rate)
{
  sound_buffer_t *item;
  ALenum format = bits == 16 ? AL_FORMAT_MONO16 : AL_FORMAT_MONO8;

  for (item = sound_buffers; item; item = item->next)
    if (item->sound_id == sound_id)
      return item->buffer;

  item = Z_Calloc(1, sizeof(*item));
  item->sound_id = sound_id;

  alGetError();
  alGenBuffers(1, &item->buffer);
  alBufferData(item->buffer, format, data, (ALsizei)length, sample_rate);
  if (alGetError() != AL_NO_ERROR)
  {
    if (item->buffer)
      alDeleteBuffers(1, &item->buffer);
    Z_Free(item);
    return 0;
  }

  item->next = sound_buffers;
  sound_buffers = item;
  return item->buffer;
}

dboolean I_OAL_InitSound(int channel_count)
{
  int i;

  sound_sources = Z_Calloc(channel_count, sizeof(*sound_sources));
  sound_channel_count = channel_count;

  alGetError();
  alGenSources(channel_count, sound_sources);
  if (alGetError() != AL_NO_ERROR)
  {
    lprintf(LO_WARN, "couldn't create OpenAL sound effect sources\n");
    I_OAL_ShutdownSound();
    return false;
  }

  for (i = 0; i < sound_channel_count; ++i)
    I_OAL_ResetSource2D(i);

  soft_source_spatialize = alIsExtensionPresent("AL_SOFT_source_spatialize") == AL_TRUE;

  // Spatialization is required even for 2D panning emulation.
  if (soft_source_spatialize)
  {
    for (i = 0; i < sound_channel_count; ++i)
      alSourcei(sound_sources[i], AL_SOURCE_SPATIALIZE_SOFT, AL_TRUE);
  }

  I_OAL_SetResampler();

  return true;
}

void I_OAL_ShutdownSound(void)
{
  int i;
  sound_buffer_t *item;

  if (sound_sources)
  {
    for (i = 0; i < sound_channel_count; ++i)
      alSourceStop(sound_sources[i]);

    alDeleteSources(sound_channel_count, sound_sources);
    Z_Free(sound_sources);
    sound_sources = NULL;
  }

  while (sound_buffers)
  {
    item = sound_buffers;
    sound_buffers = item->next;
    alDeleteBuffers(1, &item->buffer);
    Z_Free(item);
  }

  sound_channel_count = 0;
  soft_source_spatialize = false;
}

void I_OAL_StopSound(int channel)
{
  if (!sound_sources)
  {
    return;
  }

  alSourceStop(sound_sources[channel]);
  alSourcei(sound_sources[channel], AL_BUFFER, 0);
}

dboolean I_OAL_StartSound(int channel, int sound_id, const void *data, size_t length,
                          int bits, int sample_rate, dboolean loop, float pitch)
{
  ALuint buffer;

  if (!sound_sources)
  {
    return false;
  }

  I_OAL_StopSound(channel);

  buffer = I_OAL_CacheSound(sound_id, data, length, bits, sample_rate);
  if (!buffer)
  {
    lprintf(LO_WARN, "couldn't buffer OpenAL sound effect\n");
    return false;
  }

  alGetError();
  alSourcei(sound_sources[channel], AL_BUFFER, buffer);
  alSourcei(sound_sources[channel], AL_LOOPING, loop ? AL_TRUE : AL_FALSE);
  alSourcef(sound_sources[channel], AL_PITCH, pitch);
  alSourcePlay(sound_sources[channel]);

  if (alGetError() != AL_NO_ERROR)
  {
    lprintf(LO_WARN, "couldn't play OpenAL sound effect\n");
    I_OAL_StopSound(channel);
    return false;
  }

  return true;
}

dboolean I_OAL_SoundIsPlaying(int channel)
{
  ALint state;

  if (!sound_sources)
  {
    return false;
  }

  alGetSourcei(sound_sources[channel], AL_SOURCE_STATE, &state);

  return state == AL_PLAYING;
}

void I_OAL_SetVolume(int channel, int volume)
{
  if (!sound_sources)
  {
    return;
  }

  alSourcef(sound_sources[channel], AL_GAIN, VOL_TO_GAIN(volume));
}

void I_OAL_SetPan(int channel, int separation)
{
  ALfloat pan;

  if (!sound_sources)
  {
    return;
  }

  // Emulate 2D panning (https://github.com/kcat/openal-soft/issues/194).
  // This works by sliding the sound source along the x-axis while inverting
  // the circular shape of the sound field along the z-axis. The end result
  // is perceived to move in a straight line along the x-axis only (panning).
  pan = (ALfloat)separation / 255.0f - 0.5f;
  alSource3f(sound_sources[channel], AL_POSITION, pan, 0.0f,
             -sqrtf(1.0f - pan * pan));
}
