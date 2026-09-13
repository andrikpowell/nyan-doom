/* Emacs style mode select   -*- C -*-
 *-----------------------------------------------------------------------------
 *
 *
 *  PrBoom: a Doom port merged with LxDoom and LSDLDoom
 *  based on BOOM, a modified and improved DOOM engine
 *  Copyright (C) 1999 by
 *  id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
 *  Copyright (C) 1999-2006 by
 *  Jess Haas, Nicolas Kalkhof, Colin Phipps, Florian Schulze
 *  Copyright 2005, 2006 by
 *  Florian Schulze, Colin Phipps, Neil Stevens, Andrey Budko
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 *  02111-1307, USA.
 *
 * DESCRIPTION:
 *  Screenshot functions, moved out of i_video.c
 *
 *-----------------------------------------------------------------------------
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <string.h>

#include "SDL.h"
#include "spng.h"

#include "doomstat.h"
#include "doomdef.h"
#include "doomtype.h"
#include "v_video.h"
#include "i_video.h"
#include "z_zone.h"
#include "lprintf.h"
#include "m_file.h"

#include "dsda/gl/render_scale.h"

int renderW;
int renderH;

void I_UpdateRenderSize(void)
{
  renderW = renderer_rect.w;
  renderH = renderer_rect.h;
}

//
// I_ScreenShot // Modified to work with SDL2 resizeable window and fullscreen desktop - DTIED
//

// [FG] save screenshots in PNG format
static int I_WritePNGFile(const char *filename, const byte *pixels)
{
  FILE *file;
  spng_ctx *ctx;
  struct spng_ihdr ihdr = {0};
  size_t image_size = (size_t)renderW * renderH * 3;
  int result;

  file = M_OpenFile(filename, "wb");
  if (!file)
    return -1;

  ctx = spng_ctx_new(SPNG_CTX_ENCODER);
  spng_set_png_file(ctx, file);
  spng_set_option(ctx, SPNG_IMG_COMPRESSION_LEVEL, 1);

  ihdr.width = renderW;
  ihdr.height = renderH;
  ihdr.color_type = SPNG_COLOR_TYPE_TRUECOLOR;
  ihdr.bit_depth = 8;
  spng_set_ihdr(ctx, &ihdr);

  result = spng_encode_image(ctx, pixels, image_size, SPNG_FMT_PNG,
                             SPNG_ENCODE_FINALIZE);

  if (result)
  {
      lprintf(LO_ERROR, "I_WritePNGfile: spng_encode_image failed: %s\n",
              spng_strerror(result));
  }
  else
  {
      lprintf(LO_DEBUG, "I_WritePNGfile: %s", filename);
  }

  fclose(file);
  spng_ctx_free(ctx);

  return result;
}

int I_ScreenShot(const char *fname)
{
  const byte *pixels = I_GrabScreen();

  if (!pixels)
    return -1;

  return I_WritePNGFile(fname, pixels);
}

// NSM
// returns current screen contents as RGB24 (raw)
// returned pointer should be freed when done
//
// Modified to work with SDL2 resizeable window and fullscreen desktop - DTIED
//

unsigned char *I_GrabScreen(void)
{
  static unsigned char *pixels = NULL;
  static int pixels_size = 0;
  int size;

  I_UpdateRenderSize();

  if (V_IsOpenGLMode())
  {
    return gld_ReadScreen();
  }

  size = renderW * renderH * 3;
  if (!pixels || size > pixels_size)
  {
    pixels_size = size;
    pixels = (unsigned char*)Z_Realloc(pixels, size);
  }

  if (pixels && size)
  {
    int dest_x, dest_y;
    SDL_Rect screen = viewport_rect;

    // software can include borderbox areas outside the game viewport
    // keep those borderbox pixels black, but read the viewport pixels
    memset(pixels, 0, size);

    // to avoid screenshot drawing from the top left, center it (pad with borderboxes)
    dest_x = screen.x * 3;
    dest_y = screen.y * renderW * 3;

    SDL_RenderReadPixels(sdl_renderer, &screen, SDL_PIXELFORMAT_RGB24, pixels + dest_x + dest_y, renderW * 3);
  }

  return pixels;
}
