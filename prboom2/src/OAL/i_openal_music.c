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

#include <math.h>
#include <string.h>

#include <SDL3/SDL.h>
#include "al.h"
#include "ebur128.h"

#include "i_openal_music.h"
#include "lprintf.h"

// Define the number of buffers and buffer size (in milliseconds) to use. 4
// buffers with 4096 samples each gives a nice per-chunk size, and lets the
// queue last for about half second at 44.1khz.
#define NUM_BUFFERS 4
#define BUFFER_SAMPLES 4096

#define DB_TO_GAIN(x) powf(10.0f, (x) / 20.0f)

typedef struct
{
  // These are the buffers and source to play out through OpenAL with
  ALuint buffers[NUM_BUFFERS];
  ALuint source;

  byte *data;

  ALfloat gain;
  ALfloat auto_gain;

  // The format of the output stream
  ALenum format;
  ALsizei freq;
  ALsizei frame_size;
  int channels;
  int total_frames;
} stream_player_t;

static stream_player_t player;
static openal_music_callback_t music_callback;
static SDL_Thread *stream_thread;
static SDL_AtomicInt stream_running;
static SDL_AtomicInt stream_restart;
static SDL_AtomicInt stream_auto_gain;
static SDL_AtomicInt stream_music_volume;
static SDL_AtomicInt stream_reset_gain;
static ebur128_state *ebur_state;

static void ShutdownAutoGain(void)
{
  if (ebur_state)
  {
    ebur128_destroy(&ebur_state);
  }
}

static void InitAutoGain(void)
{
  int volume;

  ShutdownAutoGain();

  ebur_state = ebur128_init(player.channels, player.freq, EBUR128_MODE_S
    | EBUR128_MODE_I | EBUR128_MODE_SAMPLE_PEAK | EBUR128_MODE_HISTOGRAM);

  volume = SDL_GetAtomicInt(&stream_music_volume);

  player.gain = (ALfloat)volume / 15.0f;
  player.auto_gain = 1.0f;
  player.total_frames = 0;

  alSourcef(player.source, AL_GAIN, player.auto_gain);
}

static void AutoGain(int frames)
{
  const float target = -23.0f; // UFS

  dboolean failed = false;
  double momentary = 0.0;
  double shortterm = 0.0;
  double global = 0.0;
  double relative = 0.0;

  if (!SDL_GetAtomicInt(&stream_auto_gain) || !ebur_state)
    return;

  if (player.format == AL_FORMAT_MONO16 || player.format == AL_FORMAT_STEREO16)
  {
    ebur128_add_frames_short(ebur_state, (int16_t *)player.data, frames);
  }
  else
  {
    ebur128_add_frames_float(ebur_state, (float *)player.data, frames);
  }

  player.total_frames += frames;

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

  if (player.total_frames < 3 * BUFFER_SAMPLES && !failed)
  {
    if (momentary > -70.0)
    {
      const float diff = target - (float)momentary;
      player.auto_gain = DB_TO_GAIN(diff);
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

    if (player.channels == 2 &&
        EBUR128_SUCCESS != ebur128_prev_sample_peak(ebur_state, 1, &peak_R))
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
        player.auto_gain = gain;
      }
    }
  }

  alSourcef(player.source, AL_GAIN, player.gain * player.auto_gain);
}

static void UpdateMusicGain(void)
{
  if (SDL_SetAtomicInt(&stream_reset_gain, 0))
    InitAutoGain();

  AutoGain(BUFFER_SAMPLES);
}

static int MusicThread(void *unused)
{
  const int size = BUFFER_SAMPLES * player.frame_size;
  int i;

  (void)unused;

  player.data = SDL_malloc(size);
  if (!player.data)
    return 1;

  for (i = 0; i < NUM_BUFFERS; ++i)
  {
    memset(player.data, 0, size);
    music_callback(player.data, size);
    UpdateMusicGain();
    alBufferData(player.buffers[i], player.format, player.data, size, player.freq);
  }

  alSourceQueueBuffers(player.source, NUM_BUFFERS, player.buffers);
  alSourcePlay(player.source);

  while (SDL_GetAtomicInt(&stream_running))
  {
    ALint processed = 0;

    // Clear queued audio and restart playback immediately for a new song
    if (SDL_SetAtomicInt(&stream_restart, 0))
    {
      alSourceStop(player.source);
      alSourcei(player.source, AL_BUFFER, 0);

      for (i = 0; i < NUM_BUFFERS; ++i)
      {
        memset(player.data, 0, size);
        music_callback(player.data, size);
        UpdateMusicGain();
        alBufferData(player.buffers[i], player.format, player.data, size, player.freq);
      }

      alSourceQueueBuffers(player.source, NUM_BUFFERS, player.buffers);
      alSourcePlay(player.source);
    }

    alGetSourcei(player.source, AL_BUFFERS_PROCESSED, &processed);
    while (processed-- > 0)
    {
      ALuint buffer_id;

      alSourceUnqueueBuffers(player.source, 1, &buffer_id);
      memset(player.data, 0, size);
      music_callback(player.data, size);
      UpdateMusicGain();
      alBufferData(buffer_id, player.format, player.data, size, player.freq);
      alSourceQueueBuffers(player.source, 1, &buffer_id);
    }

    {
      ALint state;
      alGetSourcei(player.source, AL_SOURCE_STATE, &state);
      if (state != AL_PLAYING)
        alSourcePlay(player.source);
    }

    SDL_Delay(1);
  }

  alSourceStop(player.source);
  SDL_free(player.data);
  player.data = NULL;
  return 0;
}

void I_RestartOpenALMusic(void)
{
  SDL_SetAtomicInt(&stream_restart, 1);
}

dboolean I_StartOpenALMusic(int sample_rate, openal_music_callback_t callback)
{
  player.format = AL_FORMAT_STEREO16;
  player.freq = sample_rate;
  player.channels = 2;
  player.frame_size = player.channels * sizeof(short);
  music_callback = callback;
  SDL_SetAtomicInt(&stream_reset_gain, 1);

  alGenSources(1, &player.source);
  alGenBuffers(NUM_BUFFERS, player.buffers);
  if (alGetError() != AL_NO_ERROR)
  {
    lprintf(LO_WARN, "couldn't create OpenAL streaming objects\n");
    I_StopOpenALMusic();
    return false;
  }

  SDL_SetAtomicInt(&stream_running, 1);
  stream_thread = SDL_CreateThread(MusicThread, "OpenAL music", NULL);
  if (!stream_thread)
  {
    lprintf(LO_WARN, "couldn't create OpenAL music thread (%s)\n", SDL_GetError());
    I_StopOpenALMusic();
    return false;
  }

  return true;
}

void I_ConfigureOpenALMusicGain(dboolean enabled, int volume)
{
  SDL_SetAtomicInt(&stream_auto_gain, enabled);
  SDL_SetAtomicInt(&stream_music_volume, volume);
  SDL_SetAtomicInt(&stream_reset_gain, 1);
}

void I_ResetOpenALMusicGain(void)
{
  SDL_SetAtomicInt(&stream_reset_gain, 1);
}

void I_StopOpenALMusic(void)
{
  if (stream_thread)
  {
    SDL_SetAtomicInt(&stream_running, 0);
    SDL_WaitThread(stream_thread, NULL);
    stream_thread = NULL;
  }

  ShutdownAutoGain();

  if (player.source)
  {
    alSourcei(player.source, AL_BUFFER, 0);
    alDeleteSources(1, &player.source);
    player.source = 0;
  }

  if (player.buffers[0])
  {
    alDeleteBuffers(NUM_BUFFERS, player.buffers);
    memset(player.buffers, 0, sizeof(player.buffers));
  }
}
