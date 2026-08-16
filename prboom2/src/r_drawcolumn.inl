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
 *-----------------------------------------------------------------------------*/

#define R_DRAWCOLUMN_TRANSLATED (R_DRAWCOLUMN_PIPELINE & RDC_TRANSLATED || R_DRAWCOLUMN_PIPELINE & RDC_TRTL || R_DRAWCOLUMN_PIPELINE & RDC_ALT_TRTL )
#define R_DRAWCOLUMN_TRANSLUCENT (R_DRAWCOLUMN_PIPELINE & RDC_TRANSLUCENT || R_DRAWCOLUMN_PIPELINE & RDC_TRTL )
#define R_DRAWCOLUMN_TRANSLUCENT_REVERSE ( R_DRAWCOLUMN_PIPELINE & RDC_ALT_TRTL || R_DRAWCOLUMN_PIPELINE & RDC_ALT_TL)

#define R_DRAWCOLUMN_FUZZ (R_DRAWCOLUMN_PIPELINE & RDC_FUZZ || R_DRAWCOLUMN_PIPELINE & RDC_FUZZ_SCALED)
#define R_DRAWCOLUMN_SKY_COLOR_CAP (R_DRAWCOLUMN_PIPELINE & RDC_SKY_COLOR_CAP)

#if (R_DRAWCOLUMN_TRANSLATED)
#define GETCOL_MAPPED(col) (translation[(col)])
#else
#define GETCOL_MAPPED(col) (col)
#endif

#if (R_DRAWCOLUMN_PIPELINE & RDC_NOCOLMAP)
  #define GETCOL_DEPTH(col) GETCOL_MAPPED(col)
#else
  #define GETCOL_DEPTH(col) colormap[GETCOL_MAPPED(col)]
#endif

// If Hexen double sky is active, check for transparency in sky 1
// Translucency works by skipping black (byte 0)
#if (R_DRAWCOLUMN_PIPELINE & RDC_DOUBLESKY)
  #define SKY1_TRANS(frac) source[(frac)>>FRACBITS]
  #define GETCOL(frac) \
    SKY1_TRANS(frac) == 0 \
      ? GETCOL_DEPTH(source2[(frac)>>FRACBITS]) \
      : GETCOL_DEPTH(source[(frac)>>FRACBITS])
#else
// Otherwise draw normal sky
#define GETCOL(frac) GETCOL_DEPTH(source[(frac)>>FRACBITS])
#endif

#if R_DRAWCOLUMN_TRANSLUCENT
#define R_COLUMN_COLOR(color) (tranmap[((*dest) << 8) + (color)])
#elif R_DRAWCOLUMN_TRANSLUCENT_REVERSE
#define R_COLUMN_COLOR(color) (tranmap[(*dest) + ((color) << 8)])
#else // Opaque
#define R_COLUMN_COLOR(color) (color)
#endif

static void R_DRAWCOLUMN_FUNCNAME(draw_column_vars_t *dcvars)
{
  int              count;

#if (!(R_DRAWCOLUMN_FUZZ))
  byte             *dest;            // killough
  fixed_t          frac;
  const fixed_t    fracstep = dcvars->iscale;
#endif

#if (R_DRAWCOLUMN_FUZZ)
  fuzz_cutoff = false;

  // Adjust borders. Low...
  if (!dcvars->yl)
    dcvars->yl = 1;

  // .. and high.
  if (dcvars->yh == viewheight-1)
  {
    dcvars->yh = viewheight - 2;
    fuzz_cutoff = true;
  }
#endif

  // leban 1/17/99:
  // removed the + 1 here, adjusted the if test, and added an increment
  // later.  this helps a compiler pipeline a bit better.  the x86
  // assembler also does this.

  count = dcvars->yh - dcvars->yl;

  // leban 1/17/99:
  // this case isn't executed too often.  depending on how many instructions
  // there are between here and the second if test below, this case could
  // be moved down and might save instructions overall.  since there are
  // probably different wads that favor one way or the other, i'll leave
  // this alone for now.
  if (count < 0)    // Zero length, column does not exceed a pixel.
    return;

#ifdef RANGECHECK
  if (dcvars->x >= SCREENWIDTH
      || dcvars->yl < 0
      || dcvars->yh >= SCREENHEIGHT)
#if R_DRAWCOLUMN_FUZZ
    I_Error("R_DrawFuzzColumn: %i to %i at %i", dcvars->yl, dcvars->yh, dcvars->x);
#else
    I_Error("R_DrawColumn: %i to %i at %i", dcvars->yl, dcvars->yh, dcvars->x);
#endif
#endif

#if (!(R_DRAWCOLUMN_FUZZ))
  if (dcvars->flags & DRAW_COLUMN_ISPATCH)
    frac = ((dcvars->yl - dcvars->dy) * fracstep) & 0xFFFF;
  else
    frac = dcvars->texturemid + (dcvars->yl-centery)*fracstep;
#endif

  // Crop to custom region
  if (dcvars->flags & DRAW_COLUMN_ISPATCH)
  {
    if (dcvars->crop.top != 0 || dcvars->crop.bottom != 0)
    {
      if (dcvars->crop.top > dcvars->yl)
      {
    #if (!(R_DRAWCOLUMN_FUZZ))
        int delta = dcvars->crop.top - dcvars->yl;
    #endif

        dcvars->yl = dcvars->crop.top;

    #if (!(R_DRAWCOLUMN_FUZZ))
          frac += fracstep * delta;
    #endif
      }

      if (dcvars->crop.bottom < dcvars->yh)
        dcvars->yh = dcvars->crop.bottom;

      count = dcvars->yh - dcvars->yl;

      if (count < 0)
        return;
    }
  }

// Draw scaled fuzz directly into the transposed framebuffer
#if R_DRAWCOLUMN_FUZZ
  #if (R_DRAWCOLUMN_PIPELINE & RDC_FUZZ_SCALED)
    R_DrawFuzzColumn(dcvars, scaled_fuzzcellsize, &scaledfuzzpos);
  #else
    R_DrawFuzzColumn(dcvars, fuzzcellsize, &fuzzpos);
  #endif

// do nothing else when drawin fuzz columns
#else
  {
    // Framebuffer destination address.
    dest = drawvars.topleft + dcvars->yl + dcvars->x * drawvars.pitch;

    const byte          *source = dcvars->source;

#if (R_DRAWCOLUMN_PIPELINE & RDC_DOUBLESKY)
    const byte          *source2 = dcvars->source2;
#endif

#if (!(R_DRAWCOLUMN_PIPELINE & RDC_NOCOLMAP))
    const lighttable_t  *colormap = dcvars->colormap;
#endif

#if (R_DRAWCOLUMN_TRANSLATED)
    const byte          *translation = dcvars->translation;
#endif

    count++;

// [Woof] Draw sky with color on top
#if (R_DRAWCOLUMN_SKY_COLOR_CAP)
    const byte sky_cap_color = dcvars->skycolor;
    const byte sky_texture_color = GETCOL(0);
    const byte fade_50 = dcvars->sky_tranmap[sky_texture_color * 256 + sky_cap_color];
    const byte fade_25 = dcvars->sky_tranmap[fade_50 * 256 + sky_cap_color];
    int n;

    if (frac < -2 * FRACUNIT)
    {
      n = (-frac - 2 * FRACUNIT + fracstep - 1) / fracstep;
      if (n > count)
        n = count;

      count -= n;
      while (n--)
      {
        *dest = R_COLUMN_COLOR(sky_cap_color);
        dest++;
        frac += fracstep;
      }

      if (!count)
        return;
    }

    if (frac < -FRACUNIT)
    {
      n = (-frac - FRACUNIT + fracstep - 1) / fracstep;
      if (n > count)
        n = count;

      count -= n;
      while (n--)
      {
        *dest = R_COLUMN_COLOR(fade_25);
        dest++;
        frac += fracstep;
      }

      if (!count)
        return;
    }

    if (frac < 0)
    {
      n = (-frac + fracstep - 1) / fracstep;
      if (n > count)
        n = count;

      count -= n;
      while (n--)
      {
        *dest = R_COLUMN_COLOR(fade_50);
        dest++;
        frac += fracstep;
      }

      if (!count)
        return;
    }
#endif

    // Inner loop that does the actual texture mapping,
    //  e.g. a DDA-lile scaling.
    // This is as fast as it gets.       (Yeah, right!!! -- killough)
    //
    // killough 2/1/98: more performance tuning

    // [AR] Fix SSG fire bleeding bottom line
    // Player sprites can round one row past the current post.
    // Moved from R_DrawMaskedColumn() to fix >128 height (multi-post) psprites
    if (dcvars->pspritepostheight) {
      const fixed_t maxfrac = (dcvars->pspritepostheight << FRACBITS) - 1;

      while (count--) {
        const fixed_t pspritefrac = CLAMP(frac, 0, maxfrac);

        *dest = R_COLUMN_COLOR(GETCOL(pspritefrac));
        dest++;
        frac += fracstep;
      }
    // [R&R] Masked posts are already clipped and don't need texture wrapping
    // Better performance for opaque midtexs and sprites
#if ((R_DRAWCOLUMN_PIPELINE & RDC_STANDARD) && !(R_DRAWCOLUMN_PIPELINE & RDC_NOCOLMAP))
    } else if (dcvars->drawingmasked) {
      while ((count -= 2) >= 0) {
        *dest = colormap[source[frac >> FRACBITS]];
        dest++;
        frac += fracstep;
        *dest = colormap[source[frac >> FRACBITS]];
        dest++;
        frac += fracstep;
      }
      if (count & 1)
        *dest = colormap[source[frac >> FRACBITS]];
#endif
    } else if (dcvars->texheight == 128) {
      #define FIXEDT_128MASK ((127<<FRACBITS)|0xffff)
      while(count--) {
        *dest = R_COLUMN_COLOR(GETCOL(frac & FIXEDT_128MASK));
        dest++;
        frac += fracstep;
      }
    } else if (dcvars->texheight == 0) {
      /* cph - another special case */
      while (count--) {
        *dest = R_COLUMN_COLOR(GETCOL(frac));
        dest++;
        frac += fracstep;
      }
    } else {
      unsigned heightmask = dcvars->texheight-1; // CPhipps - specify type
      if (! (dcvars->texheight & heightmask) ) { // power of 2 -- killough
        fixed_t fixedt_heightmask = (heightmask<<FRACBITS)|0xffff;
        while ((count-=2)>=0) { // texture height is a power of 2 -- killough
          *dest = R_COLUMN_COLOR(GETCOL(frac & fixedt_heightmask));
          dest++;
          frac += fracstep;
          *dest = R_COLUMN_COLOR(GETCOL(frac & fixedt_heightmask));
          dest++;
          frac += fracstep;
        }
        if (count & 1)
          *dest = R_COLUMN_COLOR(GETCOL(frac & fixedt_heightmask));
      } else {
        heightmask++;
        heightmask <<= FRACBITS;

        if (frac < 0)
          while ((frac += heightmask) <  0);
        else
          while (frac >= (int)heightmask)
            frac -= heightmask;

        while (count--) {
          // Re-map color indices from wall texture column
          //  using a lighting/special effects LUT.

          // heightmask is the Tutti-Frutti fix -- killough

          *dest = R_COLUMN_COLOR(GETCOL(frac));
          dest++;
          if ((frac += fracstep) >= (int)heightmask)
            frac -= heightmask;
        }
      }
    }
  }
#endif // (!(R_DRAWCOLUMN_PIPELINE & RDC_FUZZ))
}

#undef GETCOL_MAPPED
#undef GETCOL_DEPTH
#undef GETCOL
#undef R_COLUMN_COLOR
#undef R_DRAWCOLUMN_TRANSLATED
#undef R_DRAWCOLUMN_TRANSLUCENT
#undef R_DRAWCOLUMN_TRANSLUCENT_REVERSE
#undef R_DRAWCOLUMN_FUZZ
#undef R_DRAWCOLUMN_SKY_COLOR_CAP
#undef R_DRAWCOLUMN_FUNCNAME
#undef R_DRAWCOLUMN_PIPELINE
