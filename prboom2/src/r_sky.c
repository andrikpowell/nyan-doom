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
 *  Sky rendering. The DOOM sky is a texture map like any
 *  wall, wrapping around. A 1024 columns equal 360 degrees.
 *  The default sky map is 256 columns and repeats 4 times
 *  on a 320 screen?
 *
 *-----------------------------------------------------------------------------*/

#include <stdlib.h>

#include "r_sky.h"
#include "r_main.h"
#include "r_data.h"
#include "r_patch.h"
#include "v_video.h"
#include "z_zone.h"
#include "e6y.h"

#include "dsda/configuration.h"
#include "dsda/excmd.h"
#include "dsda/mapinfo.h"
#include "dsda/palette.h"
#include "dsda/settings.h"

//
// sky mapping
//
int skyflatnum;
int skytexture;
int skytexture2;
int skytexturemid;

int skystretch;
fixed_t freelookviewheight;

//
// [Woof] Color Above Sky
//

static int R_CompareSkyColors(const void *a, const void *b)
{
  const SDL_Color *rgb_a = a;
  const SDL_Color *rgb_b = b;

  int red_a = rgb_a->r, grn_a = rgb_a->g, blu_a = rgb_a->b;
  int red_b = rgb_b->r, grn_b = rgb_b->g, blu_b = rgb_b->b;

  int sum_a = red_a * red_a + grn_a * grn_a + blu_a * blu_a;
  int sum_b = red_b * red_b + grn_b * grn_b + blu_b * blu_b;

  return sum_a - sum_b;
}

static byte R_GetSkyTextureColor(int tex)
{
  const byte *playpal = V_GetPlaypal();
  int i, r = 0, g = 0, b = 0;

  const rpatch_t *patch = R_TextureCompositePatchByNum(tex);
  const int width = textures[tex]->width;

  SDL_Color *colors = Z_Malloc(sizeof(*colors) * width);

  // [FG] count colors
  for (i = 0; i < width; ++i)
  {
    const byte pixel = R_GetTextureColumn(patch, i)[0];
    colors[i].r = playpal[3 * pixel + 0];
    colors[i].g = playpal[3 * pixel + 1];
    colors[i].b = playpal[3 * pixel + 2];
  }

  qsort(colors, width, sizeof(*colors), R_CompareSkyColors);

  // Desaturate colours
  r = colors[width / 3].r;
  g = colors[width / 3].g;
  b = colors[width / 3].b;
  Z_Free(colors);

  return V_BestColor(playpal, r, g, b);
}

typedef struct skycolor_s
{
  int texturenum;
  byte color;
  int palette;
  struct skycolor_s *next;
} skycolor_t;

// the sky colors hash table
#define NUMSKYCHAINS 13
static skycolor_t *skycolors[NUMSKYCHAINS];
#define skycolorkey(a) ((a) % NUMSKYCHAINS)

byte R_GetSkyColor(int texturenum)
{
  int key;
  int palette;
  skycolor_t *target = NULL;

  key = skycolorkey(texturenum);
  palette = dsda_PlayPalIndex();

  if (skycolors[key])
  {
      // search in chain
      skycolor_t *rover = skycolors[key];

      while (rover)
      {
          if (rover->texturenum == texturenum)
          {
              target = rover;
              break;
          }

          rover = rover->next;
      }
  }

  if (target == NULL)
  {
    target = Z_Malloc(sizeof(*target));

    target->texturenum = texturenum;
    target->palette = -1;

    // use head insertion
    target->next = skycolors[key];
    skycolors[key] = target;
  }

  if (target->palette != palette)
  {
    target->color = R_GetSkyTextureColor(texturenum);
    target->palette = palette;
  }

  return target->color;
}

//
// R_InitSkyMap
// Called whenever the view size changes.
//
void R_InitSkyMap(void)
{
  int r_stretchsky;

  r_stretchsky = dsda_IntConfig(dsda_config_render_stretchsky);

  if (raven || !dsda_FreeAim())
  {
    skystretch = false;
    skytexturemid = (raven ? 200 : 100) * FRACUNIT;
    if (viewwidth != 0)
    {
      skyiscale = (fixed_t)(((uint64_t)FRACUNIT * SCREENWIDTH * 200) / (viewwidth * SCREENHEIGHT));
    }
  }
  else
  {
    int skyheight;

    if (!textureheight)
      return;

    // There are various combinations for sky rendering depending on how tall the sky is:
    //        h <  128: Unstretched and tiled, centered on horizon
    // 128 <= h <  200: Can possibly be stretched. When unstretched, the baseline is
    //                  28 rows below the horizon so that the top of the texture
    //                  aligns with the top of the screen when looking straight ahead.
    //                  When stretched, it is scaled to 228 pixels with the baseline
    //                  in the same location as an unstretched 128-tall sky, so the top
    //					of the texture aligns with the top of the screen when looking
    //                  fully up.
    //        h == 200: Unstretched, baseline is on horizon, and top is at the top of
    //                  the screen when looking fully up.
    //        h >  200: Unstretched, but the baseline is shifted down so that the top
    //                  of the texture is at the top of the screen when looking fully up.

    skyheight = textureheight[skytexture]>>FRACBITS;
    skystretch = false;
    skytexturemid = 0;
    if (skyheight >= 128 && skyheight < 200)
    {
      skystretch = (r_stretchsky && skyheight >= 128);
      skytexturemid = -28*FRACUNIT;
    }
    else if (skyheight > 200)
    {
      skytexturemid = (200 - skyheight) << FRACBITS;
    }

    if (viewwidth != 0 && viewheight != 0)
    {
      //skyiscale = 200 * FRACUNIT / freelookviewheight;
      skyiscale = (fixed_t)(((uint64_t)FRACUNIT * SCREENWIDTH * 200) / (viewwidth * SCREENHEIGHT));
      // line below is from zdoom, but it works incorrectly with prboom
      // with widescreen resolutions (eg 1280x720) by some reasons
      //skyiscale = (fixed_t)((int64_t)skyiscale * FieldOfView / 2048);
    }

    if (skystretch)
    {
      skyiscale = (fixed_t)((int64_t)skyiscale * skyheight / SKYSTRETCH_HEIGHT);
      skytexturemid = (int)((int64_t)skytexturemid * skyheight / SKYSTRETCH_HEIGHT);
    }
    else
    {
      skytexturemid = 100*FRACUNIT;
    }
  }
}
