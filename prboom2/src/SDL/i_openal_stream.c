//
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
//	OpenAL output stream
//

#include <string.h>

#include "SDL.h"
#include "al.h"

#include "i_openal_stream.h"
#include "lprintf.h"

#define OPENAL_STREAM_BUFFER_COUNT 4

static ALuint stream_source;
static ALuint stream_buffers[OPENAL_STREAM_BUFFER_COUNT];
static SDL_Thread *stream_thread;
static SDL_atomic_t stream_running;
static int stream_buffer_bytes;
static int stream_sample_rate;
static openal_stream_callback_t stream_callback;
static void *stream_userdata;

static int StreamThread(void *unused)
{
  unsigned char *buffer;
  int i;

  (void)unused;

  buffer = SDL_malloc(stream_buffer_bytes);
  if (!buffer)
    return 1;

  for (i = 0; i < OPENAL_STREAM_BUFFER_COUNT; ++i)
  {
    memset(buffer, 0, stream_buffer_bytes);
    stream_callback(stream_userdata, buffer, stream_buffer_bytes);
    alBufferData(stream_buffers[i], AL_FORMAT_STEREO16, buffer, stream_buffer_bytes, stream_sample_rate);
  }

  alSourceQueueBuffers(stream_source, OPENAL_STREAM_BUFFER_COUNT, stream_buffers);
  alSourcePlay(stream_source);

  while (SDL_AtomicGet(&stream_running))
  {
    ALint processed = 0;

    alGetSourcei(stream_source, AL_BUFFERS_PROCESSED, &processed);
    while (processed-- > 0)
    {
      ALuint buffer_id;

      alSourceUnqueueBuffers(stream_source, 1, &buffer_id);
      memset(buffer, 0, stream_buffer_bytes);
      stream_callback(stream_userdata, buffer, stream_buffer_bytes);
      alBufferData(buffer_id, AL_FORMAT_STEREO16, buffer, stream_buffer_bytes, stream_sample_rate);
      alSourceQueueBuffers(stream_source, 1, &buffer_id);
    }

    {
      ALint state;
      alGetSourcei(stream_source, AL_SOURCE_STATE, &state);
      if (state != AL_PLAYING)
        alSourcePlay(stream_source);
    }

    SDL_Delay(1);
  }

  alSourceStop(stream_source);
  SDL_free(buffer);
  return 0;
}

dboolean I_StartOpenALStream(int sample_rate, int sample_frames, openal_stream_callback_t callback, void *userdata)
{
  int buffer_frames = (sample_frames + OPENAL_STREAM_BUFFER_COUNT - 1) / OPENAL_STREAM_BUFFER_COUNT;

  stream_sample_rate = sample_rate;
  // Split buffer size across all OpenAL buffers
  // This fixes sound effects being delayed
  stream_buffer_bytes = buffer_frames * 2 * sizeof(short);
  stream_callback = callback;
  stream_userdata = userdata;

  alGenSources(1, &stream_source);
  alGenBuffers(OPENAL_STREAM_BUFFER_COUNT, stream_buffers);
  if (alGetError() != AL_NO_ERROR)
  {
    lprintf(LO_WARN, "couldn't create OpenAL streaming objects\n");
    I_StopOpenALStream();
    return false;
  }

  SDL_AtomicSet(&stream_running, 1);
  stream_thread = SDL_CreateThread(StreamThread, "OpenAL output", NULL);
  if (!stream_thread)
  {
    lprintf(LO_WARN, "couldn't create OpenAL output thread (%s)\n", SDL_GetError());
    I_StopOpenALStream();
    return false;
  }

  return true;
}

void I_StopOpenALStream(void)
{
  if (stream_thread)
  {
    SDL_AtomicSet(&stream_running, 0);
    SDL_WaitThread(stream_thread, NULL);
    stream_thread = NULL;
  }

  if (stream_source)
  {
    alSourcei(stream_source, AL_BUFFER, 0);
    alDeleteSources(1, &stream_source);
    stream_source = 0;
  }

  if (stream_buffers[0])
  {
    alDeleteBuffers(OPENAL_STREAM_BUFFER_COUNT, stream_buffers);
    memset(stream_buffers, 0, sizeof(stream_buffers));
  }
}
