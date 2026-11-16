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

#ifndef I_OPENAL_STREAM_H
#define I_OPENAL_STREAM_H

#include "doomtype.h"

typedef void (*openal_stream_callback_t)(void *userdata, unsigned char *buffer, int length);

dboolean I_StartOpenALStream(int sample_rate, int sample_frames, openal_stream_callback_t callback, void *userdata);
void I_StopOpenALStream(void);

#endif
