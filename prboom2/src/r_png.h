//
// SLADE - It's a Doom Editor
// Copyright(C) 2008 - 2019 Simon Judd
// Copyright(C) 2024 Roman Fomin
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
//	NYAN PNG to Patch Support
//  - based on Woof
//

#ifndef R_PNG_H
#define R_PNG_H

#include <stddef.h>

#include "doomtype.h"
#include "spng.h"

#define NO_COLOR_KEY (-1)

typedef struct
{
    spng_ctx *ctx;
    byte *image;
    byte *translate;
    size_t image_size;
    int width;
    int height;
    int color_key;
} png_t;

dboolean R_IsPNGLump(int lumpnum);
dboolean InitPNG(png_t *png, const void *buffer, int buffer_length);
void FreePNG(png_t *png);
dboolean DecodePNG(png_t *png);
dboolean DecodePNG_RGBA(png_t *png);
void GetPNGOffsets(spng_ctx *ctx, int *leftoffset, int *topoffset);

#endif
