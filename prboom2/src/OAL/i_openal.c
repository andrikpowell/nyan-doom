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
#include "alext.h"

#include "dsda/configuration.h"
#include "i_openal.h"
#include "lprintf.h"

static ALCdevice *oal_device;
static ALCcontext *oal_context;
static int oal_sample_rate;

static void I_OpenALAttributes(ALCint *attributes, int sample_rate)
{
  int i = 0;

  attributes[i++] = ALC_FREQUENCY;
  attributes[i++] = sample_rate;

#ifdef ALC_OUTPUT_MODE_SOFT
  if (oal_device &&
      alcIsExtensionPresent(oal_device, "ALC_SOFT_output_mode") == ALC_TRUE)
  {
    attributes[i++] = ALC_OUTPUT_MODE_SOFT;
    attributes[i++] = ALC_STEREO_BASIC_SOFT;
  }
#endif

  if (oal_device &&
      alcIsExtensionPresent(oal_device, "ALC_SOFT_output_limiter") == ALC_TRUE)
  {
    attributes[i++] = ALC_OUTPUT_LIMITER_SOFT;
    attributes[i++] = dsda_IntConfig(dsda_config_snd_limiter) ? ALC_TRUE : ALC_FALSE;
  }

  attributes[i] = 0;
}

dboolean I_OpenALDevice(const char *device_name, int sample_rate)
{
  ALCint attributes[7];

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

  I_OpenALAttributes(attributes, sample_rate);

  oal_context = alcCreateContext(oal_device, attributes);
  if (!oal_context || !alcMakeContextCurrent(oal_context))
  {
    lprintf(LO_WARN, "couldn't create an OpenAL context\n");
    I_CloseOpenALDevice();
    return false;
  }

  oal_sample_rate = sample_rate;

  return true;
}

dboolean I_ResetOpenALDevice(void)
{
  LPALCRESETDEVICESOFT alcResetDeviceSOFT;
  ALCint attributes[7];

  if (!oal_device ||
      alcIsExtensionPresent(oal_device, "ALC_SOFT_HRTF") != ALC_TRUE)
    return false;

  alcResetDeviceSOFT =
    (LPALCRESETDEVICESOFT)alcGetProcAddress(oal_device, "alcResetDeviceSOFT");
  if (!alcResetDeviceSOFT)
    return false;

  I_OpenALAttributes(attributes, oal_sample_rate);

  if (alcResetDeviceSOFT(oal_device, attributes) != ALC_TRUE)
  {
    lprintf(LO_WARN, "couldn't reset the OpenAL device\n");
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

  oal_sample_rate = 0;
}
