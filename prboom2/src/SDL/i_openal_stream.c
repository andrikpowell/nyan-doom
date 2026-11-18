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
//	OpenAL output stream
//

#include <string.h>

#include "SDL.h"
#include "al.h"
#include "ebur128.h"

#include "i_openal_stream.h"
#include "lprintf.h"

// Define the number of buffers and buffer size (in milliseconds) to use. 4
// buffers with 4096 samples each gives a nice per-chunk size, and lets the
// queue last for about half second at 44.1khz.
#define NUM_BUFFERS 4
#define BUFFER_SAMPLES 4096

#define DB_TO_GAIN(x) powf(10.0f, (x) / 20.0f)

static ALuint stream_source;
static ALuint stream_buffers[NUM_BUFFERS];
static SDL_Thread *stream_thread;
static SDL_atomic_t stream_running;
static int stream_buffer_bytes;
static int stream_sample_rate;
static openal_stream_callback_t stream_callback;
static void *stream_userdata;
static SDL_atomic_t stream_auto_gain;
static SDL_atomic_t stream_music_volume;
static SDL_atomic_t stream_reset_gain;
static ebur128_state *ebur_state;
static float auto_gain = 1.0f;
static int total_frames;

static void ShutdownAutoGain(void)
{
  if (ebur_state)
  {
    ebur128_destroy(&ebur_state);
  }
}

static void InitAutoGain(void)
{
  ShutdownAutoGain();

  ebur_state = ebur128_init(2, stream_sample_rate, EBUR128_MODE_S
    | EBUR128_MODE_I | EBUR128_MODE_SAMPLE_PEAK | EBUR128_MODE_HISTOGRAM);

  auto_gain = 1.0f;
  total_frames = 0;

  alSourcef(stream_source, AL_GAIN, 1.0f);
}

static void AutoGain(const int16_t *buffer, int frames)
{
  const float target = -23.0f; // UFS

  dboolean failed = false;
  double momentary = 0.0;
  double shortterm = 0.0;
  double global = 0.0;
  double relative = 0.0;

  if (!SDL_AtomicGet(&stream_auto_gain) || !ebur_state)
    return;

  ebur128_add_frames_short(ebur_state, buffer, frames);

  total_frames += frames;

  if (EBUR128_SUCCESS != ebur128_loudness_momentary(ebur_state, &momentary))
  {
      failed = true;
  }

  if (EBUR128_SUCCESS != ebur128_loudness_shortterm(ebur_state, &shortterm))
  {
      failed = true;
  }

  if (EBUR128_SUCCESS != ebur128_loudness_global(ebur_state, &global))
  {
      failed = true;
  }

  if (EBUR128_SUCCESS != ebur128_relative_threshold(ebur_state, &relative))
  {
      failed = true;
  }

  if (total_frames < 3 * BUFFER_SAMPLES && !failed)
  {
    if (momentary > -70.0)
    {
      const float diff = target - (float)momentary;
      auto_gain = DB_TO_GAIN(diff);
    }
  }

  if (relative > -70.0 && momentary > relative && !failed)
  {
    double peak_L = 0.0;
    double peak_R = 0.0;

    if (EBUR128_SUCCESS != ebur128_prev_sample_peak(ebur_state, 0, &peak_L))
    {
        failed = true;
    }

    if (EBUR128_SUCCESS != ebur128_prev_sample_peak(ebur_state, 1, &peak_R))
    {
        failed = true;
    }

    if (!failed)
    {
      const float weight_m = 0.1f;
      const float weight_s = 1.0f;
      const float weight_i = 1.0f;
      float loudness = (weight_m * (float)momentary + weight_s * (float)shortterm
                        + weight_i * (float)global)
                        / (weight_m + weight_s + weight_i);

      float diff = target - loudness;

      float gain = DB_TO_GAIN(diff);

      double peak = (peak_L > peak_R) ? peak_L : peak_R;

      if (peak >= 0.00001 && gain * peak < 1.0f)
      {
        auto_gain = gain;
      }
    }
  }

  alSourcef(stream_source, AL_GAIN, ((float)SDL_AtomicGet(&stream_music_volume) / 15.0f) * auto_gain);
}

static void UpdateStreamGain(unsigned char *buffer)
{
  if (SDL_AtomicSet(&stream_reset_gain, 0))
    InitAutoGain();

  // 2 channels x 2 bytes = 4 bytes
  AutoGain((const int16_t *)buffer, stream_buffer_bytes / 4);
}

static int StreamThread(void *unused)
{
  unsigned char *buffer;
  int i;

  (void)unused;

  buffer = SDL_malloc(stream_buffer_bytes);
  if (!buffer)
    return 1;

  for (i = 0; i < NUM_BUFFERS; ++i)
  {
    memset(buffer, 0, stream_buffer_bytes);
    stream_callback(stream_userdata, buffer, stream_buffer_bytes);
    UpdateStreamGain(buffer);
    alBufferData(stream_buffers[i], AL_FORMAT_STEREO16, buffer, stream_buffer_bytes, stream_sample_rate);
  }

  alSourceQueueBuffers(stream_source, NUM_BUFFERS, stream_buffers);
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
      UpdateStreamGain(buffer);
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
  int buffer_frames = (sample_frames + NUM_BUFFERS - 1) / NUM_BUFFERS;

  stream_sample_rate = sample_rate;
  // Split buffer size across all OpenAL buffers
  // This fixes sound effects being delayed
  stream_buffer_bytes = buffer_frames * 2 * sizeof(short);
  stream_callback = callback;
  stream_userdata = userdata;
  SDL_AtomicSet(&stream_reset_gain, 1);

  alGenSources(1, &stream_source);
  alGenBuffers(NUM_BUFFERS, stream_buffers);
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

void I_ConfigureOpenALStreamGain(dboolean enabled, int volume)
{
  SDL_AtomicSet(&stream_auto_gain, enabled);
  SDL_AtomicSet(&stream_music_volume, volume);
  SDL_AtomicSet(&stream_reset_gain, 1);
}

void I_ResetOpenALStreamGain(void)
{
  SDL_AtomicSet(&stream_reset_gain, 1);
}

void I_StopOpenALStream(void)
{
  if (stream_thread)
  {
    SDL_AtomicSet(&stream_running, 0);
    SDL_WaitThread(stream_thread, NULL);
    stream_thread = NULL;
  }

  ShutdownAutoGain();

  if (stream_source)
  {
    alSourcei(stream_source, AL_BUFFER, 0);
    alDeleteSources(1, &stream_source);
    stream_source = 0;
  }

  if (stream_buffers[0])
  {
    alDeleteBuffers(NUM_BUFFERS, stream_buffers);
    memset(stream_buffers, 0, sizeof(stream_buffers));
  }
}
