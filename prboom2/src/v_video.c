/* Emacs style mode select   -*- C -*-
 *-----------------------------------------------------------------------------
 *
 *
 *  PrBoom: a Doom port merged with LxDoom and LSDLDoom
 *  based on BOOM, a modified and improved DOOM engine
 *  Copyright (C) 1999 by
 *  id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
 *  Copyright (C) 1999-2000 by
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
 *  Gamma correction LUT stuff.
 *  Color range translation support
 *  Functions to draw patches (by post) directly to screen.
 *  Functions to blit a block to the screen.
 *
 *-----------------------------------------------------------------------------
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <assert.h>
#include "SDL.h"

#include "am_map.h"
#include "doomdef.h"
#include "doomstat.h"
#include "r_main.h"
#include "r_draw.h"
#include "r_patch.h"
#include "m_bbox.h"
#include "w_wad.h"   /* needed for color translation lump lookup */
#include "v_video.h"
#include "i_video.h"
#include "lprintf.h"
#include "st_stuff.h"
#include "e6y.h"

#include "dsda/animinfo.h"
#include "dsda/configuration.h"
#include "dsda/cr_table.h"
#include "dsda/global.h"
#include "dsda/palette.h"
#include "dsda/stretch.h"
#include "dsda/tranmap.h"
#include "dsda/text_color.h"
#include "dsda/mapinfo/hexen.h"

// DWF 2012-05-10
// SetRatio sets the following global variables based on window geometry and
// user preferences. The integer ratio is hardly used anymore, so further
// simplification may be in order.
dboolean tallscreen;
unsigned int ratio_multiplier, ratio_scale;
float gl_ratio;
int psprite_offset; // Needed for "tallscreen" modes


// Each screen is [SCREENWIDTH*SCREENHEIGHT];
screeninfo_t screens[NUM_SCREENS];

/* jff 4/24/98 initialize this at runtime */
const byte *colrngs[CR_LIMIT];

int usegamma;
int extra_brightness;

int V_BloodColor(int blood)
{
  if (blood < 1 || blood > 8)
    return 0;

  return CR_BLOOD + blood - 1;
}

// haleyjd: DOSDoom-style single translucency lookup-up table
// generation code. This code has a 32k (plus a bit more)
// footprint but allows a much wider range of translucency effects
// than BOOM-style translucency. This will be used for particles,
// for variable mapthing trans levels, and for screen patches.

// haleyjd: Updated 06/21/08 to use 32k lookup, mainly to fix
// additive translucency. Note this code is included in Odamex and
// so it can be considered GPL as used here, rather than BSD. But,
// I don't care either way. It is effectively dual-licensed I suppose.

unsigned int Col2RGB8[65][256];
byte RGB32k[32][32][32];

#define MAKECOLOR(a) (((a)<<3)|((a)>>2))

void V_InitFlexTranTable(void)
{
  static int flexTranInit = false;

  if (!flexTranInit)
  {
    int r, g, b, x, y, pos;
    const unsigned char *palette = V_GetPlaypal();

    // mark that we've initialized the flex tran table
    flexTranInit = true;

    // build RGB table
    for(r = 0; r < 32; r++)
    {
      for(g = 0; g < 32; g++)
      {
        for(b = 0; b < 32; b++)
        {
          RGB32k[r][g][b] = V_BestColor(palette,
            MAKECOLOR(r), MAKECOLOR(g), MAKECOLOR(b));
        }
      }
    }

    // build lookup table
    for(x = 0; x < 65; x++)
    {
      pos = 0;
      for(y = 0; y < 256; y++)
      {
        Col2RGB8[x][y] =
          (((palette[pos + 0] * x) >> 4) << 20) |
          ((palette[pos + 1] * x) >> 4) |
          (((palette[pos + 2] * x) >> 4) << 10);

        pos += 3;
      }
    }
  }
}

void V_InitColorTranslation(void)
{
  int i;
  byte* full_table;

  full_table = dsda_GenerateCRTable();

  for (i = 0; i < CR_LIMIT; ++i)
    colrngs[i] = full_table + 256 * i;

  dsda_LoadTextColor();
}

//
// V_CopyRect
//
// Copies a source rectangle in a screen buffer to a destination
// rectangle in another screen buffer. Source origin in srcx,srcy,
// destination origin in destx,desty, common size in width and height.
// Source buffer specfified by srcscrn, destination buffer by destscrn.
//
// Marks the destination rectangle on the screen dirty.
//
// No return.
//
static void FUNC_V_CopyRect(int srcscrn, int destscrn,
                int x, int y, int width, int height,
                enum patch_translation_e flags)
{
  byte *src;
  byte *dest;

  if (flags & VPT_STRETCH_MASK)
  {
    stretch_param_t *params;
    int sx = x;
    int sy = y;

    params = dsda_StretchParams(flags);

    x  = params->video->x1lookup[x];
    y  = params->video->y1lookup[y];
    width  = params->video->x2lookup[sx + width - 1] - x + 1;
    height = params->video->y2lookup[sy + height - 1] - y + 1;
    x += params->deltax1;
    y += params->deltay1;
  }

  if (x < 0)
  {
    width += x;
    x = 0;
  }

  if (x + width > SCREENWIDTH)
  {
    width = SCREENWIDTH - x;
  }

  if (y < 0)
  {
    height += y;
    y = 0;
  }

  if (y + height > SCREENHEIGHT)
  {
    height = SCREENHEIGHT - y;
  }

  if (width <= 0 || height <= 0)
  {
    return;
  }

  src = screens[srcscrn].data + screens[srcscrn].pitch * y + x;
  dest = screens[destscrn].data + screens[destscrn].pitch * y + x;

  for ( ; height>0 ; height--)
    {
      memcpy (dest, src, width);
      src += screens[srcscrn].pitch;
      dest += screens[destscrn].pitch;
    }
}

static void FUNC_V_FillRaw(int lump, int scrn, int x, int y, int lumpwidth, int lumpheight, int width, int height, int x_offset, int y_offset, enum patch_translation_e flags)
{
  int x0, y0;
  int x1, y1;
  int w, h;
  int xoff, yoff;
  
  // Clipping Stuff
  {
    x0 = x;
    y0 = y;
    x1 = x + width;
    y1 = y + height;

    if (x0 < 0)
      x0 = 0;

    if (y0 < 0)
      y0 = 0;

    if (x1 > SCREENWIDTH)
      x1 = SCREENWIDTH;

    if (y1 > SCREENHEIGHT)
      y1 = SCREENHEIGHT;

    w = x1 - x0;
    h = y1 - y0;

    if (w <= 0 || h <= 0)
      return;

    xoff = x0 - x + x_offset;
    yoff = y0 - y + y_offset;
  }

  // Actual Drawing Stuff
  {
    int sx, sy, src_x_offset, src_y_offset;
    const byte *data = W_LumpByNum(lump);
    int pitch = screens[scrn].pitch;
    byte *dest = screens[scrn].data + y0 * pitch + x0;
    const byte *row;

    // Ratio Correction
    stretch_param_t* stretch = dsda_StretchParams(flags);
    float ratio_x = stretch->video->width / 320.f;
    float ratio_y = stretch->video->height / 200.f;

    for (sy = 0; sy < h; ++sy)
    {
      src_y_offset = (int)((sy + yoff) / ratio_y) % lumpheight;
      row = data + src_y_offset * lumpwidth;

      for (sx = 0; sx < w; ++sx)
      {
        src_x_offset = (int)((sx + xoff) / ratio_x) % lumpwidth;
        dest[sy * pitch + sx] = row[src_x_offset];
      }
    }
  }
}

static void FUNC_V_FillRawPrecise(int lump, int scrn, float x, float y, int lumpwidth, int lumpheight, int width, int height, int x_offset, int y_offset, enum patch_translation_e flags)
{
  FUNC_V_FillRaw(lump, scrn, (int)x, (int)y, lumpwidth, lumpheight, width, height, x_offset, y_offset, flags);
}

static void FUNC_V_FillFlat(int lump, int scrn, int x, int y, int width, int height, enum patch_translation_e flags)
{
  FUNC_V_FillRaw(lump + firstflat, scrn, x, y, 64, 64, width, height, x, y, flags);
}

static void FUNC_V_FillPatch(int lump, int scrn, int x, int y, int width, int height, enum patch_translation_e flags)
{
  int sx, sy, w, h;

  w = R_NumPatchWidth(lump);
  h = R_NumPatchHeight(lump);

  for (sy = y; sy < y + height; sy += h)
  {
    int remaining_height = (y + height) - sy;
    int patch_draw_height = MIN(h, remaining_height);

    for (sx = x; sx < x + width; sx += w)
    {
      int remaining_width = (x + width) - sx;
      int patch_draw_width = MIN(w, remaining_width);

      int clip_right  = w - patch_draw_width;    // pixels to crop from right
      int clip_bottom = h - patch_draw_height;   // pixels to crop from bottom

      V_DrawNumPatchCropBG(sx, sy, lump, 0, clip_bottom, 0, clip_right, CR_DEFAULT, flags);
    }
  }
}

//
// V_Init
//
// Allocates the 4 full screen buffers in low DOS memory
// No return
//

void V_Init (void)
{
  int  i;

  // reset the all
  for (i = 0; i<NUM_SCREENS; i++) {
    screens[i].data = NULL;
    screens[i].not_on_heap = false;
    screens[i].width = 0;
    screens[i].height = 0;
    screens[i].pitch = 0;
  }
}

//
// V_DrawMemPatch
//
// CPhipps - unifying patch drawing routine, handles all cases and combinations
//  of stretching, flipping and translating
//
// This function is big, hopefully not too big that gcc can't optimise it well.
// In fact it packs pretty well, there is no big performance lose for all this merging;
// the inner loops themselves are just the same as they always were
// (indeed, laziness of the people who wrote the 'clones' of the original V_DrawPatch
//  means that their inner loops weren't so well optimised, so merging code may even speed them).
//
static void V_DrawPatch(int x, int y, int scrn, const rpatch_t *patch,
        const byte *transmap, const byte *colortr,
        int clip_top, int clip_bottom, int clip_left, int clip_right,
        enum patch_translation_e flags)
{
    int    col;
    int    pitch = screens[scrn].pitch;
    int    w = patch->width;
    int    start_col = clip_left;
    int    end_col = patch->width - clip_right - 1;

    int TR = flags & VPT_COLOR;
    int TL = flags & VPT_TRANSMAP;
    int ALT_TL = flags & VPT_ALT_TRANSMAP;

    if (y<0 || y+patch->height > ((flags & VPT_STRETCH) ? 200 :  SCREENHEIGHT)) {
      // killough 1/19/98: improved error message:
      lprintf(LO_WARN, "V_DrawPatch: Patch (%d,%d)-(%d,%d) exceeds LFB in vertical direction (horizontal is clipped)\n"
              "Bad V_DrawPatch (flags=%u)", x, y, x+patch->width, y+patch->height, flags);
      return;
    }

    w--; // CPhipps - note: w = width-1 now, speeds up flipping

    if (flags & VPT_FLIP)
    {
      int tmp = start_col;
      start_col = w - end_col;
      end_col = w - tmp;
    }

    for (col = start_col; col <= end_col; col++) {
      int i;
      const int colindex = (flags & VPT_FLIP) ? (w - col) : (col);
      const rcolumn_t *column = R_GetPatchColumn(patch, colindex);
      int screen_x = x + col;
      byte *desttop = screens[scrn].data+y*screens[scrn].pitch+screen_x;

      // screen_x accounts for crop
      if (screen_x < 0)
        continue;
      if (screen_x >= SCREENWIDTH)
        break;

      // step through the posts in a column
      for (i=0; i<column->numPosts; i++) {
        const rpost_t *post = &column->posts[i];
        int post_start, post_end, clipped_start, clipped_end;
        const byte *source;
        byte *dest;
        int count;

     // here's our patch cropping code
        {        
          post_start = post->topdelta;
          post_end = post_start + post->length;

     // vertical clipping
          clipped_start = post_start < clip_top ? clip_top : post_start;
          clipped_end = patch->height - clip_bottom;
          if (post_end > clipped_end)
              post_end = clipped_end;

     // fully clipped out
          if (post_end <= clipped_start)
              continue;
        }

        // killough 2/21/98: Unrolled and performance-tuned

        source = column->pixels + clipped_start;
        dest = desttop + clipped_start * pitch;
        count = post_end - clipped_start;

     // both translucent and color translated
        if (TR && TL) {
          if ((count-=4)>=0)
            do {
              register byte s0,s1;
              s0 = source[0];
              s1 = source[1];
              s0 = transmap[(*dest<<8)+colortr[s0]];
              s1 = transmap[(*dest<<8)+colortr[s1]];
              dest[0] = s0;
              dest[pitch] = s1;
              dest += pitch*2;
              s0 = source[2];
              s1 = source[3];
              s0 = transmap[(*dest<<8)+colortr[s0]];
              s1 = transmap[(*dest<<8)+colortr[s1]];
              source += 4;
              dest[0] = s0;
              dest[pitch] = s1;
              dest += pitch*2;
            } while ((count-=4)>=0);
          if (count+=4)
            do {
              *dest = transmap[(*dest<<8)+colortr[*source++]];
              dest += pitch;
            } while (--count);
        }
     // both reverse translucent and color translated
        else if (TR && ALT_TL) {
          if ((count-=4)>=0)
            do {
              register byte s0,s1;
              s0 = source[0];
              s1 = source[1];
              s0 = transmap[*dest+(colortr[s0]<<8)];
              s1 = transmap[*dest+(colortr[s1]<<8)];
              dest[0] = s0;
              dest[pitch] = s1;
              dest += pitch*2;
              s0 = source[2];
              s1 = source[3];
              s0 = transmap[*dest+(colortr[s0]<<8)];
              s1 = transmap[*dest+(colortr[s1]<<8)];
              source += 4;
              dest[0] = s0;
              dest[pitch] = s1;
              dest += pitch*2;
            } while ((count-=4)>=0);
          if (count+=4)
            do {
              *dest = transmap[*dest+colortr[*source<<8]];
              source++;
              dest += pitch;
            } while (--count);
        }
    // color translated patch
        else if (TR) {
          if ((count-=4)>=0)
            do {
              register byte s0,s1;
              s0 = source[0];
              s1 = source[1];
              s0 = colortr[s0];
              s1 = colortr[s1];
              dest[0] = s0;
              dest[pitch] = s1;
              dest += pitch*2;
              s0 = source[2];
              s1 = source[3];
              s0 = colortr[s0];
              s1 = colortr[s1];
              source += 4;
              dest[0] = s0;
              dest[pitch] = s1;
              dest += pitch*2;
            } while ((count-=4)>=0);
          if (count+=4)
            do {
              *dest = colortr[*source++];
              dest += pitch;
            } while (--count);
        }
    // reverse translucent patch
        else if (ALT_TL) {
          if ((count-=4)>=0)
            do {
              register byte s0,s1;
              s0 = source[0];
              s1 = source[1];
              s0 = transmap[*dest+(s0<<8)];
              s1 = transmap[*dest+(s1<<8)];
              dest[0] = s0;
              dest[pitch] = s1;
              dest += pitch*2;
              s0 = source[2];
              s1 = source[3];
              s0 = transmap[*dest+(s0<<8)];
              s1 = transmap[*dest+(s1<<8)];
              source += 4;
              dest[0] = s0;
              dest[pitch] = s1;
              dest += pitch*2;
            } while ((count-=4)>=0);
          if (count+=4)
            do {
              *dest = transmap[*dest+(*source<<8)];
              source++;
              dest += pitch;
            } while (--count);
        }
    // translucent patch
        else if (TL) {
          if ((count-=4)>=0)
            do {
              register byte s0,s1;
              s0 = source[0];
              s1 = source[1];
              s0 = transmap[(*dest<<8)+s0];
              s1 = transmap[(*dest<<8)+s1];
              dest[0] = s0;
              dest[pitch] = s1;
              dest += pitch*2;
              s0 = source[2];
              s1 = source[3];
              s0 = transmap[(*dest<<8)+s0];
              s1 = transmap[(*dest<<8)+s1];
              source += 4;
              dest[0] = s0;
              dest[pitch] = s1;
              dest += pitch*2;
            } while ((count-=4)>=0);
          if (count+=4)
            do {
              *dest = transmap[(*dest<<8)+*source++];
              dest += pitch;
            } while (--count);
        }
    // normal patch
        else {
          if ((count-=4)>=0)
            do {
              register byte s0,s1;
              s0 = source[0];
              s1 = source[1];
              dest[0] = s0;
              dest[pitch] = s1;
              dest += pitch*2;
              s0 = source[2];
              s1 = source[3];
              source += 4;
              dest[0] = s0;
              dest[pitch] = s1;
              dest += pitch*2;
            } while ((count-=4)>=0);
          if (count+=4)
            do {
              *dest = *source++;
              dest += pitch;
            } while (--count);
        }
      }
    }
}

static void V_DrawPatchStretch(int x, int y, int scrn, const rpatch_t *patch,
        const byte *transmap, const byte *colortr,
        int clip_top, int clip_bottom, int clip_left, int clip_right,
        enum patch_translation_e flags)
{
    // CPhipps - move stretched patch drawing code here
    //         - reformat initialisers, move variables into inner blocks

    int   col;
    int   w = (patch->width << 16) - 1; // CPhipps - -1 for faster flipping
    int   left, right, top, bottom;
    int   DXI, DYI;
    int   deltay1;
    R_DrawColumn_f colfunc;
    draw_column_vars_t dcvars;
    draw_vars_t olddrawvars = drawvars;
    stretch_param_t *params = dsda_StretchParams(flags);
    int screen_clip_left, screen_clip_right;
    int screen_clip_top, screen_clip_bottom;

    int TR = flags & VPT_COLOR;
    int TL = flags & VPT_TRANSMAP;
    int ALT_TL = flags & VPT_ALT_TRANSMAP;

    R_SetDefaultDrawColumnVars(&dcvars);

    drawvars.topleft = screens[scrn].data;
    drawvars.pitch = screens[scrn].pitch;

    if (TR && TL) {     // both translucent and color translated
      colfunc = R_GetDrawColumnFunc(RDC_PIPELINE_TRTL, RDRAW_FILTER_NONE);
      dcvars.translation = colortr;
      tranmap = transmap;
    }
    else if (TR && ALT_TL) {     // both translucent and color translated
      colfunc = R_GetDrawColumnFunc(RDC_PIPELINE_ALT_TRTL, RDRAW_FILTER_NONE);
      dcvars.translation = colortr;
      tranmap = transmap;
    }
    else if (TR) {    // color translated patch
      colfunc = R_GetDrawColumnFunc(RDC_PIPELINE_TRANSLATED, RDRAW_FILTER_NONE);
      dcvars.translation = colortr;
    }
    else if (ALT_TL) {     // both translucent and color translated
      colfunc = R_GetDrawColumnFunc(RDC_PIPELINE_ALT_TL, RDRAW_FILTER_NONE);
      tranmap = transmap;
    }
    else if (TL) {    // translucent patch
      colfunc = R_GetDrawColumnFunc(RDC_PIPELINE_TRANSLUCENT, RDRAW_FILTER_NONE);
      tranmap = transmap;
    }
    else {    // normal patch
      colfunc = R_GetDrawColumnFunc(RDC_PIPELINE_STANDARD, RDRAW_FILTER_NONE);
    }

    DXI = params->video->xstep;
    DYI = params->video->ystep;

    // Stretch cropping code (required for dcvars.clip_top and dcvars.clip_bottom)
    {
        int clip_x1, clip_x2, clip_y1, clip_y2;
        int clip_deltay1 = params->deltay1;

        clip_x1 = x + clip_left;
        clip_x2 = x + patch->width - clip_right;
        clip_y1 = y + clip_top;
        clip_y2 = y + patch->height - clip_bottom;

        // Left
        if (clip_x1 < 0 || clip_x1 > 320)
          screen_clip_left = (clip_x1 * params->video->width) / 320;
        else
          screen_clip_left = params->video->x1lookup[clip_x1];

        // Right
        if (clip_x2 < 0 || clip_x2 > 320)
          screen_clip_right = (clip_x2 * params->video->width) / 320;
        else
          screen_clip_right = params->video->x2lookup[clip_x2];

        // Top
        if (clip_y1 < 0 || clip_y1 > 200)
          screen_clip_top = (clip_y1 * params->video->height) / 200;
        else
          screen_clip_top = params->video->y1lookup[clip_y1];

        // Bottom
        if (clip_y2 < 0 || clip_y2 > 200)
          screen_clip_bottom = (clip_y2 * params->video->height) / 200;
        else
          screen_clip_bottom = params->video->y2lookup[(clip_y2 - 1)]; // -1 needed here to match "bottom" below

        if (TOP_ALIGNMENT(flags & VPT_STRETCH_MASK))
          clip_deltay1 += global_patch_top_offset;

        screen_clip_left   += params->deltax1;
        screen_clip_right  += params->deltax2;
        screen_clip_top    += clip_deltay1;
        screen_clip_bottom += clip_deltay1;

        // This is what actually does the y patch cropping
        dcvars.clip_top    = screen_clip_top;
        dcvars.clip_bottom = screen_clip_bottom;
    }

    left = (x < 0 || x > 320 ? (x * params->video->width) / 320 : params->video->x1lookup[x]);
    top =  (y < 0 || y > 200 ? (y * params->video->height) / 200 : params->video->y1lookup[y]);

    if (x + patch->width < 0 || x + patch->width > 320)
      right = ( ((x + patch->width) * params->video->width - 1) / 320 );
    else
      right = params->video->x2lookup[x + patch->width - 1];

    if (y + patch->height < 0 || y + patch->height > 200)
      bottom = ( ((y + patch->height - 0) * params->video->height) / 200 );
    else
      bottom = params->video->y2lookup[y + patch->height - 1];

    deltay1 = params->deltay1;

    if (TOP_ALIGNMENT(flags & VPT_STRETCH_MASK))
      deltay1 += global_patch_top_offset;

    left   += params->deltax1;
    right  += params->deltax2;
    top    += deltay1;
    bottom += deltay1;

    dcvars.texheight = patch->height;
    dcvars.iscale = DYI;
    dcvars.drawingmasked = MAX(patch->width, patch->height) > 8;

    col = 0;

    for (dcvars.x=left; dcvars.x<=right; dcvars.x++, col+=DXI) {
      int i;
      const int colindex = (flags & VPT_FLIP) ? ((w - col)>>16): (col>>16);
      const rcolumn_t *column = R_GetPatchColumn(patch, colindex);
      const rcolumn_t *prevcolumn = R_GetPatchColumn(patch, colindex-1);
      const rcolumn_t *nextcolumn = R_GetPatchColumn(patch, colindex+1);

      // ignore this column if it's to the left of our clampRect
      if (dcvars.x < 0)
        continue;
      if (dcvars.x >= SCREENWIDTH)
        break;

      // clamp for crop
      {
        if (dcvars.x < screen_clip_left)
          continue;
        if (dcvars.x >= screen_clip_right)
          break;
      }

      // step through the posts in a column
      for (i=0; i<column->numPosts; i++) {
        const rpost_t *post = &column->posts[i];
        int yoffset = 0;

        //e6y
        if (!(flags & VPT_STRETCH_MASK))
        {
          dcvars.yl = y + post->topdelta;
          dcvars.yh = ((((y + post->topdelta + post->length) << 16) - (FRACUNIT>>1))>>FRACBITS);
        }
        else
        {
          // e6y
          // More accurate patch drawing from Eternity.
          // Predefined arrays are used instead of dynamic calculation
          // of the top and bottom screen coordinates of a column.

          int tmpy;

          tmpy = y + post->topdelta;
          if (tmpy < 0 || tmpy > 200)
            dcvars.yl = (tmpy * params->video->height) / 200 + deltay1;
          else
            dcvars.yl = params->video->y1lookup[tmpy] + deltay1;

          tmpy = y + post->topdelta + post->length - 1;
          if (tmpy < 0 || tmpy > 200)
            dcvars.yh = (tmpy * params->video->height) / 200 + deltay1;
          else
            dcvars.yh = params->video->y2lookup[tmpy] + deltay1;
        }
        dcvars.edgeslope = post->slope;

        if ((dcvars.yh < 0) || (dcvars.yh < top))
          continue;
        if ((dcvars.yl >= SCREENHEIGHT) || (dcvars.yl >= bottom))
          continue;

        if (dcvars.yh >= bottom) {
          //dcvars.yh = bottom-1;
          dcvars.edgeslope &= ~RDRAW_EDGESLOPE_BOT_MASK;
        }
        if (dcvars.yh >= SCREENHEIGHT) {
          dcvars.yh = SCREENHEIGHT-1;
          dcvars.edgeslope &= ~RDRAW_EDGESLOPE_BOT_MASK;
        }

        if (dcvars.yl < 0) {
          yoffset = (0-dcvars.yl) * 200/params->video->height;
          dcvars.yl = 0;
          dcvars.edgeslope &= ~RDRAW_EDGESLOPE_TOP_MASK;
        }
        if (dcvars.yl < top) {
          yoffset = (top-dcvars.yl) * 200/params->video->height;
          dcvars.yl = top;
          dcvars.edgeslope &= ~RDRAW_EDGESLOPE_TOP_MASK;
        }

        dcvars.source = column->pixels + post->topdelta + yoffset;
        dcvars.prevsource = prevcolumn ? prevcolumn->pixels + post->topdelta + yoffset: dcvars.source;
        dcvars.nextsource = nextcolumn ? nextcolumn->pixels + post->topdelta + yoffset: dcvars.source;

        dcvars.texturemid = -((dcvars.yl-centery)*dcvars.iscale);

        //e6y
        dcvars.dy = deltay1;
        dcvars.flags |= DRAW_COLUMN_ISPATCH;

        colfunc(&dcvars);
      }
    }

    R_ResetColumnBuffer();
    drawvars = olddrawvars;
}

typedef struct {
  dboolean active;
  int trans;
  const byte *colortr;
  const byte *transmap;
  enum patch_translation_e flags;
} v_patchinfo_t;

#define NO_TRANS -1

v_patchinfo_t V_GetMainDrawInfo(int cm, enum patch_translation_e flags)
{
  v_patchinfo_t patch;
  int trans_context;
  extern int dsda_ExHudTranslucency(void);

  patch.transmap = NULL;
  patch.flags = flags;
  patch.trans = NO_TRANS;

  // color translation
  if (cm == CR_DEFAULT)
    patch.colortr = &colormaps[0][0];
  else if (cm == CR_SHADOW)
    patch.colortr = &colormaps[0][256 * 31];
  else if (cm == CR_DARKEN)
    patch.colortr = &colormaps[0][256 * 15];
  else if (cm < CR_LIMIT)
    patch.colortr = colrngs[cm];
  else
    patch.colortr = translationtables + 256*((cm - CR_LIMIT) - 1);

  // CPhipps - null translation pointer => no translation
  if (!patch.colortr)
    patch.flags &= ~VPT_COLOR;

  if (patch.flags & VPT_TRANSMAP)
  {
    patch.trans = tran_filter_pct;
    trans_context = TMC_MAIN;
  }
  else if (patch.flags & VPT_ALT_TRANSMAP)
  {
    patch.trans = alttint_filter_pct;
    trans_context = TMC_ALT;
  }

  // ExHUD translucency logic
  if ((patch.flags & VPT_EX_TRANS) && dsda_ExHudTranslucency())
  {
    patch.trans = exhud_tran_filter_pct;
    trans_context = TMC_EXHUD;
  
    if (patch.flags & VPT_TRANSMAP)
    {
      patch.trans = exhud_tint_filter_pct;
      trans_context = TMC_EXHUD_MAIN;
    }
    else if (patch.flags & VPT_ALT_TRANSMAP)
    {
      patch.trans = exhud_alttint_filter_pct;
      trans_context = TMC_EXHUD_ALT;
    }
  
    if (!(patch.flags & VPT_TRANSMAP))
      patch.flags |= VPT_TRANSMAP;
  }

  // if close, just go with 100
  if (patch.trans == 99)
  {
    patch.trans = NO_TRANS;
    patch.flags &= ~VPT_TRANSMAP;
  }

  if (patch.trans != NO_TRANS)
    patch.transmap = dsda_TranMap_Custom(patch.trans, trans_context);

  return patch;
}

v_patchinfo_t V_GetShadowDrawInfo(enum patch_translation_e flags, int shadowtype) {
  v_patchinfo_t shadow = { 0 };
  int trans_context;
  shadow.trans = NO_TRANS;

  if ((shadowtype == SHADOW_DEFAULT && !dsda_ShadowTranslucency()))
    shadowtype = 0;

  if (!shadowtype)
  {
    shadow.active = false;
    return shadow;
  }

  shadow.active = true;
  shadow.colortr = &colormaps[0][256 * 31]; // close to black
  shadow.transmap = NULL;
  shadow.flags = flags | VPT_SHADOW;
  shadow.trans = (shadowtype == SHADOW_DEFAULT) ? shadow_ui_filter_pct : tran_filter_pct;
  trans_context = (shadowtype == SHADOW_DEFAULT) ? TMC_UI_SHADOW : TMC_SHADOW;

  // Shadow always has translucency and color
  if (!(shadow.flags & VPT_TRANSMAP))
    shadow.flags |= VPT_TRANSMAP;
  if (!(shadow.flags & VPT_COLOR))
    shadow.flags |= VPT_COLOR;

  // Ex hud stuff
  if ((shadow.flags & VPT_EX_TRANS) && dsda_ExHudTranslucency())
  {
    shadow.trans = exhud_shadow_filter_pct;
    trans_context = TMC_EXHUD_SHADOW;
  }

  // if close, just go with 100
  if (shadow.trans == 99)
  {
    shadow.trans = NO_TRANS;
    shadow.flags &= ~VPT_TRANSMAP;
  }

  if (shadow.trans != NO_TRANS)
    shadow.transmap = dsda_TranMap_Custom(shadow.trans, trans_context);

  return shadow;
}

void V_DrawMemPatch(int x, int y, int scrn, const rpatch_t *patch,
        dboolean center, int shadowtype,
        int clip_top, int clip_bottom, int clip_left, int clip_right,
        int cm, enum patch_translation_e flags)
{
  v_patchinfo_t patchinfo = {0}, shadowinfo = {0};
  int shadow_x, shadow_y;

  // remove offsets
  if (!(flags & VPT_NOOFFSET))
  {
    y -= patch->topoffset;
    x -= patch->leftoffset;
  }

  // CPhipps - auto-no-stretch if not high-res
  if ((flags & VPT_STRETCH_MASK) && SCREEN_320x200)
    flags &= ~VPT_STRETCH_MASK;

  // Clamp crop values if they exceed patch size
  if (clip_left + clip_right >= patch->width) {
      clip_left = 0;
      clip_right = 0;
  }

  if (clip_top + clip_bottom >= patch->height) {
      clip_top = 0;
      clip_bottom = 0;
  }

  // [FG] automatically center wide patches without horizontal offset
  if (center)
  {
    if (patch->width > 320 && patch->leftoffset == 0)
      x -= (patch->width - 320) / 2;
  }

  patchinfo  = V_GetMainDrawInfo(cm, flags);
  shadowinfo = V_GetShadowDrawInfo(flags, shadowtype);

  // Clamp shadow so it doesn't exceed screen bounds,
  // Stops V_DrawPatch vertical overflow error.
  {
    shadow_x = x + shadowtype;
    shadow_y = y + shadowtype;

    if (shadow_x < 0) shadow_x = 0;
    if (shadow_x + patch->width > SCREENWIDTH)
        shadow_x = SCREENWIDTH - patch->width;

    if (shadow_y < 0) shadow_y = 0;
    if (shadow_y + patch->height > SCREENHEIGHT)
        shadow_y = SCREENHEIGHT - patch->height;
  }

  // Draw patch unscaled
  if (!(flags & VPT_STRETCH_MASK)) {
    if (shadowinfo.active)
      V_DrawPatch(shadow_x, shadow_y, scrn, patch, shadowinfo.transmap, shadowinfo.colortr, clip_top, clip_bottom, clip_left, clip_right, shadowinfo.flags);
    V_DrawPatch(x, y, scrn, patch, patchinfo.transmap, patchinfo.colortr, clip_top, clip_bottom, clip_left, clip_right, patchinfo.flags);
  }

  // Or draw scaled patch with pipelines
  else {
    if (shadowinfo.active)
      V_DrawPatchStretch(shadow_x, shadow_y, scrn, patch, shadowinfo.transmap, shadowinfo.colortr, clip_top, clip_bottom, clip_left, clip_right, shadowinfo.flags);
    V_DrawPatchStretch(x, y, scrn, patch, patchinfo.transmap, patchinfo.colortr, clip_top, clip_bottom, clip_left, clip_right, patchinfo.flags);
  }
}

//
// FUNC_V_DrawShaded
//
// Adapted from Woof.
//
// This uses a dark colormap to create
// a dark faded background under menus.
//

#define FULLSHADE 20
byte shademap[FULLSHADE + 1][256];
static dboolean cacheshade = false;

static void FUNC_V_DrawShaded(int x, int y, int width, int height, int shade)
{ 
  byte* dest;
  int ix, iy;

  if (shade <= 0 || shade > FULLSHADE) return;

  // Cache shade for performance :)
  if (!cacheshade)
  {
    for (int s = 0; s <= FULLSHADE; s++)
    {
      for (int c = 0; c < 256; c++)
      {
        shademap[s][c] = colormaps[0][s * 256 + c];
      }
    }
    cacheshade = true;
  }

  for (iy = y; iy < y + height; ++iy)
  {
    dest = screens[0].data + screens[0].pitch * iy + x;

    for (ix = x; ix < x + width; ++ix)
    {
      dest[0] = shademap[shade][dest[0]];
      dest++;
    }
  }
}


// CPhipps - some simple, useful wrappers for that function, for drawing patches from wads

// CPhipps - GNU C only suppresses generating a copy of a function if it is
// static inline; other compilers have different behaviour.
// This inline is _only_ for the function below

static void FUNC_V_DrawNumPatch(int x, int y, int scrn, int lump, dboolean center,
         int clip_top, int clip_bottom, int clip_left, int clip_right,
         int cm, enum patch_translation_e flags)
{
  V_DrawMemPatch(x, y, scrn, R_PatchByNum(lump), center, false, clip_top, clip_bottom, clip_left, clip_right, cm, flags);
}

static void FUNC_V_DrawNumPatchPrecise(float x, float y, int scrn, int lump, dboolean center,
         float clip_top, float clip_bottom, float clip_left, float clip_right,
         int cm, enum patch_translation_e flags)
{
  V_DrawMemPatch((int)x, (int)y, scrn, R_PatchByNum(lump), center, false, (int)clip_top, (int)clip_bottom, (int)clip_left, (int)clip_right, cm, flags);
}

static void FUNC_V_DrawShadowedNumPatch(int x, int y, int scrn, int lump, dboolean center, int shadowtype,
         int clip_top, int clip_bottom, int clip_left, int clip_right,
         int cm, enum patch_translation_e flags)
{
  V_DrawMemPatch(x, y, scrn, R_PatchByNum(lump), center, shadowtype, clip_top, clip_bottom, clip_left, clip_right, cm, flags);
}

static void FUNC_V_DrawShadowedNumPatchPrecise(float x, float y, int scrn, int lump, dboolean center, int shadowtype,
         float clip_top, float clip_bottom, float clip_left, float clip_right,
         int cm, enum patch_translation_e flags)
{
  V_DrawMemPatch((int)x, (int)y, scrn, R_PatchByNum(lump), center, shadowtype, (int)clip_top, (int)clip_bottom, (int)clip_left, (int)clip_right, cm, flags);
}

static int currentPaletteIndex = 0;

void V_TouchPalette(void)
{
  V_SetPalette(currentPaletteIndex);
}

//
// V_SetPalette
//
// CPhipps - New function to set the palette to palette number pal.
// Handles loading of PLAYPAL and calls I_SetPalette

void V_SetPalette(int pal)
{
  currentPaletteIndex = pal;

  if (V_IsOpenGLMode()) {
    gld_SetPalette(pal);
  } else {
    I_SetPalette(pal);
  }
}

void V_SetPlayPal(int playpal_index)
{
  dsda_SetPlayPal(playpal_index);
  R_UpdatePlayPal();
  V_SetPalette(currentPaletteIndex);

  if (V_IsOpenGLMode())
  {
    gld_FlushTextures();
  }
}

//
// V_FillRect
//
// CPhipps - New function to fill a rectangle with a given colour
static void V_FillRect8(int scrn, int x, int y, int width, int height, byte colour)
{
  byte* dest = screens[scrn].data + x + y*screens[scrn].pitch;
  while (height--) {
    memset(dest, colour, width);
    dest += screens[scrn].pitch;
  }
}

static void V_FillRectTrans8(int scrn, int x, int y, int width, int height, byte colour, const byte* transmap)
{
  byte* dest;
  int pitch = screens[scrn].pitch;

  for (int iy = y; iy < y + height; ++iy)
  {
    dest = screens[scrn].data + pitch * iy + x;

    for (int ix = 0; ix < width; ++ix)
    {
      *dest = transmap[(*dest << 8) | colour];
      dest++;
    }
  }
}

static void FUNC_V_FillRectTrans(int scrn, int x, int y, int width, int height, byte colour, const byte* transmap)
{
  if (!dsda_MenuTranslucency())
    V_FillRect8(scrn, x, y, width, height, colour);
  else
    V_FillRectTrans8(scrn, x, y, width, height, colour, transmap);
}

//
// First, I tried to implement V_ShadeLine from Vanilla Heretic.
// Problem is that due to the x/y lookuptables, sometimes
// the shadow would result in weird behaviours under
// the resolutions 426x200 / 640x400.
//
// V_FillRectShaded() fixes these issues
// as well as being more useful
//
void FUNC_V_FillRectShaded(int x, int y, int w, int h, int start_shade, int end_shade, int vertical)
{
  byte *dest = screens[0].data + y * screens[0].pitch + x;
  int pitch = screens[0].pitch;

  int blocks = vertical ? h : w;
  if (blocks <= 1) return;

  for (int j = 0; j < h; j++)
  {
    for (int i = 0; i < w; i++)
    {
      int block_size = vertical ? j : i;
      int shade = start_shade + ((end_shade - start_shade) * block_size) / (blocks - 1);

      const byte *shades = colormaps[0] + 9 * 256 + (shade * 2) * 256;
      dest[i] = shades[dest[i]];
    }

    dest += pitch;
  }
}

static void WRAP_V_DrawLine(fline_t* fl, int color);
static void V_PlotPixel8(int scrn, int x, int y, byte color);

static void WRAP_V_DrawLineWu(fline_t* fl, int color);
static void V_PlotPixelWu8(int scrn, int x, int y, byte color, int weight);

static void WRAP_gld_BeginUIDraw(void)
{
  gld_BeginUIDraw();
}
static void WRAP_gld_EndUIDraw(void)
{
  gld_EndUIDraw();
}
static void WRAP_gld_BeginAutomapDraw(void)
{
  gld_BeginAutomapDraw();
}
static void WRAP_gld_EndAutomapDraw(void)
{
  gld_EndAutomapDraw();
}
static void WRAP_gld_BeginMenuDraw(void)
{
  gld_BeginMenuDraw();
}
static void WRAP_gld_EndMenuDraw(void)
{
  gld_EndMenuDraw();
}
static void WRAP_gld_FillRect(int scrn, int x, int y, int width, int height, byte colour)
{
  gld_FillBlock(x,y,width,height,colour,100);
}
static void WRAP_gld_FillRectTrans(int scrn, int x, int y, int width, int height, byte colour, const byte* tranmap)
{
  int trans = dsda_MenuTranslucency() ? menu_ui_filter_pct : 100;

  gld_FillBlock(x,y,width,height,colour,trans);
}
static void WRAP_gld_FillRectShaded(int x, int y, int w, int h, int start_shade, int end_shade, int vertical) {
  gld_FillBlockShaded(x,y,w,h,start_shade,end_shade,vertical);
}
static void WRAP_gld_CopyRect(int srcscrn, int destscrn, int x, int y, int width, int height, enum patch_translation_e flags)
{
}
static void WRAP_gld_FillRaw(int lump, int n, int x, int y, int lumpwidth, int lumpheight, int width, int height, int x_offset, int y_offset, enum patch_translation_e flags)
{
  gld_FillRaw(lump, x, y, lumpwidth, lumpheight, width, height, x_offset, y_offset, flags);
}
static void WRAP_gld_FillRawPrecise(int lump, int n, float x, float y, int lumpwidth, int lumpheight, int width, int height, int x_offset, int y_offset, enum patch_translation_e flags)
{
  gld_FillRaw_f(lump, x, y, lumpwidth, lumpheight, width, height, x_offset, y_offset, flags);
}
static void WRAP_gld_FillFlat(int lump, int n, int x, int y, int width, int height, enum patch_translation_e flags)
{
  gld_FillFlat(lump, x, y, width, height, flags);
}
static void WRAP_gld_FillPatch(int lump, int n, int x, int y, int width, int height, enum patch_translation_e flags)
{
  gld_FillPatch(lump, x, y, width, height, flags);
}
static void WRAP_gld_DrawNumPatch(int x, int y, int scrn, int lump, dboolean center, int clip_top, int clip_bottom, int clip_left, int clip_right, int cm, enum patch_translation_e flags)
{
  gld_DrawNumPatch(x,y,lump,center,false,clip_top,clip_bottom,clip_left,clip_right,cm,flags);
}
static void WRAP_gld_DrawNumPatchPrecise(float x, float y, int scrn, int lump, dboolean center, float clip_top, float clip_bottom, float clip_left, float clip_right, int cm, enum patch_translation_e flags)
{
  gld_DrawNumPatch_f(x,y,lump,center,false,clip_top,clip_bottom,clip_left,clip_right,cm,flags);
}
static void WRAP_gld_DrawShadowedNumPatch(int x, int y, int scrn, int lump, dboolean center, int offset, int clip_top, int clip_bottom, int clip_left, int clip_right, int cm, enum patch_translation_e flags)
{
  int shadow = (offset == SHADOW_DEFAULT && dsda_ShadowTranslucency()) || (offset == SHADOW_RAVEN);

  if (shadow)
    gld_DrawNumPatch(x+offset,y+offset,lump,center,offset,0,0,0,0,cm,flags|VPT_SHADOW); // draw offset shadow

  gld_DrawNumPatch(x,y,lump,center,false,0,0,0,0,cm,flags);
}
static void WRAP_gld_DrawShadowedNumPatchPrecise(float x, float y, int scrn, int lump, dboolean center, int offset, float clip_top, float clip_bottom, float clip_left, float clip_right, int cm, enum patch_translation_e flags)
{
  int shadow = (offset == SHADOW_DEFAULT && dsda_ShadowTranslucency()) || (offset == SHADOW_RAVEN);

  if (shadow)
    gld_DrawNumPatch_f(x+offset,y+offset,lump,center,offset,0,0,0,0,cm,flags|VPT_SHADOW); // draw offset shadow

  gld_DrawNumPatch_f(x,y,lump,center,false,0,0,0,0,cm,flags);
}
static void V_PlotPixelGL(int scrn, int x, int y, byte color) {
  gld_DrawPoint(x, y, color);
}
static void V_PlotPixelWuGL(int scrn, int x, int y, byte color, int weight) {
  V_PlotPixelGL(scrn, x, y, color);
}
static void WRAP_gld_DrawLine(fline_t* fl, int color)
{
  gld_DrawLine_f(fl->a.fx, fl->a.fy, fl->b.fx, fl->b.fy, color);
}
static void WRAP_gld_DrawShaded(int x, int y, int width, int height, int shade)
{
  gld_DrawShaded(x, y, width, height, shade);
}

static void NULL_BeginUIDraw(void) {}
static void NULL_EndUIDraw(void) {}
static void NULL_BeginAutomapDraw(void) {}
static void NULL_EndAutomapDraw(void) {}
static void NULL_BeginMenuDraw(void) {}
static void NULL_EndMenuDraw(void) {}
static void NULL_FillRect(int scrn, int x, int y, int width, int height, byte colour) {}
static void NULL_FillRectTrans(int scrn, int x, int y, int width, int height, byte colour, const byte* transmap) {}
static void NULL_FillRectShaded(int x, int y, int width, int height, int start_shade, int end_shade, int vertical) {}
static void NULL_CopyRect(int srcscrn, int destscrn, int x, int y, int width, int height, enum patch_translation_e flags) {}
static void NULL_FillFlat(int lump, int n, int x, int y, int width, int height, enum patch_translation_e flags) {}
static void NULL_FillRaw(int lump, int n, int x, int y, int lumpwidth, int lumpheight, int width, int height, int x_offset, int y_offset, enum patch_translation_e flags) {}
static void NULL_FillRawPrecise(int lump, int n, float x, float y, int lumpwidth, int lumpheight, int width, int height, int x_offset, int y_offset, enum patch_translation_e flags) {}
static void NULL_FillPatch(int lump, int n, int x, int y, int width, int height, enum patch_translation_e flags) {}
static void NULL_DrawNumPatch(int x, int y, int scrn, int lump, dboolean center, int clip_top, int clip_bottom, int clip_left, int clip_right, int cm, enum patch_translation_e flags) {}
static void NULL_DrawNumPatchPrecise(float x, float y, int scrn, int lump, dboolean center, float clip_top, float clip_bottom, float clip_left, float clip_right, int cm, enum patch_translation_e flags) {}
static void NULL_DrawShadowedNumPatch(int x, int y, int scrn, int lump, dboolean center, int shadow, int clip_top, int clip_bottom, int clip_left, int clip_right, int cm, enum patch_translation_e flags) {}
static void NULL_DrawShadowedNumPatchPrecise(float x, float y, int scrn, int lump, dboolean center, int shadow, float clip_top, float clip_bottom, float clip_left, float clip_right, int cm, enum patch_translation_e flags) {}
static void NULL_PlotPixel(int scrn, int x, int y, byte color) {}
static void NULL_PlotPixelWu(int scrn, int x, int y, byte color, int weight) {}
static void NULL_DrawLine(fline_t* fl, int color) {}
static void NULL_DrawLineWu(fline_t* fl, int color) {}
static void NULL_DrawShaded(int x, int y, int width, int height, int shade) {}

static video_mode_t current_videomode = VID_MODESW;

V_BeginUIDraw_f V_BeginUIDraw = NULL_BeginUIDraw;
V_EndUIDraw_f V_EndUIDraw = NULL_EndUIDraw;
V_BeginUIDraw_f V_BeginAutomapDraw = NULL_BeginAutomapDraw;
V_EndUIDraw_f V_EndAutomapDraw = NULL_EndAutomapDraw;
V_BeginUIDraw_f V_BeginMenuDraw = NULL_BeginMenuDraw;
V_EndUIDraw_f V_EndMenuDraw = NULL_EndMenuDraw;
V_CopyRect_f V_CopyRect = NULL_CopyRect;
V_FillRectGen_f V_FillRectGen = NULL_FillRect;
V_FillRectTrans_f V_FillRectTrans = NULL_FillRectTrans;
V_FillRectShaded_f V_FillRectShaded = NULL_FillRectShaded;
V_DrawNumPatchGen_f V_DrawNumPatchGen = NULL_DrawNumPatch;
V_DrawNumPatchGenPrecise_f V_DrawNumPatchGenPrecise = NULL_DrawNumPatchPrecise;
V_DrawShadowedNumPatchGen_f V_DrawShadowedNumPatchGen = NULL_DrawShadowedNumPatch;
V_DrawShadowedNumPatchGenPrecise_f V_DrawShadowedNumPatchGenPrecise = NULL_DrawShadowedNumPatchPrecise;
V_FillFlat_f V_FillFlat = NULL_FillFlat;
V_FillRaw_f V_FillRaw = NULL_FillRaw;
V_FillRawPrecise_f V_FillRawPrecise = NULL_FillRawPrecise;
V_FillPatch_f V_FillPatch = NULL_FillPatch;
V_PlotPixel_f V_PlotPixel = NULL_PlotPixel;
V_PlotPixelWu_f V_PlotPixelWu = NULL_PlotPixelWu;
V_DrawLine_f V_DrawLine = NULL_DrawLine;
V_DrawLineWu_f V_DrawLineWu = NULL_DrawLineWu;
V_DrawShaded_f V_DrawShaded = NULL_DrawShaded;

//
// V_InitMode
//
void V_InitMode(video_mode_t mode) {
  switch (mode) {
    case VID_MODESW:
      lprintf(LO_DEBUG, "V_InitMode: using software video mode\n");
      V_BeginUIDraw = NULL_BeginUIDraw; // [XA] no-op in software
      V_EndUIDraw = NULL_EndUIDraw; // [XA] ditto for the other begin/ends
      V_BeginAutomapDraw = NULL_BeginAutomapDraw;
      V_EndAutomapDraw = NULL_EndAutomapDraw;
      V_BeginMenuDraw = NULL_BeginMenuDraw;
      V_EndMenuDraw = NULL_EndMenuDraw;
      V_CopyRect = FUNC_V_CopyRect;
      V_FillRectGen = V_FillRect8;
      V_FillRectTrans = FUNC_V_FillRectTrans;
      V_FillRectShaded = FUNC_V_FillRectShaded;
      V_DrawNumPatchGen = FUNC_V_DrawNumPatch;
      V_DrawNumPatchGenPrecise = FUNC_V_DrawNumPatchPrecise;
      V_DrawShadowedNumPatchGen = FUNC_V_DrawShadowedNumPatch;
      V_DrawShadowedNumPatchGenPrecise = FUNC_V_DrawShadowedNumPatchPrecise;
      V_FillFlat = FUNC_V_FillFlat;
      V_FillRaw = FUNC_V_FillRaw;
      V_FillRawPrecise = FUNC_V_FillRawPrecise;
      V_FillPatch = FUNC_V_FillPatch;
      V_PlotPixel = V_PlotPixel8;
      V_PlotPixelWu = V_PlotPixelWu8;
      V_DrawLine = WRAP_V_DrawLine;
      V_DrawLineWu = WRAP_V_DrawLineWu;
      V_DrawShaded = FUNC_V_DrawShaded;
      current_videomode = VID_MODESW;
      break;
    case VID_MODEGL:
      lprintf(LO_DEBUG, "V_InitMode: using OpenGL video mode\n");
      V_BeginUIDraw = WRAP_gld_BeginUIDraw;
      V_EndUIDraw = WRAP_gld_EndUIDraw;
      V_BeginAutomapDraw = WRAP_gld_BeginAutomapDraw;
      V_EndAutomapDraw = WRAP_gld_EndAutomapDraw;
      V_BeginMenuDraw = WRAP_gld_BeginMenuDraw;
      V_EndMenuDraw = WRAP_gld_EndMenuDraw;
      V_CopyRect = WRAP_gld_CopyRect;
      V_FillRectGen = WRAP_gld_FillRect;
      V_FillRectTrans = WRAP_gld_FillRectTrans;
      V_FillRectShaded = WRAP_gld_FillRectShaded;
      V_DrawNumPatchGen = WRAP_gld_DrawNumPatch;
      V_DrawNumPatchGenPrecise = WRAP_gld_DrawNumPatchPrecise;
      V_DrawShadowedNumPatchGen = WRAP_gld_DrawShadowedNumPatch;
      V_DrawShadowedNumPatchGenPrecise = WRAP_gld_DrawShadowedNumPatchPrecise;
      V_FillFlat = WRAP_gld_FillFlat;
      V_FillRaw = WRAP_gld_FillRaw;
      V_FillRawPrecise = WRAP_gld_FillRawPrecise;
      V_FillPatch = WRAP_gld_FillPatch;
      V_PlotPixel = V_PlotPixelGL;
      V_PlotPixelWu = V_PlotPixelWuGL;
      V_DrawLine = WRAP_gld_DrawLine;
      V_DrawLineWu = WRAP_gld_DrawLine;
      V_DrawShaded = WRAP_gld_DrawShaded;
      current_videomode = VID_MODEGL;
      break;
  }
}

dboolean V_IsSoftwareMode(void) {
  return current_videomode == VID_MODESW;
}

dboolean V_IsOpenGLMode(void) {
  return current_videomode == VID_MODEGL;
}

dboolean V_IsUILightmodeIndexed(void) {
  return gl_ui_lightmode_indexed;
}

dboolean V_IsAutomapLightmodeIndexed(void) {
  return gl_automap_lightmode_indexed;
}

dboolean V_IsMenuLightmodeIndexed(void) {
  return gl_menu_lightmode_indexed;
}

void V_CopyScreen(int srcscrn, int destscrn)
{
  V_CopyRect(srcscrn, destscrn, 0, 0, SCREENWIDTH, SCREENHEIGHT, VPT_NONE);
}

//
// V_AllocScreen
//
void V_AllocScreen(screeninfo_t *scrn) {
  if (!scrn->not_on_heap)
    if ((scrn->pitch * scrn->height) > 0)
      //e6y: Clear the screen to black.
      scrn->data = Z_Calloc(scrn->pitch*scrn->height, 1);
}

//
// V_AllocScreens
//
void V_AllocScreens(void) {
  int i;

  for (i=0; i<NUM_SCREENS; i++)
    V_AllocScreen(&screens[i]);
}

//
// V_FreeScreen
//
void V_FreeScreen(screeninfo_t *scrn) {
  if (!scrn->not_on_heap) {
    Z_Free(scrn->data);
    scrn->data = NULL;
  }
}

//
// V_FreeScreens
//
void V_FreeScreens(void) {
  int i;

  for (i=0; i<NUM_SCREENS; i++)
    V_FreeScreen(&screens[i]);
}

//
// V_PlotPixel
//

static void V_PlotPixel8_1px(int scrn, int x, int y, byte color) {
  screens[scrn].data[x+screens[scrn].pitch*y] = color;
}

static void V_PlotCircle8(int scrn, int cx, int cy, int thickness, byte color)
{
  fixed_t radius, radius_sq;
  int row_radius;
  int dy;

  radius = thickness * FRACUNIT / 2;
  radius_sq = (fixed_t)(((int64_t)radius * radius) >> FRACBITS);
  row_radius = (radius + FRACUNIT - 1) >> FRACBITS;

  for (dy = -row_radius; dy <= row_radius; ++dy)
  {
    int y;
    fixed_t ydist, ydist_sq;
    fixed_t inside;
    fixed_t halfwidth;
    int x0, x1;
    byte *row;

    y = cy + dy;

    // screen clamp (col)
    if (y < 0 || y >= screens[scrn].height)
      continue;

    // Distance from center to this row at pixel center
    ydist = (dy << FRACBITS) + FRACUNIT/2;
    ydist_sq = (fixed_t)(((int64_t)ydist * ydist) >> FRACBITS);

    if (ydist_sq > radius_sq)
      continue;

    inside = radius_sq - ydist_sq;
    if (inside < 0) continue;
    halfwidth = (fixed_t)(sqrt((double)inside * (double)FRACUNIT) + 0.5);

    x0 = cx + ((-halfwidth + FRACUNIT/2) >> FRACBITS);
    x1 = cx + (( halfwidth - FRACUNIT/2) >> FRACBITS);

    // Quick screen bounds check
    if (x1 < 0 || x0 >= screens[scrn].width)
      continue;

    // screen clamp (row)
    if (x0 < 0) x0 = 0;
    if (x1 >= screens[scrn].width) x1 = screens[scrn].width - 1;

    row = screens[scrn].data + screens[scrn].pitch * y;
    memset(row + x0, color, (size_t)(x1 - x0 + 1));
  }
}

static void V_PlotPixel8(int scrn, int x, int y, byte color)
{
  int thickness = AM_GetLineWeight();

  if (thickness > 1)
    V_PlotCircle8(scrn, x, y, thickness, color);
  else
    V_PlotPixel8_1px(scrn, x, y, color);
}

#define PUTDOT(xx,yy,cc) V_PlotPixel(0,xx,yy,(byte)cc)

//
// WRAP_V_DrawLine()
//
// Draw a line in the frame buffer.
// Classic Bresenham w/ whatever optimizations needed for speed
//
// Passed the frame coordinates of line, and the color to be drawn
// Returns nothing
//
static void WRAP_V_DrawLine_1px(fline_t* fl, int color)
{
  register int x;
  register int y;
  register int dx;
  register int dy;
  register int sx;
  register int sy;
  register int ax;
  register int ay;
  register int d;

#ifdef RANGECHECK         // killough 2/22/98
  static int fuck = 0;

  // For debugging only
  if
  (
       fl->a.x < 0 || fl->a.x >= SCREENWIDTH
    || fl->a.y < 0 || fl->a.y >= SCREENHEIGHT
    || fl->b.x < 0 || fl->b.x >= SCREENWIDTH
    || fl->b.y < 0 || fl->b.y >= SCREENHEIGHT
  )
  {
    //jff 8/3/98 use logical output routine
    lprintf(LO_DEBUG, "fuck %d \r", fuck++);
    return;
  }
#endif

  dx = fl->b.x - fl->a.x;
  ax = 2 * (dx<0 ? -dx : dx);
  sx = dx<0 ? -1 : 1;

  dy = fl->b.y - fl->a.y;
  ay = 2 * (dy<0 ? -dy : dy);
  sy = dy<0 ? -1 : 1;

  x = fl->a.x;
  y = fl->a.y;

  if (ax > ay)
  {
    d = ay - ax/2;
    while (1)
    {
      PUTDOT(x,y,color);
      if (x == fl->b.x) return;
      if (d>=0)
      {
        y += sy;
        d -= ax;
      }
      x += sx;
      d += ay;
    }
  }
  else
  {
    d = ax - ay/2;
    while (1)
    {
      PUTDOT(x, y, color);
      if (y == fl->b.y) return;
      if (d >= 0)
      {
        x += sx;
        d -= ay;
      }
      y += sy;
      d += ax;
    }
  }
}

static void V_DrawVerticalSpan8(int scrn, int x, int y0, int y1, byte color)
{
  byte *p;
  int pitch, y;

  // screen clamp
  if (x < 0 || x >= screens[scrn].width) return;
  if (y0 < 0) y0 = 0;
  if (y1 >= screens[scrn].height) y1 = screens[scrn].height - 1;
  if (y0 > y1) return;

  p = screens[scrn].data + x + screens[scrn].pitch * y0;
  pitch = screens[scrn].pitch;

  for (y = y0; y <= y1; ++y)
  {
    *p = color;
    p += pitch;
  }
}

static void V_DrawHorizontalSpan8(int scrn, int y, int x0, int x1, byte color)
{
  byte *row;

  // screen clamp
  if (y < 0 || y >= screens[scrn].height) return;
  if (x0 < 0) x0 = 0;
  if (x1 >= screens[scrn].width) x1 = screens[scrn].width - 1;
  if (x0 > x1) return;

  row = screens[scrn].data + screens[scrn].pitch * y;
  memset(row + x0, color, (size_t)(x1 - x0 + 1));
}

static void WRAP_V_DrawLine_Thick(fline_t *fl, int color, int thickness)
{
  int x0, x1, y0, y1;
  int dx, dy;
  int sx, sy;
  int xlen, ylen;
  int half;
  byte col;

  // Calculate line lengths
  xlen = fl->b.x - fl->a.x;
  if (xlen < 0) xlen = -xlen;
  ylen = fl->b.y - fl->a.y;
  if (ylen < 0) ylen = -ylen;

  // Draw circle if line is too short, then exit
  if (xlen <= 2 && ylen <= 2)
  {
    // middle of circle coordinates
    int mx = (fl->a.x + fl->b.x) / 2;
    int my = (fl->a.y + fl->b.y) / 2;

    PUTDOT(mx, my, color);
    return;
  }

  // Line points
  x0 = fl->a.x;
  y0 = fl->a.y;
  x1 = fl->b.x;
  y1 = fl->b.y;

  // Set up line info
  dx = x1 - x0;
  sx = (dx < 0) ? -1 : 1;
  dx = (dx < 0) ? -dx : dx;
  dy = y1 - y0;
  sy = (dy < 0) ? -1 : 1;
  dy = (dy < 0) ? -dy : dy;

  // line properties
  half = thickness / 2;
  col = (byte)color;

  // Draw spans instead of lines
  if (dx >= dy)
  {
    int erroracc = dx / 2;
    int erroradj = dy;

    for (;;)
    {
      V_DrawVerticalSpan8(0, x0, y0 - half, y0 + (thickness - half - 1), col);

      if (x0 == x1) break;

      erroracc -= erroradj;
      if (erroracc < 0)
      {
        y0 += sy;
        erroracc += dx;
      }
      x0 += sx;
    }
  }
  else
  {
    int erroracc = dy / 2;
    int erroradj = dx;

    for (;;)
    {
      V_DrawHorizontalSpan8(0, y0, x0 - half, x0 + (thickness - half - 1), col);

      if (y0 == y1) break;

      erroracc -= erroradj;
      if (erroracc < 0)
      {
        x0 += sx;
        erroracc += dy;
      }
      y0 += sy;
    }
  }

  // Draw points on the ends (optimisation: skip on small lines)
  if (xlen + ylen >= 3)
  {
    PUTDOT(fl->a.x, fl->a.y, color);
    PUTDOT(fl->b.x, fl->b.y, color);
  }
}

// Change rendering path based on thickness
void WRAP_V_DrawLine(fline_t *fl, int color)
{
  int thickness = AM_GetLineWeight();

  if (thickness > 1)
    WRAP_V_DrawLine_Thick(fl, color, thickness);
  else
    WRAP_V_DrawLine_1px(fl, color);
}

extern SDL_Surface *screen;
#define RGB2COLOR(r, g, b)\
  ((r >> screen->format->Rloss) << screen->format->Rshift) |\
  ((g >> screen->format->Gloss) << screen->format->Gshift) |\
  ((b >> screen->format->Bloss) << screen->format->Bshift)\

// Given 65536, we need 2048; 65536 / 2048 == 32 == 2^5
// Why 2048? ANG90 == 0x40000000 which >> 19 == 0x800 == 2048.
// The trigonometric correction is based on an angle from 0 to 90.
#define wu_fineshift 5

#define wu_weightbits 6

// Given 64 levels in the Col2RGB8 table, 65536 / 64 == 1024 == 2^10
#define wu_fixedshift (16-wu_weightbits)

//
// V_PlotPixelWu
//
// haleyjd 06/13/09: Pixel plotter for Wu line drawing.
//
static void V_PlotPixelWu8_1px(int scrn, int x, int y, byte color, int weight)
{
  unsigned int bg_color = screens[scrn].data[x+screens[scrn].pitch*y];
  unsigned int *fg2rgb = Col2RGB8[weight];
  unsigned int *bg2rgb = Col2RGB8[64 - weight];
  unsigned int fg = fg2rgb[color];
  unsigned int bg = bg2rgb[bg_color];

  fg = (fg + bg) | 0x1f07c1f;
  V_PlotPixel(scrn, x, y, RGB32k[0][0][fg & (fg >> 15)]);
}

// Change rendering path based on thickness
static void V_PlotPixelWu8(int scrn, int x, int y, byte color, int weight)
{
  int thickness = AM_GetLineWeight();

  if (thickness > 1)
    V_PlotCircle8(scrn, x, y, thickness, color);
  else
    V_PlotPixelWu8_1px(scrn, x, y, color, weight);
}

//
// WRAP_V_DrawLineWu
//
// haleyjd 06/12/09: Wu line drawing for the automap, with trigonometric
// brightness correction by SoM. I call this the Wu-McGranahan line drawing
// algorithm.
//
void WRAP_V_DrawLineWu_1px(fline_t *fl, int color)
{
  unsigned short erroracc, erroradj, erroracctmp;
  int dx, dy, xdir = 1;
  int x, y;

  // swap end points if necessary
  if(fl->a.y > fl->b.y)
  {
    fpoint_t tmp = fl->a;

    fl->a = fl->b;
    fl->b = tmp;
  }

  // determine change in x, y and direction of travel
  dx = fl->b.x - fl->a.x;
  dy = fl->b.y - fl->a.y;

  if(dx < 0)
  {
    dx   = -dx;
    xdir = -xdir;
  }

  // detect special cases -- horizontal, vertical, and 45 degrees;
  // revert to Bresenham
  if(dx == 0 || dy == 0 || dx == dy)
  {
    V_DrawLine(fl, color);
    return;
  }

  // draw first pixel
  PUTDOT(fl->a.x, fl->a.y, color);

  x = fl->a.x;
  y = fl->a.y;

  if(dy > dx)
  {
    // line is y-axis major.
    erroracc = 0;
    erroradj = (unsigned short)(((unsigned int)dx << 16) / (unsigned int)dy);

    while(--dy)
    {
      erroracctmp = erroracc;

      erroracc += erroradj;

      // if error has overflown, advance x coordinate
      if(erroracc <= erroracctmp)
        x += xdir;

      y += 1; // advance y

      // the trick is in the trig!
      V_PlotPixelWu(0, x, y, (byte)color,
        finecosine[erroracc >> wu_fineshift] >> wu_fixedshift);
      V_PlotPixelWu(0, x + xdir, y, (byte)color,
        finesine[erroracc >> wu_fineshift] >> wu_fixedshift);
    }
  }
  else
  {
    // line is x-axis major.
    erroracc = 0;
    erroradj = (unsigned short)(((unsigned int)dy << 16) / (unsigned int)dx);

    while(--dx)
    {
      erroracctmp = erroracc;

      erroracc += erroradj;

      // if error has overflown, advance y coordinate
      if(erroracc <= erroracctmp)
        y += 1;

      x += xdir; // advance x

      // the trick is in the trig!
      V_PlotPixelWu(0, x, y, (byte)color,
        finecosine[erroracc >> wu_fineshift] >> wu_fixedshift);
      V_PlotPixelWu(0, x, y + 1, (byte)color,
        finesine[erroracc >> wu_fineshift] >> wu_fixedshift);
    }
  }

  // draw last pixel
  PUTDOT(fl->b.x, fl->b.y, color);
}

// Change rendering path based on thickness
void WRAP_V_DrawLineWu(fline_t *fl, int color)
{
  int thickness = AM_GetLineWeight();

  if (thickness > 1)
    WRAP_V_DrawLine_Thick(fl, color, thickness);
  else
    WRAP_V_DrawLineWu_1px(fl, color);
}

const unsigned char* V_GetPlaypal(void)
{
  dsda_playpal_t* playpal_data;

  playpal_data = dsda_PlayPalData();

  if (!playpal_data->lump)
  {
    int lump = W_GetNumForName(playpal_data->lump_name);
    const byte *data = W_LumpByNum(lump);
    playpal_data->length = W_LumpLength(lump);
    playpal_data->lump = Z_Malloc(playpal_data->length);
    memcpy(playpal_data->lump, data, playpal_data->length);
  }

  return playpal_data->lump;
}

void V_FreePlaypal(void)
{
  dsda_FreePlayPal();
}

int V_GetPlaypalCount(void)
{
  V_GetPlaypal(); // ensure playpal data is initialized
  return (dsda_PlayPalData()->length / PALETTE_SIZE);
}

//
// V_GetPatchColor
// Get the color of patch /
// RAW graphic via lumpnum
//

SDL_Color V_GetPatchColor (int lumpnum)
{
  SDL_Color col = { 0, 0, 0 };
  int r = 0, g = 0, b = 0;
  const unsigned char *playpal = V_GetPlaypal();
  int x, y, pixel_cnt = 0;
  const byte* lump;
  short width;

  lump = W_LumpByNum(lumpnum);

  // Doom Patch format
  if (R_IsPatchLump(lumpnum))
  {
    width = *((const int16_t *) lump);
    width = LittleShort(width);

    for (x = 0; x < width; ++x) {
      byte length;
      byte entry;
      const byte* p;
      int32_t offset;

      // Only calculate for the leftmost and rightmost 16 columns
      if (width > 32 && x > 16 && x < width - 16)
        continue;

      // Skip irrelevant data in the doom patch header
      p = lump + 8 + 4 * x;
      offset = *((const int32_t *) p);
      p = lump + LittleLong(offset);

      while (*p != 0xff) {
        p++;
        length = *p++;
        p++;

        // Get RGB values per pixel
        for (y = 0; y < length; ++y) {
          entry = *p++;
          r += playpal[3 * entry + 0];
          g += playpal[3 * entry + 1];
          b += playpal[3 * entry + 2];
          pixel_cnt++;
        }

        p++;
      }
    }
  }
  // RAW Screen format (Heretic / Hexen)
  else
  {
    width = W_LumpLength(lumpnum) / 200;

    for (x = 0; x < width; ++x) {
      // Only calculate for the leftmost and rightmost 16 columns
      if (width > 32 && x > 16 && x < width - 16)
        continue;

      // Get RGB values per pixel
      // Only need to check 200 height
      for (y = 0; y < 200; ++y) {
        byte entry = lump[y * width + x];
        r += playpal[3 * entry + 0];
        g += playpal[3 * entry + 1];
        b += playpal[3 * entry + 2];
        pixel_cnt++;
      }
    }
  }

  // Avoid transparent pixel crash
  if (pixel_cnt < 1)
    pixel_cnt = 1;

  // Average RGB values
  col.r = r / pixel_cnt;
  col.g = g / pixel_cnt;
  col.b = b / pixel_cnt;

  return col;
}

static byte V_GetBorderColor(const char* lump)
{
  int lumpnum = W_GetNumForName(lump);
  static int prevlump = -1;
  static byte col;

  if (animateLumps)
    lumpnum = N_GetPatchAnimateNum(lump, true);

  if (prevlump != lumpnum)
  {
    const unsigned char *playpal = V_GetPlaypal();
    SDL_Color patch_color = V_GetPatchColor(lumpnum);
    int r = patch_color.r;
    int g = patch_color.g;
    int b = patch_color.b;

    // Desaturate colours
    r /= 2;
    g /= 2;
    b /= 2;

    // Convert to palette
    col = V_BestColor(playpal, r, g, b);
    prevlump = lumpnum;
  }

  return col;
}

static void V_DrawBorder(byte pillarboxcolor, int screenfill)
{
  int bordtop, bordbottom, bordleft, bordright;

  bordleft = wide_offsetx;
  bordright = wide_offset2x - wide_offsetx;
  bordtop = wide_offsety;
  bordbottom = wide_offset2y - wide_offsety;

  if (screenfill)
  {
    // Draw color under image - fixes eternall.wad's partly transparent CREDIT
    V_FillRect(0, 0, SCREENWIDTH, SCREENHEIGHT, pillarboxcolor);
    return;
  }

  if (bordtop > 0)
  {
    // Top
    V_FillRect(0, 0, SCREENWIDTH, bordtop, pillarboxcolor);
    // Bottom
    V_FillRect(0, SCREENHEIGHT - bordbottom, SCREENWIDTH, bordbottom, pillarboxcolor);
  }

  if (bordleft > 0)
  {
    // Left
    V_FillRect(0, bordtop, bordleft, SCREENHEIGHT - bordbottom - bordtop, pillarboxcolor);
    // Right
    V_FillRect(SCREENWIDTH - bordright, bordtop, bordright, SCREENHEIGHT - bordbottom - bordtop, pillarboxcolor);
  }
}

void V_ClearBorderbox(const char* lump, int screenfill)
{
  dboolean ColorBorder = dsda_IntConfig(dsda_config_colored_borderbox) && (lump != NULL);

  if (render_stretch_hud == patch_stretch_fit_to_width)
    return;

  V_DrawBorder(ColorBorder ? V_GetBorderColor(lump) : 0, screenfill);
}

// DWF 2012-05-10
// Euclid's algorithm to find the greatest common divisor.
static unsigned int gcd (unsigned int n, unsigned int d) { return (d ? gcd(d, n%d) : n); }

// DWF 2012-05-10
// Reduce aspect ratio fractions to make the log messages nicer.  Even if
// integer math were still being used for FPS scaling, this would not
// necessarily speed it up, but it does no harm.
// Order of parameters (numerator, denominator) doesn't matter.
static void ReduceFraction (unsigned *num1, unsigned *num2)
{
  unsigned int g;
  assert(*num1 || *num2);
  g = gcd (*num1, *num2);
  *num1 /= g;
  *num2 /= g;
}

// DWF 2012-05-01
// C substitute for C++ std::swap.
static void swap(unsigned int *num1, unsigned int *num2)
{
  unsigned int temp = *num1;
  *num1 = *num2;
  *num2 = temp;
}

// DWF 2012-05-01
// Set global variables for video scaling.
void SetRatio(int width, int height)
{
  lprintf(LO_DEBUG, "SetRatio: width/height parameters %dx%d\n", width, height);

  ratio_multiplier = width;
  ratio_scale = height;
  ReduceFraction(&ratio_multiplier, &ratio_scale);

  // The terms storage aspect ratio, pixel aspect ratio, and display aspect
  // ratio came from Wikipedia.  SAR x PAR = DAR
  lprintf(LO_DEBUG, "SetRatio: storage aspect ratio %u:%u\n", ratio_multiplier, ratio_scale);
  if (height == 200 || height == 400 || !dsda_IntConfig(dsda_config_aspect_ratio_correction))
  {
    lprintf(LO_DEBUG, "SetRatio: recognized VGA mode with pixel aspect ratio 5:6\n");
    ratio_multiplier = width * 5;
    ratio_scale = height * 6;
    ReduceFraction(&ratio_multiplier, &ratio_scale);
  }
  else
  {
    lprintf(LO_DEBUG, "SetRatio: assuming square pixels\n");
  }
  lprintf(LO_DEBUG, "SetRatio: display aspect ratio %u:%u\n", ratio_multiplier, ratio_scale);

  // If user wants to force aspect ratio, let them.
  {
    unsigned int new_multiplier = ratio_multiplier;
    unsigned int new_scale = ratio_scale;
    int render_aspect = dsda_IntConfig(dsda_config_render_aspect);

    // Hardcoded to match render_aspects_list
    switch (render_aspect)
    {
      case 0:
        break;
      case 1:
        new_multiplier = 16;
        new_scale = 9;
        break;
      case 2:
        new_multiplier = 16;
        new_scale = 10;
        break;
      case 3:
        new_multiplier = 4;
        new_scale = 3;
        break;
      case 4:
        new_multiplier = 5;
        new_scale = 4;
        break;
      default:
        lprintf(LO_ERROR, "SetRatio: render_aspect has invalid value %d\n", render_aspect);
    }

    if (ratio_multiplier != new_multiplier || ratio_scale != new_scale)
    {
      lprintf(LO_DEBUG, "SetRatio: overruled by user configuration setting\n");
      ratio_multiplier = new_multiplier;
      ratio_scale = new_scale;
      lprintf(LO_DEBUG, "SetRatio: revised display aspect ratio %u:%u\n", ratio_multiplier, ratio_scale);
    }
  }

  gl_ratio = RMUL * ratio_multiplier / ratio_scale;
  lprintf(LO_DEBUG, "SetRatio: gl_ratio %f\n", gl_ratio);

  // Calculate modified multiplier following the pattern of the old
  // BaseRatioSizes table in PrBoom-Plus 2.5.1.3.
  swap(&ratio_multiplier, &ratio_scale);
  ratio_multiplier *= 4;
  ratio_scale *= 3;
  ReduceFraction(&ratio_multiplier, &ratio_scale);

  tallscreen = (ratio_scale < ratio_multiplier);
  if (tallscreen)
  {
    float ratio_quotient = (float)ratio_multiplier/ratio_scale;
    float ratio_percentage = (ratio_quotient - 1) * 100.0;
    psprite_offset = (int)(ratio_percentage*FRACUNIT);

    lprintf(LO_DEBUG, "SetRatio: tallscreen aspect recognized; flipping multiplier\n");
    swap(&ratio_multiplier, &ratio_scale);
  }
  else
  {
    psprite_offset = 0;
  }
  lprintf(LO_DEBUG, "SetRatio: multiplier %u/%u\n", ratio_multiplier, ratio_scale);

  // The rest is carried over from CheckRatio in PrBoom-Plus 2.5.1.3.
  if (tallscreen)
  {
    WIDE_SCREENWIDTH = SCREENWIDTH;
    WIDE_SCREENHEIGHT = SCREENHEIGHT * ratio_multiplier / ratio_scale;
  }
  else
  {
    WIDE_SCREENWIDTH = SCREENWIDTH * ratio_multiplier / ratio_scale;
    WIDE_SCREENHEIGHT = SCREENHEIGHT;
  }

  WIDE_SCREENWIDTH = MAX(1, WIDE_SCREENWIDTH);
  WIDE_SCREENHEIGHT = MAX(1, WIDE_SCREENHEIGHT);

  yaspectmul = Scale((320<<FRACBITS), WIDE_SCREENHEIGHT, 200 * WIDE_SCREENWIDTH);

  dsda_EvaluatePatchScale();

  SCREEN_320x200 =
    (SCREENWIDTH == 320) && (SCREENHEIGHT == 200) &&
    (WIDE_SCREENWIDTH == 320) && (WIDE_SCREENHEIGHT == 200);
}

void V_GetWideRect(int *x, int *y, int *w, int *h, enum patch_translation_e flags)
{
  stretch_param_t *params = dsda_StretchParams(flags);
  int sx = *x;
  int sy = *y;

  *x = params->video->x1lookup[*x];
  *y = params->video->y1lookup[*y];
  *w = params->video->x2lookup[sx + *w - 1] - *x + 1;
  *h = params->video->y2lookup[sy + *h - 1] - *y + 1;
  *x += params->deltax1;
  *y += params->deltay1;
}

//
// V_BestColor
//
// Adapted from zdoom -- thanks to Randy Heit.
//
// This always assumes a 256-color palette;
// it's intended for use in startup functions to match hard-coded
// color values to the best fit in the game's palette (allows
// cross-game usage among other things).
//
int V_BestColor(const unsigned char *palette, int r, int g, int b)
{
  int color;

  // use color 0 as a worst-case match for any color
  int bestcolor = 0;
  int bestdist = 257 * 257 + 257 * 257 + 257 * 257;

  for (color = 0; color < 256; color++)
  {
    int dr, dg, db, dist;

    dr = r - *palette++;
    dg = g - *palette++;
    db = b - *palette++;

    dist = dr * dr + dg * dg + db * db;

    if (dist < bestdist)
    {
      // exact match
      if (dist == 0)
        return color;

      bestdist = dist;
      bestcolor = color;
    }
  }

  return bestcolor;
}

// Alt-Enter: fullscreen <-> windowed
void V_ToggleFullscreen(void)
{
  dsda_UpdateIntConfig(dsda_config_use_fullscreen, !desired_fullscreen, true);
}

void V_ChangeScreenResolution(void)
{
  I_UpdateVideoMode();

  if (V_IsOpenGLMode())
  {
    gld_PreprocessLevel();
  }
}

void V_FillRectVPT(int x, int y, int width, int height, byte color, enum patch_translation_e flags)
{
  V_GetWideRect(&x, &y, &width, &height, flags);
  V_FillRect(x, y, width, height, color);
}

int V_FillHeightVPT(int y, int height, byte color, enum patch_translation_e flags)
{
  stretch_param_t *params = dsda_StretchParams(flags);
  int sy = y;

  y = params->video->y1lookup[y];
  height = params->video->y2lookup[sy + height - 1] - y + 1;
  y += params->deltay1;
  V_FillRect(0, y, SCREENWIDTH, height, color);

  return height;
}

//
//
// Heretic / Hexen
//
//

void V_DrawRawScreen(const char *lump_name)
{
  // e6y: wide-res
  V_ClearBorder(lump_name);

  V_DrawRawScreenOffset(lump_name, 0, 0, VPT_STRETCH);
}

void V_DrawRawScreenOffset(const char *lump_name, float x_offset, float y_offset, enum patch_translation_e flags)
{
  int lump_num = W_CheckNumForName(lump_name);
  int lumpheight = 200;
  int lumpwidth = W_LumpLength(lump_num) / lumpheight;

  if (!R_IsPatchLump(lump_num))
  {
    stretch_param_t* stretch;
    float ratio_x, ratio_y;
    int width = lumpwidth;
    int height = lumpheight;

    if (flags & VPT_STRETCH_MASK)
    {
      // ratio Correction
      stretch = dsda_StretchParams(VPT_STRETCH);
      ratio_x = stretch->video->width / 320.f;
      ratio_y = stretch->video->height / 200.f;

      // scale variables based on ratio factors
      width = (int)(lumpwidth * ratio_x);
      height = (int)(lumpheight * ratio_y);

      x_offset = (float)((SCREENWIDTH - (lumpwidth * ratio_x)) / 2);
      y_offset = (float)((y_offset * ratio_y));
    }

    // draw the RAW screen
    V_FillNumRawPrecise(lump_num, x_offset, y_offset, lumpwidth, lumpheight, width, height, VPT_STRETCH);
  }
  else
  {
    // custom widescreen assets are a different format
    V_DrawNamePatchPreciseFS(x_offset, y_offset, lump_name, CR_DEFAULT, VPT_STRETCH);
  }
}

void V_DrawTLNumPatch(int x, int y, int lump)
{
  V_DrawNumPatch(x, y, lump, CR_DEFAULT, VPT_STRETCH | VPT_TRANSMAP);
}

void V_DrawTLNamePatch(int x, int y, const char* name)
{
  V_DrawNamePatch(x, y, name, CR_DEFAULT, VPT_STRETCH | VPT_TRANSMAP);
}

void V_DrawReverseTLNumPatch(int x, int y, int lump)
{
  V_DrawNumPatch(x, y, lump, CR_DEFAULT, VPT_STRETCH | VPT_ALT_TRANSMAP);
}

void V_DrawReverseTLNamePatch(int x, int y, const char* lump)
{
  V_DrawNamePatch(x, y, lump, CR_DEFAULT, VPT_STRETCH | VPT_ALT_TRANSMAP);
}
