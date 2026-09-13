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
 *      The actual span/column drawing functions.
 *      Here find the main potential for optimization,
 *       e.g. inline assembly, different algorithms.
 *
 *-----------------------------------------------------------------------------*/

#include <stdint.h>

#include "doomstat.h"
#include "w_wad.h"
#include "r_main.h"
#include "r_draw.h"
#include "v_video.h"
#include "st_stuff.h"
#include "g_game.h"
#include "am_map.h"
#include "lprintf.h"
#include "i_system.h"

#include "dsda/animinfo.h"
#include "dsda/palette.h"
#include "dsda/settings.h"
#include "dsda/stretch.h"

//
// All drawing to the view buffer is accomplished in this file.
// The other refresh files only know about ccordinates,
//  not the architecture of the frame buffer.
// Conveniently, the frame buffer is a linear one,
//  and we need only the base address,
//  and the total size == width*height*depth/8.,
//

byte *viewimage;
int  viewwidth;
int  scaledviewwidth;
int  viewheight;
int  viewwindowx;
int  viewwindowy;

// Color tables for different players,
//  translate a limited part to another
//  (color ramps used for  suit colors).
//

// CPhipps - made const*'s
const byte *tranmap;          // translucency filter maps 256x256   // phares
const byte *main_tranmap;     // killough 4/11/98

//
// R_DrawColumn
// Source is the top of the column to scale.
//

//
// Spectre/Invisibility.
//

#define FUZZTABLE 50
// proff 08/17/98: Changed for high-res
//#define FUZZOFF (SCREENWIDTH)
#define FUZZOFF 1

static const int fuzzoffset_org[FUZZTABLE] = {
  FUZZOFF,-FUZZOFF,FUZZOFF,-FUZZOFF,FUZZOFF,FUZZOFF,-FUZZOFF,
  FUZZOFF,FUZZOFF,-FUZZOFF,FUZZOFF,FUZZOFF,FUZZOFF,-FUZZOFF,
  FUZZOFF,FUZZOFF,FUZZOFF,-FUZZOFF,-FUZZOFF,-FUZZOFF,-FUZZOFF,
  FUZZOFF,-FUZZOFF,-FUZZOFF,FUZZOFF,FUZZOFF,FUZZOFF,FUZZOFF,-FUZZOFF,
  FUZZOFF,-FUZZOFF,FUZZOFF,FUZZOFF,-FUZZOFF,-FUZZOFF,FUZZOFF,
  FUZZOFF,-FUZZOFF,-FUZZOFF,-FUZZOFF,-FUZZOFF,FUZZOFF,FUZZOFF,
  FUZZOFF,FUZZOFF,-FUZZOFF,FUZZOFF,FUZZOFF,-FUZZOFF,FUZZOFF
};


#define FUZZDARK (6 * 256)
#define FUZZPAL  256

static const int fuzzdark[FUZZTABLE] =
{
    4 * FUZZPAL, 0, 6 * FUZZPAL, 0, 6 * FUZZPAL, 6 * FUZZPAL, 0,
    6 * FUZZPAL, 6 * FUZZPAL, 0, 6 * FUZZPAL, 6 * FUZZPAL, 6 * FUZZPAL, 0,
    6 * FUZZPAL, 8 * FUZZPAL, 6 * FUZZPAL, 0, 0, 0, 0,
    4 * FUZZPAL, 0, 0, 6 * FUZZPAL, 6 * FUZZPAL, 6 * FUZZPAL, 6 * FUZZPAL, 0,
    4 * FUZZPAL, 0, 6 * FUZZPAL, 6 * FUZZPAL, 0, 0, 6 * FUZZPAL,
    6 * FUZZPAL, 0, 0, 0, 0, 6 * FUZZPAL, 6 * FUZZPAL,
    6 * FUZZPAL, 6 * FUZZPAL, 0, 6 * FUZZPAL, 6 * FUZZPAL, 0, 6 * FUZZPAL,
};

static int fuzzoffset[FUZZTABLE];

static int fuzzpos = 0;
static int scaledfuzzpos = 0;

// Fuzz cell size for scaled software fuzz
int fuzzcellsize;
int min_fuzzcellsize;
int scaled_fuzzcellsize;
int fuzz_cutoff = false;

// Scaled software fuzz algorithm
static void R_DrawFuzzColumn(draw_column_vars_t *dcvars, int cellsize, int *fuzzpos)
{
  const dboolean blocky_fuzz      = dsda_IntConfig(dsda_config_fuzzmode) == 0 || dsda_StrictMode();
  const dboolean refraction_fuzz  = dsda_IntConfig(dsda_config_fuzzmode) == 1 && !dsda_StrictMode();
  const dboolean shadow_fuzz      = dsda_IntConfig(dsda_config_fuzzmode) == 2 && !dsda_StrictMode();
  int count = dcvars->yh - dcvars->yl + 1;
  byte *dest;

  if (count <= 0)
  {
    return;
  }

  if (shadow_fuzz)
  {
    dest = drawvars.topleft + dcvars->yl + dcvars->x * drawvars.pitch;

    while (count--)
    {
      *dest = fullcolormap[8 * 256 + *dest];
      dest++;
    }
    return;
  }

  if (blocky_fuzz || refraction_fuzz)
  {
    const int fuzz_end = dcvars->x + 1;
    int fuzz_fill;
    int lines;
    int dark = FUZZDARK;
    int offset = 0;

    // Partly fill fuzz block to right edge
    if (fuzz_end % cellsize)
    {
      if (fuzz_end != SCREENWIDTH)
        return;

      fuzz_fill = fuzz_end % cellsize;
    }
    // Fuzz fill normally
    else
    {
      fuzz_fill = cellsize;
    }

    dest = drawvars.topleft + dcvars->yl + (fuzz_end - fuzz_fill) * drawvars.pitch;
    lines = cellsize - (dcvars->yl % cellsize);

    do
    {
      int mask;
      const byte fuzz = refraction_fuzz ?
        fullcolormap[dark + dest[offset]] :
        fullcolormap[6 * 256 + dest[fuzzoffset[*fuzzpos]]];

      count -= lines;

      // if (count < 0)
      // {
      //    lines += count;
      //    count = 0;
      // }
      mask = count >> (8 * sizeof(mask) - 1);
      lines += count & mask;
      count &= ~mask;

      do
      {
        int i;
        byte *rowdest = dest;

        for (i = 0; i < fuzz_fill; ++i)
          rowdest[i * drawvars.pitch] = fuzz;

        ++dest;
      } while (--lines);

      ++*fuzzpos;

      // Clamp table lookup index.
      *fuzzpos &= (*fuzzpos - FUZZTABLE) >> (8 * sizeof(*fuzzpos) - 1);

      if (refraction_fuzz)
      {
        dark = fuzzdark[*fuzzpos];
        offset = fuzzoffset[*fuzzpos];
      }

      lines = cellsize;
    } while (count);

    // [crispy] if the line at the bottom had to be cut off,
    // draw one extra line using only pixels of that line and the one above
    if (fuzz_cutoff)
    {
      int i;
      const byte fuzz = refraction_fuzz ?
        fullcolormap[dark + dest[-1]] :
        fullcolormap[6 * 256 + dest[-1]];

      for (i = 0; i < fuzz_fill; ++i)
        dest[i * drawvars.pitch] = fuzz;
    }
  }
}

// render pipelines
#define RDC_STANDARD          1
#define RDC_TRANSLUCENT       2 // translucent
#define RDC_TRANSLATED        4 // color
#define RDC_TRTL              8 // Translucent + color
#define RDC_ALT_TL           16 // alt-translucent
#define RDC_ALT_TRTL         32 // alt-translucent + color
#define RDC_DOUBLESKY        64 // hexen double sky
#define RDC_SKY_COLOR_CAP   128 // [Woof] sky color cap
#define RDC_FUZZ            256
#define RDC_FUZZ_SCALED     512
// no color mapping
#define RDC_NOCOLMAP       1024

draw_vars_t drawvars = {
  NULL, // topleft
  0, // pitch
};

dboolean R_FullView(void)
{
  return viewheight == SCREENHEIGHT;
}

dboolean R_PartialView(void)
{
  return viewheight != SCREENHEIGHT;
}

dboolean R_StatusBarVisible(void)
{
  return R_PartialView() || automap_solid || dsda_FullAutomapHud();
}


//
// R_DrawColumn
//

//
// A column is a vertical slice/span from a wall texture that,
//  given the DOOM style restrictions on the view orientation,
//  will always have constant z depth.
// Thus a special case loop for very fast rendering can
//  be used. It has also been used with Wolfenstein 3D.
//
// [AR] Direct transposed drawing replaces R_FlushColumns and R_ResetColumnBuffer.
//

byte *translationtables;

#define R_DRAWCOLUMN_PIPELINE_TYPE RDC_PIPELINE_STANDARD
#define R_DRAWCOLUMN_PIPELINE_BASE RDC_STANDARD

#define R_DRAWCOLUMN_FUNCNAME_COMPOSITE(postfix) R_DrawColumn ## postfix
#include "r_drawcolpipeline.inl"

#undef R_DRAWCOLUMN_PIPELINE_BASE
#undef R_DRAWCOLUMN_PIPELINE_TYPE

//
// R_DrawDoubleSkyColumn
//
// Doesn't draw black pixels on source2
// Draws source underneath
//

#define R_DRAWCOLUMN_PIPELINE_TYPE RDC_PIPELINE_DOUBLESKY
#define R_DRAWCOLUMN_PIPELINE_BASE RDC_DOUBLESKY

#define R_DRAWCOLUMN_FUNCNAME_COMPOSITE(postfix) R_DrawDoubleSkyColumn ## postfix
#include "r_drawcolpipeline.inl"

#undef R_DRAWCOLUMN_PIPELINE_BASE
#undef R_DRAWCOLUMN_PIPELINE_TYPE

// Here is the version of R_DrawColumn that deals with translucent  // phares
// textures and sprites. It's identical to R_DrawColumn except      //    |
// for the spot where the color index is stuffed into *dest. At     //    V
// that point, the existing color index and the new color index
// are mapped through the TRANMAP lump filters to get a new color
// index whose RGB values are the average of the existing and new
// colors.
//
// Since we're concerned about performance, the 'translucent or
// opaque' decision is made outside this routine, not down where the
// actual code differences are.

#define R_DRAWCOLUMN_PIPELINE_TYPE RDC_PIPELINE_TRANSLUCENT
#define R_DRAWCOLUMN_PIPELINE_BASE RDC_TRANSLUCENT

#define R_DRAWCOLUMN_FUNCNAME_COMPOSITE(postfix) R_DrawTLColumn ## postfix
#include "r_drawcolpipeline.inl"

#undef R_DRAWCOLUMN_PIPELINE_BASE
#undef R_DRAWCOLUMN_PIPELINE_TYPE

//
// R_DrawTranslatedColumn
// Used to draw player sprites
//  with the green colorramp mapped to others.
// Could be used with different translation
//  tables, e.g. the lighter colored version
//  of the BaronOfHell, the HellKnight, uses
//  identical sprites, kinda brightened up.
//

#define R_DRAWCOLUMN_PIPELINE_TYPE RDC_PIPELINE_TRANSLATED
#define R_DRAWCOLUMN_PIPELINE_BASE RDC_TRANSLATED

#define R_DRAWCOLUMN_FUNCNAME_COMPOSITE(postfix) R_DrawTranslatedColumn ## postfix
#include "r_drawcolpipeline.inl"

#undef R_DRAWCOLUMN_PIPELINE_BASE
#undef R_DRAWCOLUMN_PIPELINE_TYPE

//
// R_DrawAltTLColumn
// Used to draw sprites with reverse translucency!
//

#define R_DRAWCOLUMN_PIPELINE_TYPE RDC_PIPELINE_ALT_TL
#define R_DRAWCOLUMN_PIPELINE_BASE RDC_ALT_TL

#define R_DRAWCOLUMN_FUNCNAME_COMPOSITE(postfix) R_DrawAltTLColumn ## postfix
#include "r_drawcolpipeline.inl"

#undef R_DRAWCOLUMN_PIPELINE_BASE
#undef R_DRAWCOLUMN_PIPELINE_TYPE

//
// R_DrawAltTRTLColumn
// Used to draw sprites with specific colours
//  with reverse translucency!
//

#define R_DRAWCOLUMN_PIPELINE_TYPE RDC_PIPELINE_ALT_TRTL
#define R_DRAWCOLUMN_PIPELINE_BASE RDC_ALT_TRTL

#define R_DRAWCOLUMN_FUNCNAME_COMPOSITE(postfix) R_DrawAltTRTLColumn ## postfix
#include "r_drawcolpipeline.inl"

#undef R_DRAWCOLUMN_PIPELINE_BASE
#undef R_DRAWCOLUMN_PIPELINE_TYPE

//
// R_DrawTRTLColumn
// Used to draw sprites with specific colours
//  with translucency!
//

#define R_DRAWCOLUMN_PIPELINE_TYPE RDC_PIPELINE_TRTL
#define R_DRAWCOLUMN_PIPELINE_BASE RDC_TRTL

#define R_DRAWCOLUMN_FUNCNAME_COMPOSITE(postfix) R_DrawTRTLColumn ## postfix
#include "r_drawcolpipeline.inl"

#undef R_DRAWCOLUMN_PIPELINE_BASE
#undef R_DRAWCOLUMN_PIPELINE_TYPE

//
// R_DrawSkyColorCapColumn
// Extends the top of a sky texture with a solid color
//

#define R_DRAWCOLUMN_PIPELINE_TYPE RDC_PIPELINE_SKY_COLOR_CAP
#define R_DRAWCOLUMN_PIPELINE_BASE RDC_SKY_COLOR_CAP

#define R_DRAWCOLUMN_FUNCNAME_COMPOSITE(postfix) R_DrawSkyColorCapColumn ## postfix
#include "r_drawcolpipeline.inl"

#undef R_DRAWCOLUMN_PIPELINE_BASE
#undef R_DRAWCOLUMN_PIPELINE_TYPE

//
// Framebuffer postprocessing.
// Creates a fuzzy image by copying pixels
//  from adjacent ones to left and right.
// Used with an all black colormap, this
//  could create the SHADOW effect,
//  i.e. spectres and invisible players.
//

#define R_DRAWCOLUMN_PIPELINE_TYPE RDC_PIPELINE_FUZZ
#define R_DRAWCOLUMN_PIPELINE_BASE RDC_FUZZ

#define R_DRAWCOLUMN_FUNCNAME_COMPOSITE(postfix) R_DrawFuzzColumn ## postfix
#include "r_drawcolpipeline.inl"

#undef R_DRAWCOLUMN_PIPELINE_BASE
#undef R_DRAWCOLUMN_PIPELINE_TYPE

//
// NYAN Distance Scaled Fuzz.
// Same as the fuzz filter above,
// but is separate from weapon sprites.
//

#define R_DRAWCOLUMN_PIPELINE_TYPE RDC_PIPELINE_FUZZ_SCALED
#define R_DRAWCOLUMN_PIPELINE_BASE RDC_FUZZ_SCALED

#define R_DRAWCOLUMN_FUNCNAME_COMPOSITE(postfix) R_DrawFuzzScaledColumn ## postfix
#include "r_drawcolpipeline.inl"

#undef R_DRAWCOLUMN_PIPELINE_BASE
#undef R_DRAWCOLUMN_PIPELINE_TYPE

static R_DrawColumn_f drawcolumnfuncs[RDRAW_FILTER_MAXFILTERS][RDC_PIPELINE_MAXPIPELINES] = {
  {
    R_DrawColumn_PointUV,
    R_DrawTLColumn_PointUV,
    R_DrawTranslatedColumn_PointUV,
    R_DrawTRTLColumn_PointUV,
    R_DrawAltTLColumn_PointUV,
    R_DrawAltTRTLColumn_PointUV,
    R_DrawDoubleSkyColumn_PointUV,
    R_DrawSkyColorCapColumn_PointUV,
    R_DrawFuzzColumn_PointUV,
    R_DrawFuzzScaledColumn_PointUV,
  },
  {
    R_DrawColumn_PointUV_PointZ,
    R_DrawTLColumn_PointUV_PointZ,
    R_DrawTranslatedColumn_PointUV_PointZ,
    R_DrawTRTLColumn_PointUV_PointZ,
    R_DrawAltTLColumn_PointUV_PointZ,
    R_DrawAltTRTLColumn_PointUV_PointZ,
    R_DrawDoubleSkyColumn_PointUV_PointZ,
    R_DrawSkyColorCapColumn_PointUV_PointZ,
    R_DrawFuzzColumn_PointUV_PointZ,
    R_DrawFuzzScaledColumn_PointUV_PointZ,
  },
};

R_DrawColumn_f R_GetDrawColumnFunc(enum column_pipeline_e type, enum draw_filter_type_e filterz) {
  R_DrawColumn_f result = drawcolumnfuncs[filterz][type];
  if (result == NULL)
    I_Error("R_GetDrawColumnFunc: undefined function (%d, %d)", type, filterz);
  return result;
}

void R_SetDefaultDrawColumnVars(draw_column_vars_t *dcvars) {
  dcvars->x = dcvars->yl = dcvars->yh = 0;
  dcvars->iscale = dcvars->texturemid = dcvars->texheight = 0;
  dcvars->source = dcvars->prevsource = dcvars->nextsource = NULL;
  dcvars->colormap = colormaps[0];
  dcvars->translation = NULL;
  dcvars->edgeslope = dcvars->drawingmasked = 0;
  dcvars->crop = (patch_crop_t){0};
  dcvars->flags = 0;
  dcvars->isplayersprite = false;
  dcvars->pspritepostheight = 0;
  dcvars->skycolor = 0;
  dcvars->sky_tranmap = NULL;

  // heretic
  dcvars->baseclip = -1;

  // hexen - doublesky
  dcvars->source2 = NULL;
}

//
// R_InitTranslationTables
// Creates the translation tables to map
//  the green color ramp to gray, brown, red.
// Assumes a given structure of the PLAYPAL.
// Could be read from a lump instead.
//

byte playernumtotrans[MAX_MAXPLAYERS];

// HERETIC_TODO: player colors
const byte player_colors[] = { 0x70, 0x60, 0x40, 0x20 };

void R_InitTranslationTables (void)
{
  int i, j;
#define MAXTRANS 3
  byte transtocolour[MAXTRANS];

  if (hexen)
  {
    int lumpnum = W_GetNumForName("trantbl0");
    translationtables = Z_Malloc(256 * 3 * (g_maxplayers - 1));

    for (i = 0; i < g_maxplayers; i++)
      playernumtotrans[i] = i;

    for (i = 0; i < 3 * (g_maxplayers - 1); i++)
    {
        const byte* transLump = W_LumpByNum(lumpnum + i);
        memcpy(translationtables + i * 256, transLump, 256);
    }

    return;
  }

  // killough 5/2/98:
  // Remove dependency of colormaps aligned on 256-byte boundary

  if (translationtables == NULL) // CPhipps - allow multiple calls
    translationtables = Z_Malloc(256*MAXTRANS);

  for (i=0; i<MAXTRANS; i++) transtocolour[i] = 255;

  for (i = 0; i < g_maxplayers; i++) {
    byte wantcolour = player_colors[i];
    playernumtotrans[i] = 0;
    if (wantcolour != 0x70) // Not green, would like translation
      for (j = 0; j < MAXTRANS; j++)
        if (transtocolour[j] == 255) {
          transtocolour[j] = wantcolour;
          playernumtotrans[i] = j + 1;
          break;
        }
  }

  // translate just the 16 green colors
  for (i=0; i<256; i++)
    if (i >= 0x70 && i<= 0x7f)
    {
      // CPhipps - configurable player colours
      translationtables[i] = colormaps[0][(i&0xf) + transtocolour[0]];
      translationtables[i+256] = colormaps[0][(i&0xf) + transtocolour[1]];
      translationtables[i+512] = colormaps[0][(i&0xf) + transtocolour[2]];
    }
    else  // Keep all other colors as is.
      translationtables[i]=translationtables[i+256]=translationtables[i+512]=i;
}

//
// R_DrawSpan
// With DOOM style restrictions on view orientation,
//  the floors and ceilings consist of horizontal slices
//  or spans with constant z depth.
// However, rotation around the world z axis is possible,
//  thus this mapping, while simpler and faster than
//  perspective correct texture mapping, has to traverse
//  the texture at an angle in all but a few cases.
// In consequence, flats are not stored by column (like walls),
//  and the inner loop has to step in texture space u and v.
//

void R_DrawSpan(draw_span_vars_t *dsvars) {
  unsigned count = dsvars->x2 - dsvars->x1 + 1;
  fixed_t xfrac = dsvars->xfrac;
  fixed_t yfrac = dsvars->yfrac;
  const fixed_t xstep = dsvars->xstep;
  const fixed_t ystep = dsvars->ystep;
  const byte *source = dsvars->source;
  const byte *colormap = dsvars->colormap;
  byte *dest = drawvars.topleft + dsvars->y + dsvars->x1 * drawvars.pitch;

  while (count) {
    const fixed_t xtemp = (xfrac >> 16) & 63;
    const fixed_t ytemp = (yfrac >> 10) & 4032;
    const fixed_t spot = xtemp | ytemp;
    *dest = colormap[source[spot]];

    xfrac += xstep;
    yfrac += ystep;
    dest += drawvars.pitch;
    count--;
  }
}

void R_InitBuffersRes(void)
{
  extern byte *solidcol;

  if (solidcol) Z_Free(solidcol);
  solidcol = Z_Calloc(1, SCREENWIDTH * sizeof(*solidcol));
}

//
// R_InitBuffer
// Creats lookup tables that avoid
//  multiplies and other hazzles
//  for getting the framebuffer address
//  of a pixel to draw.
//

void R_UpdateFuzzSize(void)
{
  int scale = dsda_IntConfig(dsda_config_fuzzscale);

  if (!tallscreen)
    min_fuzzcellsize = (SCREENHEIGHT + 100) / (200 + (scale*100));  // Just set this til it looked good at 2k resolution (lowest 3 pixels at distance)
  else
    min_fuzzcellsize = (SCREENWIDTH + 160) / (320 + (scale*160));  // Just set this til it looked good at 2k resolution (lowest 3 pixels at distance)
}

void R_InitBuffer(int width, int height)
{
  int i=0;
  // Handle resize,
  //  e.g. smaller view windows
  //  with border and/or status bar.

  viewwindowx = (SCREENWIDTH-width) >> 1;

  // Same with base row offset.

  viewwindowy = width == SCREENWIDTH ? 0 : (SCREENHEIGHT - ST_SCALED_HEIGHT - height) >> 1;

  drawvars.topleft = screens[FG].data + viewwindowy + viewwindowx * screens[FG].pitch;
  drawvars.pitch = screens[FG].pitch;

  for (i=0; i<FUZZTABLE; i++)
    fuzzoffset[i] = fuzzoffset_org[i];
  
  if (!tallscreen)
    fuzzcellsize = scaled_fuzzcellsize = (SCREENHEIGHT + 100) / 200;
  else
    fuzzcellsize = scaled_fuzzcellsize = (SCREENWIDTH + 160) / 320;

  R_UpdateFuzzSize();
}

//
// R_FillBackColor
// Fills the statusbar widescreen area
// with a color
//

void R_FillBackColor (void)
{
  extern patchnum_t stbarbg;
  static byte col;
  static byte col_top;
  static int prevlump = -1;
  static int prevpalette = -1;
  const int stbar_top = SCREENHEIGHT - ST_SCALED_HEIGHT;
  stretch_param_t* stretch = dsda_StretchParams(VPT_STRETCH);
  float ratio_y = stretch->video->height / 200.f;
  const int ST_SCALED_BORDER = (int)(brdr_b.height * ratio_y)/2;
  int lump = N_GetPatchAnimateNum(W_LumpName(stbarbg.lumpnum), false);
  int palette = dsda_PlayPalIndex();

  if (prevlump != lump || prevpalette != palette)
  {
    const unsigned char *playpal = V_GetPlaypal();
    SDL_Color stbar_color = V_GetPatchColor(lump);
    int r = stbar_color.r;
    int g = stbar_color.g;
    int b = stbar_color.b;

    // Convert to palette and tune down saturation
    col = V_BestColor(playpal, r/3, g/3, b/3);
    col_top = V_BestColor(playpal, r/2, g/2, b/2);

    // If colors are the same, brighten top
    if (col_top == col)
      col_top = V_BestColor(playpal, r, g, b);

    prevlump = lump;
    prevpalette = palette;
  }

  V_BeginMenuDraw();
  V_FillRectBG(0, stbar_top, SCREENWIDTH, ST_SCALED_BORDER, col_top);
  V_FillRectBG(0, stbar_top + ST_SCALED_BORDER, SCREENWIDTH, ST_SCALED_HEIGHT - ST_SCALED_BORDER, col);
  V_EndMenuDraw();
}

//
// R_DrawStbarBorder
// Draws border on top of stbar
//

static void R_DrawStbarBorder (void)
{
  int stbar_top = SCREENHEIGHT - ST_SCALED_HEIGHT;
  int x = 0;
  int y = stbar_top;
  int w = SCREENWIDTH;
  int h = brdr_b.height;
  enum patch_translation_e flags = VPT_STRETCH;

  // Resize border
  {
    stretch_param_t *params = dsda_StretchParams(flags);
    dboolean stretch = !SCREEN_320x200;

    if (stretch)
    {
      // Apply screen scaling when in stretch mode
      float fx = floorf((x - params->deltax1) * 320.0f / params->video->width);
      float fy = floorf((y - params->deltay1) * 200.0f / params->video->height);
      float fw = ceilf(w * 320.0f / params->video->width) + 1.0f;

      x = (int)fx;
      y = (int)fy;
      w = (int)fw;
    }
    else
    {
      x = (x - params->deltax1) * 320 / params->video->width;
      y = (y - params->deltay1) * 200 / params->video->height;
      w = (w * 320 / params->video->width);
    }
  }

  V_FillNumPatchBG(brdr_b.lumpnum, x, y, w, h, flags); // bottom
}

//
// R_DrawBorder
// Draws borders around viewport
// for lower zoom levels
//

static void R_DrawBorder (int x, int y, int w, int h)
{
  int g = g_border_offset;
  int t = brdr_t.height;
  int b = brdr_b.height;
  int l = brdr_l.width;
  int r = brdr_r.width;
  enum patch_translation_e flags = VPT_STRETCH;

  // Resize border
  {
    stretch_param_t *params = dsda_StretchParams(flags);
    dboolean stretch = !SCREEN_320x200;

    if (stretch)
    {
      // Apply screen scaling when in stretch mode
      float fx = floorf((x - params->deltax1) * 320.0f / params->video->width);
      float fy = floorf((y - params->deltay1) * 200.0f / params->video->height);
      float fw = ceilf(w * 320.0f / params->video->width);
      float fh = ceilf(h * 200.0f / params->video->height);

      x = (int)fx;
      y = (int)fy;
      w = (int)fw;
      h = (int)fh;

      // Avoid gaps between viewport and border
      x += 1;
      y += 1;
      w -= 2;
      h -= 2;
    }
    else
    {
      x = (x - params->deltax1) * 320 / params->video->width;
      y = (y - params->deltay1) * 200 / params->video->height;
      w = w * 320 / params->video->width;
      h = h * 200 / params->video->height;
    }
  }

  V_FillNumPatchBG(brdr_t.lumpnum, x,       y - g,  w, t, flags); // top
  V_FillNumPatchBG(brdr_b.lumpnum, x,       y + h,  w, b, flags); // bottom
  V_FillNumPatchBG(brdr_l.lumpnum, x - g,   y,      l, h, flags); // left
  V_FillNumPatchBG(brdr_r.lumpnum, x + w,   y,      r, h, flags); // right

  // Draw beveled edge.
  V_DrawNumPatchBG(x - g, y - g, brdr_tl.lumpnum, CR_DEFAULT, flags); // top left
  V_DrawNumPatchBG(x + w, y - g, brdr_tr.lumpnum, CR_DEFAULT, flags); // top right
  V_DrawNumPatchBG(x - g, y + h, brdr_bl.lumpnum, CR_DEFAULT, flags); // bottom left
  V_DrawNumPatchBG(x + w, y + h, brdr_br.lumpnum, CR_DEFAULT, flags); // bottom right
}

//
// R_FillBackScreen
// Fills the back screen with a pattern
//  for variable screen sizes
// Also draws a beveled edge.
//
// CPhipps - patch drawing updated

void R_FillBackScreen (void)
{
  int automap = automap_solid || dsda_FullAutomapHud();

  if (grnrock.lumpnum == 0)
    return;

  V_BeginUIDraw();

  // e6y: wide-res
  if (ratio_multiplier != ratio_scale || wide_offsety)
  {
    int screenblocks = R_ViewSize();
    int only_stbar = (screenblocks >= 10);

    if (V_IsOpenGLMode() && !automap)
      only_stbar = (screenblocks == 10);

    if (only_stbar)
    {
      int stbar_top = SCREENHEIGHT - ST_SCALED_HEIGHT;

      if (sts_solid_bg_color)
      {
        R_FillBackColor();
        V_EndUIDraw();
        return;
      }

      V_FillNumFlatBG(grnrock.lumpnum, 0, stbar_top, SCREENWIDTH, ST_SCALED_HEIGHT, VPT_STRETCH);

      // line between view and status bar
      R_DrawStbarBorder();

      V_EndUIDraw();
      return;
    }
  }

  if (scaledviewwidth == SCREENWIDTH)
  {
    V_EndUIDraw();
    return;
  }

  V_FillNumFlatBG(grnrock.lumpnum, 0, 0, SCREENWIDTH, SCREENHEIGHT, VPT_STRETCH);

  R_DrawBorder(viewwindowx, viewwindowy, scaledviewwidth, viewheight);

  if (sts_solid_bg_color)
    R_FillBackColor();

  V_EndUIDraw();
}

//
// R_CopyScreenBufferRect
//
// Copies a rectangle between transposed screen buffers.
//

static void R_CopyScreenBufferRect(int x, int y, int width, int height)
{
  if (V_IsSoftwareMode())
  {
    byte *dest;
    byte *src;
    int i;

    dest = screens[FG].data + y + x * screens[FG].pitch;
    src  = screens[BG].data + y + x * screens[BG].pitch;

    for (i = 0; i < width; ++i)
    {
      memcpy(dest, src, height);
      dest += screens[FG].pitch;
      src  += screens[BG].pitch;
    }
  }
}

//
// R_DrawViewBorder
// Draws the border around the view
//  for different size windows?
//

void R_DrawViewBorder(void)
{
  int top, side;

  if (V_IsOpenGLMode()) {
    // proff 11/99: we don't have a backscreen in OpenGL from where we can copy this
    R_FillBackScreen();
    return;
  }

  // e6y: wide-res
  if (ratio_multiplier != ratio_scale || wide_offsety)
  {
    R_CopyScreenBufferRect(0, SCREENHEIGHT - ST_SCALED_HEIGHT, SCREENWIDTH, ST_SCALED_HEIGHT);
  }

  if ( viewheight >= ( SCREENHEIGHT - ST_SCALED_HEIGHT ))
    return; // if high-res, don't go any further!

  top = ((SCREENHEIGHT - ST_SCALED_HEIGHT) - viewheight) / 2;
  side = (SCREENWIDTH - scaledviewwidth) / 2;

  // copy top
  R_CopyScreenBufferRect(0, 0, SCREENWIDTH, top);

  // copy sides
  R_CopyScreenBufferRect(0, top, side, viewheight);
  R_CopyScreenBufferRect(viewwidth + side, top, side, viewheight);

  // copy bottom
  R_CopyScreenBufferRect(0, top + viewheight, SCREENWIDTH, SCREENHEIGHT - ST_SCALED_HEIGHT - top - viewheight);
}

void R_SetFuzzPos(int fp)
{
  fuzzpos = fp;
}

void R_SetFuzzPosScaled(int scaledfp)
{
  scaledfuzzpos = scaledfp;
}

int R_GetFuzzPos()
{
  return fuzzpos;
}

int R_GetFuzzPosScaled()
{
  return scaledfuzzpos;
}

void R_ResetFuzzCol(int height)
{
  fuzzpos = (fuzzpos + (height / fuzzcellsize)) % FUZZTABLE;
}

void R_ResetFuzzColScaled(int height)
{
  scaledfuzzpos = (scaledfuzzpos + (height / scaled_fuzzcellsize)) % FUZZTABLE;
}

void R_CheckFuzzCol(int x, int height)
{
  if (!(x % fuzzcellsize))
    R_ResetFuzzCol(height);

  if (!(x % scaled_fuzzcellsize))
    R_ResetFuzzColScaled(height);
}
