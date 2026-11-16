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
//	OpenAL output device
//

#include "al.h"
#include "alc.h"

#include "i_openal.h"
#include "lprintf.h"

static ALCdevice *oal_device;
static ALCcontext *oal_context;

dboolean I_OpenALDevice(const char *device_name, int sample_rate)
{
  ALCint attributes[] = { ALC_FREQUENCY, sample_rate, 0 };

  oal_device = alcOpenDevice(device_name);
  if (!oal_device && device_name)
  {
    lprintf(LO_WARN, "couldn't open OpenAL device \"%s\"; falling back to default\n", device_name);
    oal_device = alcOpenDevice(NULL);
  }

  if (!oal_device)
  {
    lprintf(LO_WARN, "couldn't open the default OpenAL device\n");
    return false;
  }

  oal_context = alcCreateContext(oal_device, attributes);
  if (!oal_context || !alcMakeContextCurrent(oal_context))
  {
    lprintf(LO_WARN, "couldn't create an OpenAL context\n");
    I_CloseOpenALDevice();
    return false;
  }

  return true;
}

void I_CloseOpenALDevice(void)
{
  if (oal_context)
  {
    alcMakeContextCurrent(NULL);
    alcDestroyContext(oal_context);
    oal_context = NULL;
  }

  if (oal_device)
  {
    alcCloseDevice(oal_device);
    oal_device = NULL;
  }
}
