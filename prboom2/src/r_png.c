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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "r_png.h"

#include <string.h>

#include "doomdef.h"
#include "lprintf.h"
#include "m_swap.h"
#include "v_video.h"
#include "w_wad.h"
#include "z_zone.h"

dboolean R_IsPNGLump(int lumpnum)
{
  return W_LumpLength(lumpnum) >= 8 &&
         !memcmp(W_LumpByNum(lumpnum), "\211PNG\r\n\032\n", 8);
}

void GetPNGOffsets(spng_ctx *ctx, int *leftoffset, int *topoffset)
{
  uint32_t chunk_count = 0;
  struct spng_unknown_chunk *chunks;
  int result;
  int i;

  *leftoffset = 0;
  *topoffset = 0;

  result = spng_get_unknown_chunks(ctx, NULL, &chunk_count);
  if ((result && result != SPNG_ECHUNKAVAIL) || !chunk_count)
    return;

  chunks = Z_Malloc(chunk_count * sizeof(*chunks));

  if (!spng_get_unknown_chunks(ctx, chunks, &chunk_count))
    for (i = 0; i < (int)chunk_count; ++i)
    {
      if (!memcmp(chunks[i].type, "grAb", 4) && chunks[i].length == 8)
      {
        int *offsets = chunks[i].data;
        *leftoffset = SWAP_BE32(offsets[0]);
        *topoffset = SWAP_BE32(offsets[1]);
        break;
      }
    }

  Z_Free(chunks);
}

// Uniform Color Quantization
//
// Each color component axis (red, green and blue) is divided into a few fixed
// segments (8-8-4 in 256 colors). Each found color is placed into a
// corresponding segment slot. After all the colors are added, an average
// color is calculated for each slot. Those are the colors of the palette.
typedef struct
{
    int value;
    int pixel_count;
} color_slot_t;

static void AddValue(color_slot_t *s, int component)
{
    s->value += component;
    s->pixel_count++;
}

static int GetAverage(color_slot_t *s)
{
    int result = 0;

    if (s->pixel_count > 0)
    {
        result = s->value / s->pixel_count;
    }

    return result;
}

typedef struct
{
    color_slot_t red_slots[8];
    color_slot_t green_slots[8];
    color_slot_t blue_slots[8];

    byte palette[3*512];
} uniform_quantizer_t;

static void AddColor(uniform_quantizer_t *q, int r, int g, int b)
{
    int red_index = r >> 5;
    int green_index = g >> 5;
    int blue_index = b >> 5;
    AddValue(&q->red_slots[red_index], r);
    AddValue(&q->green_slots[green_index], g);
    AddValue(&q->blue_slots[blue_index], b);
}

static void GetPalette(uniform_quantizer_t *q)
{
    byte *roller = q->palette;

    for (int rs = 0; rs < arrlen(q->red_slots); ++rs)
    {
        for (int gs = 0; gs < arrlen(q->green_slots); ++gs)
        {
            for (int bs = 0; bs < arrlen(q->blue_slots); ++bs)
            {
                *roller++ = GetAverage(&q->red_slots[rs]);
                *roller++ = GetAverage(&q->green_slots[gs]);
                *roller++ = GetAverage(&q->blue_slots[bs]);
            }
        }
    }
}

static int GetPaletteIndex(int r, int g, int b)
{
    int red_index = r >> 5;
    int green_index = g >> 5;
    int blue_index = b >> 5;
    return (red_index << 6) + (green_index << 3) + blue_index;
}

// Set memory usage limits for storing standard and unknown chunks,
// this is important when reading untrusted files!
#define PNG_MEM_LIMIT (1024 * 1024 * 64)

dboolean InitPNG(png_t *png, const void *buffer, int buffer_length)
{
    spng_ctx *ctx = spng_ctx_new(0);

    // Ignore and don't calculate chunk CRC's
    int ret = spng_set_crc_action(ctx, SPNG_CRC_USE, SPNG_CRC_USE);

    if (ret)
    {
        lprintf(LO_ERROR, "InitPNG: spng_set_crc_action %s\n",
                spng_strerror(ret));
        return false;
    }

    ret = spng_set_chunk_limits(ctx, PNG_MEM_LIMIT, PNG_MEM_LIMIT);

    if (ret)
    {
        lprintf(LO_ERROR, "InitPNG: spng_set_chunk_limits %s\n",
                spng_strerror(ret));
        return false;
    }

    ret = spng_set_png_buffer(ctx, buffer, buffer_length);

    if (ret)
    {
        lprintf(LO_ERROR, "InitPNG: spng_set_png_buffer %s\n",
                spng_strerror(ret));
        return false;
    }

    png->ctx = ctx;

    return true;
}

void FreePNG(png_t *png)
{
    spng_ctx_free(png->ctx);
    if (png->image)
    {
        Z_Free(png->image);
    }
    if (png->translate)
    {
        Z_Free(png->translate);
    }
}

dboolean DecodePNG(png_t *png)
{
    struct spng_ihdr ihdr = {0};
    int ret;
    int fmt;
    size_t image_size;
    byte *image;
    const byte *playpal;

    ret = spng_get_ihdr(png->ctx, &ihdr);

    if (ret)
    {
        lprintf(LO_ERROR, "DecodePNG: spng_get_ihdr %s\n",
                spng_strerror(ret));
        return false;
    }

    png->width = ihdr.width;
    png->height = ihdr.height;

    switch (ihdr.color_type)
    {
        case SPNG_COLOR_TYPE_INDEXED:
            fmt = SPNG_FMT_PNG;
            break;
        case SPNG_COLOR_TYPE_GRAYSCALE:
        case SPNG_COLOR_TYPE_TRUECOLOR:
            fmt = SPNG_FMT_RGB8;
            break;
        default:
            fmt = SPNG_FMT_RGBA8;
            break;
    }

    image_size = 0;
    ret = spng_decoded_image_size(png->ctx, fmt, &image_size);

    if (ret)
    {
        lprintf(LO_ERROR, "DecodePNG: spng_decoded_image_size %s",
                spng_strerror(ret));
        return false;
    }

    image = Z_Malloc(image_size);
    ret = spng_decode_image(png->ctx, image, image_size, fmt, 0);

    if (ret)
    {
        lprintf(LO_ERROR, "DecodePNG: spng_decode_image %s",
                spng_strerror(ret));
        Z_Free(image);
        return false;
    }

    playpal = V_GetPlaypal();

    if (fmt == SPNG_FMT_RGB8)
    {
        size_t indexed_size = image_size / 3;
        byte *indexed_image = Z_Malloc(indexed_size);

        uniform_quantizer_t q = {0};

        byte *roller = image;
        byte translate[512];
        byte *palette;

        for (size_t i = 0; i < indexed_size; ++i)
        {
            int r = *roller++;
            int g = *roller++;
            int b = *roller++;

            AddColor(&q, r, g, b);
        }

        GetPalette(&q);

        palette = q.palette;
        for (int i = 0; i < 512; ++i)
        {
            int r = *palette++;
            int g = *palette++;
            int b = *palette++;

            translate[i] = V_BestColor(playpal, r, g, b);
        }

        roller = image;

        for (size_t i = 0; i < indexed_size; ++i)
        {
            int r = *roller++;
            int g = *roller++;
            int b = *roller++;

            indexed_image[i] = translate[GetPaletteIndex(r, g, b)];
        }

        Z_Free(image);

        png->image = indexed_image;
        png->image_size = indexed_size;
    }
    else if (fmt == SPNG_FMT_RGBA8)
    {
        size_t indexed_size = image_size / 4;
        byte *indexed_image = Z_Malloc(indexed_size);

        uniform_quantizer_t q = {0};

        byte *roller = image;

        byte used_colors[256] = {0};
        byte translate[512];
        byte *palette;
        int color_key;
        dboolean has_alpha = false;

        for (size_t i = 0; i < indexed_size; ++i)
        {
            int r = *roller++;
            int g = *roller++;
            int b = *roller++;
            int a = *roller++;
            if (a < 255)
            {
                has_alpha = true;
                continue;
            }

            AddColor(&q, r, g, b);
        }

        GetPalette(&q);

        palette = q.palette;
        for (int i = 0; i < 512; ++i)
        {
            int r = *palette++;
            int g = *palette++;
            int b = *palette++;
            byte c = V_BestColor(playpal, r, g, b);

            used_colors[c] = 1;
            translate[i] = c;
        }

        color_key = NO_COLOR_KEY;

        if (has_alpha)
        {
            for (int i = 0; i < 256; ++i)
            {
                if (used_colors[i] == 0)
                {
                    color_key = i;
                    break;
                }
            }
            png->color_key = color_key;
        }

        roller = image;

        for (size_t i = 0; i < indexed_size; ++i)
        {
            int r = *roller++;
            int g = *roller++;
            int b = *roller++;
            int a = *roller++;
            if (a < 255)
            {
                indexed_image[i] = color_key;
                continue;
            }

            indexed_image[i] = translate[GetPaletteIndex(r, g, b)];
        }

        Z_Free(image);

        png->image = indexed_image;
        png->image_size = indexed_size;
    }
    else
    {
        struct spng_plte plte = {0};
        byte *translate;
        dboolean need_translation;
        const byte *palette;

        ret = spng_get_plte(png->ctx, &plte);

        if (ret)
        {
            lprintf(LO_ERROR, "DecodePNG: spng_get_plte %s\n",
                    spng_strerror(ret));
            return false;
        }

        translate = Z_Malloc(plte.n_entries);
        need_translation = false;
        palette = playpal;

        for (size_t i = 0; i < plte.n_entries; ++i)
        {
            struct spng_plte_entry *e = &plte.entries[i];

            byte r = *palette++;
            byte g = *palette++;
            byte b = *palette++;

            if (e->red == r && e->green == g && e->blue == b)
            {
                translate[i] = (byte)i;
                continue;
            }

            need_translation = true;
            translate[i] =
                V_BestColor(playpal, e->red, e->green, e->blue);
        }

        if (need_translation)
        {
            png->translate = translate;
        }
        else
        {
            Z_Free(translate);
        }

        png->image = image;
        png->image_size = image_size;
    }

    return true;
}

dboolean DecodePNG_RGBA(png_t *png)
{
    struct spng_ihdr ihdr = {0};
    size_t image_size;
    byte *image;
    int ret;

    ret = spng_get_ihdr(png->ctx, &ihdr);
    if (ret)
    {
        lprintf(LO_ERROR, "DecodePNG_RGBA: spng_get_ihdr %s\n",
                spng_strerror(ret));
        return false;
    }

    ret = spng_decoded_image_size(png->ctx, SPNG_FMT_RGBA8, &image_size);
    if (ret)
    {
        lprintf(LO_ERROR, "DecodePNG_RGBA: spng_decoded_image_size %s\n",
                spng_strerror(ret));
        return false;
    }

    image = Z_Malloc(image_size);
    ret = spng_decode_image(png->ctx, image, image_size, SPNG_FMT_RGBA8,
                            SPNG_DECODE_TRNS);
    if (ret)
    {
        lprintf(LO_ERROR, "DecodePNG_RGBA: spng_decode_image %s\n",
                spng_strerror(ret));
        Z_Free(image);
        return false;
    }

    png->width = (int)ihdr.width;
    png->height = (int)ihdr.height;
    png->image = image;
    png->image_size = image_size;

    return true;
}
