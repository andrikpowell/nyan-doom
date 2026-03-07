//
// Copyright(C) 2023 by Ryan Krafnick
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
//	DSDA TRANMAP
//

#include <string.h>

#include "doomdef.h"
#include "doomstat.h"
#include "md5.h"
#include "lprintf.h"
#include "m_file.h"
#include "r_data.h"
#include "w_wad.h"
#include "z_zone.h"

#include "dsda/configuration.h"
#include "dsda/data_organizer.h"
#include "dsda/settings.h"
#include "dsda/utility.h"

#include "tranmap.h"

static char* tranmap_base_dir;
static char* tranmap_palette_dir;
static dsda_cksum_t playpal_cksum;
int tran_filter_pct;
int exhud_tran_filter_pct;
int exhud_tint_filter_pct;
int exhud_alttint_filter_pct;
int shadow_filter_pct;
int alttint_filter_pct;
int exhud_shadow_filter_pct;
int shadow_ui_filter_pct;
int menu_ui_filter_pct;
int gl_alttint_filter_pct;
int gl_exhud_alttint_filter_pct;
static const int tranmap_length = 256 * 256;
static const byte* tranmap_data[100];

static void dsda_CalculatePlaypalCksum(void) {
  struct MD5Context md5;
  int lump;

  lump = W_GetNumForName("PLAYPAL");

  MD5Init(&md5);
  MD5Update(&md5, W_LumpByNum(lump), W_LumpLength(lump));
  MD5Final(playpal_cksum.bytes, &md5);
  dsda_TranslateCheckSum(&playpal_cksum);
}

static void dsda_InitTranMapBaseDir(void) {
  int length;
  const char* data_root;

  data_root = dsda_DataRoot();

  length = strlen(data_root) + 10; // "/tranmaps\0"
  tranmap_base_dir = Z_Malloc(length);
  snprintf(tranmap_base_dir, length, "%s/tranmaps", data_root);

  M_MakeDir(tranmap_base_dir, false);
}

static void dsda_InitTranMapPaletteDir(void) {
  int length;

  if (!tranmap_base_dir)
    dsda_InitTranMapBaseDir();

  if (!playpal_cksum.string[0])
    dsda_CalculatePlaypalCksum();

  length = strlen(tranmap_base_dir) + 34; // "/<cksum (32)>\0"
  tranmap_palette_dir = Z_Malloc(length);
  snprintf(tranmap_palette_dir, length, "%s/%s", tranmap_base_dir, playpal_cksum.string);

  M_MakeDir(tranmap_palette_dir, false);
}

//
// R_InitTranMap
//
// Initialize translucency filter map
//
// By Lee Killough 2/21/98
//

#define TSC 12 /* number of fixed point digits in filter percent */

static byte* dsda_GenerateTranMap(unsigned int alpha) {
  byte* buffer;
  const byte* playpal;
  int pal[3][256];
  int tot[256];
  int pal_w1[3][256];
  int w1, w2;

  playpal = W_LumpByName("PLAYPAL");

  w1 = (alpha << TSC) / 100;
  w2 = (1l << TSC) - w1;

  buffer = Z_Malloc(tranmap_length);

  // First, convert playpal into long int type, and transpose array,
  // for fast inner-loop calculations. Precompute tot array.
  {
    register int i = 255;
    register const unsigned char *p = playpal + 255 * 3;

    do {
      register int t, d;
      pal_w1[0][i] = (pal[0][i] = t = p[0]) * w1;
      d = t * t;
      pal_w1[1][i] = (pal[1][i] = t = p[1]) * w1;
      d += t * t;
      pal_w1[2][i] = (pal[2][i] = t = p[2]) * w1;
      d += t * t;
      p -= 3;
      tot[i] = d << (TSC - 1);
    }
    while (--i >= 0);
  }

  // Next, compute all entries using minimum arithmetic.
  {
    int i, j;
    byte *tp = buffer;

    for (i = 0; i < 256; i++) {
      int r1 = pal[0][i] * w2;
      int g1 = pal[1][i] * w2;
      int b1 = pal[2][i] * w2;

      for (j = 0; j < 256; j++, tp++) {
        register int color = 255;
        register int err;
        int r = pal_w1[0][j] + r1;
        int g = pal_w1[1][j] + g1;
        int b = pal_w1[2][j] + b1;
        int best = INT_MAX;

        do {
          err = tot[color] - pal[0][color] * r - pal[1][color] * g - pal[2][color] * b;
          if (err < best) {
            best = err;
            *tp = color;
          }
        }
        while (--color >= 0);
      }
    }
  }

  return buffer;
}

const byte* dsda_TranMap(unsigned int alpha) {
  int length;
  byte *buffer = NULL;

  if (alpha > 99)
    return NULL;

  if (!tranmap_data[alpha]) {
    char* filename;

    if (!tranmap_palette_dir)
      dsda_InitTranMapPaletteDir();

    length = strlen(tranmap_palette_dir) + 16; // "/tranmap_99.dat\0"
    filename = Z_Malloc(length);
    snprintf(filename, length, "%s/tranmap_%02d.dat", tranmap_palette_dir, alpha);

    length = M_ReadFile(filename, &buffer);
    if (buffer && length != tranmap_length) {
      Z_Free(buffer);
      buffer = NULL;
    }

    if (!buffer) {
      buffer = dsda_GenerateTranMap(alpha);

      M_WriteFile(filename, buffer, tranmap_length);
    }

    tranmap_data[alpha] = buffer;
  }

  return tranmap_data[alpha];
}

const byte* dsda_DefaultTranMap(void) {
  int lump;

  if (raven) return W_LumpByName("TINTTAB");

  lump = W_CheckNumForName("TRANMAP");

  if (lump != LUMP_NOT_FOUND)
    return W_LumpByNum(lump);

  return dsda_TranMap(tran_filter_pct);
}

//
//
// Add custom tranmaps to memory
//
//

static byte* custom_tranmap_data[UI_END][100]; // [context][alpha]

const byte* dsda_TranMap_Custom(unsigned int alpha, int context)
{
  byte **slot;

  if (alpha < 1 || alpha > 99)
    return NULL;

  if (context <= UI_NONE || context >= UI_END)
    return NULL;

  slot = &custom_tranmap_data[context][alpha];

  if (!*slot)
    *slot = dsda_GenerateTranMap(alpha);

  return *slot;
}

//
//
// Precache custom transmaps for fading messages
//
//

int P_ConvertTrans(int val) {
  return CLAMP(val, 1, 99);
}

//
//
// Set tranmap percentages for OpenGL
//
//

// Following values from https://zdoom.org/wiki/ZScript_constants
// 40% - MF_SHADOW (Heretic) + MF_ALTSHADOW (Hexen)
// 60% - MF_SHADOW (Hexen)

//int tranmap_pct        = 66;    // Doom + Boom Transmap
int tinttable_pct        = 40;    // Heretic MF_SHADOW + Hexen MF_ALTSHADOW
int alt_tinttable_pct    = 60;    // Hexen MF_SHADOW

void dsda_UpdateTranMap(void) {
  // Heretic + Hexen have forced percentages due to use of tinttable

  // main percentages
  tran_filter_pct       = raven ? tinttable_pct : dsda_TranslucencyPercent(); // Allow translucency customisation only for Doom / Boom
  alttint_filter_pct    = raven ? alt_tinttable_pct : P_ConvertTrans(100-tran_filter_pct); // reverse translucency
  shadow_filter_pct     = hexen ? alt_tinttable_pct : tinttable_pct;

  // ui stuff (menu text shadows) - never use tinttable
  shadow_ui_filter_pct  = P_ConvertTrans(dsda_ShadowTranslucencyPercent());
  menu_ui_filter_pct    = P_ConvertTrans(dsda_MenuTranslucencyPercent());

  // exhud percentages
  exhud_tran_filter_pct    = P_ConvertTrans(dsda_ExHudTranslucencyPercent());
  exhud_shadow_filter_pct  = P_ConvertTrans((int)(((float)shadow_filter_pct/100.0)*((float)exhud_tran_filter_pct/100.0)*100.0));
  exhud_tint_filter_pct    = P_ConvertTrans((int)(((float)tran_filter_pct/100.0)*((float)exhud_tran_filter_pct/100.0)*100.0));    // normal translucency under translucency o.O
  exhud_alttint_filter_pct = P_ConvertTrans(100-exhud_tint_filter_pct);                                                           // reverse translucency under translucency o.O

  // OpenGL special precentages
  // Let's just avoid the reversing part (since we can't access tinttable)
  gl_alttint_filter_pct       = raven ? P_ConvertTrans(tran_filter_pct + 20) : P_ConvertTrans(tran_filter_pct - 20);              // reverse translucency o.O
  gl_exhud_alttint_filter_pct = raven ? P_ConvertTrans(exhud_tint_filter_pct + 20) : P_ConvertTrans(exhud_tint_filter_pct - 20);  // reverse translucency under translucency o.O

  // store main transmaps
  main_tranmap      = dsda_DefaultTranMap();
  menu_ui_tranmap   = dsda_TranMap_Custom(menu_ui_filter_pct, UI_TRANS); // ui menu translucency
}
